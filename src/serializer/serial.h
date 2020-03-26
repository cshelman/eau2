#pragma once

#include <stdio.h>
#include <string.h>

#include "array.h"
#include "buffer.h"
#include "message.h"

// Helpers
void add_tag(const char* tag, char* val, Buffer* buffer) {
    buffer->add(tag);
    buffer->add(":{");
    buffer->add(val);
    buffer->add("}");
}

// char* are not allowed to contain double quotes at the moment
char* get_word(char* s, char open, char close) {
    char* word = new char[strlen(s) + 1];
    int bracket_count = 0;
    bool in_string = false;
    int start = -1;
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == '\"') {
            in_string = !in_string;
        }
        else if (s[i] == open && !in_string) {
            bracket_count++;
            if (start == -1) {
                start = i + 1;
            }
        }
        else if (s[i] == close && !in_string) {
            bracket_count--;
            if (bracket_count == 0) {
                memcpy(word, &s[start], i - start);
                return word;
            }
        }
    }
    printf("bad input to get_word:\n\t%s\n", s);
    exit(1);
}

StringArray* get_components(char* s) {
    StringArray* components = new StringArray();
    bool in_string = false;
    int bracket_count = 0;
    int list_count = 0;
    int start = 0;
    for (int i = 0; i < strlen(s); i++) {
        if (s[i] == '\"') {
            in_string = !in_string;
        }
        else if (s[i] == '{' && !in_string) {
            bracket_count++;
        }
        else if (s[i] == '}' && !in_string) {
            bracket_count--;
        }
        else if (s[i] == '[' && !in_string) {
            list_count++;
        }
        else if (s[i] == ']' && !in_string) {
            list_count--;
        }
        else if (s[i] == ',' && !in_string && bracket_count + list_count == 0) {
            char* comp = new char[i - start];
            memcpy(comp, &s[start], i - start);
            String* str = new String(comp, i - start);
            components->add(str);
            start = i + 1;
        }
        if (i >= strlen(s) - 1) {
            char* comp = new char[i - start + 1];
            memcpy(comp, &s[start], i - start + 1);
            String* str = new String(comp, i - start + 1);
            components->add(str);
        }
    }
    return components;
}

// Serialization / deserialization functions

void serialize_chars(char* s, Buffer* buffer) {
    char* ns = new char[strlen(s) + 3];
    memcpy(&ns[0], "\"", 1);
    memcpy(&ns[1], s, strlen(s));
    memcpy(&ns[strlen(s) + 1], "\"\0", 2);
    add_tag("char*", ns, buffer);
}

char* deserialize_chars(char* s) {
    char* quotes = get_word(s, '{', '}');
    char* removed = new char[strlen(s) - 2];
    memcpy(removed, &quotes[1], strlen(s) - 10);
    return removed;
}

void serialize_size_t(size_t n, Buffer* buffer) {
    char* ns = new char[64];
    sprintf(ns, "%zu", n);
    add_tag("size_t", ns, buffer);
}

size_t deserialize_size_t(char* s) {
    size_t st = 0;
    sscanf(get_word(s, '{', '}'), "%zu", &st);
    return st;
}

void serialize_int(int n, Buffer* buffer) {
    char* ns = new char[16];
    sprintf(ns, "%d", n);
    add_tag("int", ns, buffer);
}

int deserialize_int(char* s) {
    return atoi(get_word(s, '{', '}'));
}

void serialize_double(double d, Buffer* buffer) {
    char* ns = new char[512];
    sprintf(ns, "%f", d);
    add_tag("double", ns, buffer);
}

double deserialize_double(char* s) {
    return atof(get_word(s, '{', '}'));
}

void serialize_string(String* s, Buffer* buffer) {
    Buffer* temp = new Buffer();
    serialize_chars(s->c_str(), temp);
    temp->add(",");
    serialize_size_t(s->size(), temp);
    temp->add("\0");

    add_tag("string", temp->val, buffer);
    delete temp;
}

String* deserialize_string(char* s) {
    StringArray* components = get_components(get_word(s, '{', '}'));
    char* val = deserialize_chars(components->get(0)->c_str());
    int len = deserialize_size_t(components->get(1)->c_str());
    String* str = new String(val, len);
    return str;
}

void serialize_string_array(StringArray* sa, Buffer* buffer) {
    Buffer* temp = new Buffer();
    temp->add("list:[");
    for (int i = 0; i < sa->get_len(); i++) {
        serialize_string(sa->get(i), temp);
        if (i < sa->get_len() - 1) {
            temp->add(",");
        }
    }
    temp->add("],");

    serialize_size_t(sa->cap, temp);
    temp->add(",");
    serialize_size_t(sa->size, temp);

    add_tag("StringArray", temp->val, buffer);
    delete temp;
}

StringArray* deserialize_string_array(char* s) {
    StringArray* components = get_components(get_word(s, '{', '}'));
    StringArray* sa = new StringArray();
    sa->cap = deserialize_size_t(components->get(1)->c_str());
    sa->size = deserialize_size_t(components->get(2)->c_str());

    String** los = new String*[sa->cap];
    StringArray* list_items = get_components(get_word(components->get(0)->c_str(), '[', ']'));
    for (int i = 0; i < list_items->get_len(); i++) {
        los[i] = deserialize_string(list_items->get(i)->c_str());
    }
    sa->los = los;
    return sa;
}

void serialize_double_array(DoubleArray* da, Buffer* buffer) {
    Buffer* temp = new Buffer();
    temp->add("list:[");
    for (int i = 0; i < da->get_len(); i++) {
        serialize_double(da->get(i), temp);
        if (i < da->get_len() - 1) {
            temp->add(",");
        }
    }
    temp->add("],");

    serialize_size_t(da->cap, temp);
    temp->add(",");
    serialize_size_t(da->size, temp);

    add_tag("DoubleArray", temp->val, buffer);
    delete temp;
}

DoubleArray* deserialize_double_array(char* s) {
    StringArray* components = get_components(get_word(s, '{', '}'));
    DoubleArray* da = new DoubleArray();
    da->cap = deserialize_size_t(components->get(1)->c_str());
    da->size = deserialize_size_t(components->get(2)->c_str());

    double* lod = new double[da->cap];
    StringArray* list_items = get_components(get_word(components->get(0)->c_str(), '[', ']'));
    for (int i = 0; i < list_items->get_len(); i++) {
        lod[i] = deserialize_double(list_items->get(i)->c_str());
    }
    da->lod = lod;
    return da;
}

void serialize_msg_kind(MsgKind mk, Buffer* buffer) {
    char* ns = new char[4];
    sprintf(ns, "%d", static_cast<int>(mk));
    add_tag("MsgKind", ns, buffer);
}

MsgKind deserialize_msg_kind(char* s) {
    MsgKind mk = (MsgKind)atoi(get_word(s, '{', '}'));
    return mk;
}

void serialize_sockaddr_in(struct sockaddr_in sa, Buffer* buffer) {
    Buffer* temp = new Buffer();
    serialize_int((int)sa.sin_family, temp);
    temp->add(",");
    serialize_int((int)sa.sin_port, temp);
    temp->add(",");
    serialize_size_t(sa.sin_addr.s_addr, temp);

    add_tag("sockaddr_in", temp->val, buffer);
}

struct sockaddr_in deserialize_sockaddr_in(char* s) {
    StringArray* components = get_components(get_word(s, '{', '}'));
    
    struct sockaddr_in ret;
    ret.sin_family = (short)deserialize_int(components->get(0)->c_str());
    ret.sin_port = (short)deserialize_int(components->get(1)->c_str());
    struct in_addr ia;
    ia.s_addr = (unsigned long)deserialize_size_t(components->get(2)->c_str());
    ret.sin_addr = ia;

    return ret;
}

void serialize_message(Message* m, Buffer* buffer) {
    Buffer* temp = new Buffer();
    serialize_msg_kind(m->kind_, temp);
    temp->add(",");
    serialize_size_t(m->sender_, temp);
    temp->add(",");
    serialize_size_t(m->target_, temp);
    temp->add(",");
    serialize_size_t(m->id_, temp);

    if (m->kind_ == MsgKind::Status) {
        Status* ms = dynamic_cast<Status*>(m);
        if (ms == nullptr) {
            printf("wrong message type\n");
        }
        temp->add(",");
        serialize_string(ms->msg_, temp);
    }
    else if (m->kind_ == MsgKind::Register) {
        Register* ms = dynamic_cast<Register*>(m);
        if (ms == nullptr) {
            printf("wrong message type\n");
        }
        temp->add(",");
        serialize_sockaddr_in(ms->slave_, temp);
        temp->add(",");
        serialize_size_t(ms->port_, temp);
    }
    else if (m->kind_ == MsgKind::Directory) {
        Directory* ms = dynamic_cast<Directory*>(m);
        if (ms == nullptr) {
            printf("wrong message type\n");
        }
        temp->add(",");
        serialize_size_t(ms->slaves_, temp);
        temp->add(",");

        temp->add("list:[");
        int portlen = ms->slaves_;
        for (int i = 0; i < portlen; i++) {
            serialize_size_t(ms->ports_[i], temp);
            if (i < portlen - 1) {
                temp->add(",");
            }
        }
        temp->add("],");

        temp->add("list:[");
        int addrlen = ms->slaves_;
        for (int i = 0; i < addrlen; i++) {
            serialize_string(ms->addresses_[i], temp);
            if (i < addrlen - 1) {
                temp->add(",");
            }
        }
        temp->add("]");
    }

    add_tag("Message", temp->val, buffer);
}

Message* deserialize_message(char* s) {
    StringArray* components = get_components(get_word(s, '{', '}'));
    MsgKind kind = deserialize_msg_kind(components->get(0)->c_str());
    size_t sender = deserialize_size_t(components->get(1)->c_str());
    size_t target = deserialize_size_t(components->get(2)->c_str());
    size_t id = deserialize_size_t(components->get(3)->c_str());

    if (kind == MsgKind::Status) {
        Status* status_message = new Status(kind, sender, target, id);
        status_message->set_message(deserialize_string(components->get(4)->c_str()));
        return status_message;
    }
    else if (kind == MsgKind::Register) {
        Register* register_message = new Register(kind, sender, target, id);
        register_message->set_slave(deserialize_sockaddr_in(components->get(4)->c_str()));
        register_message->set_port(deserialize_size_t(components->get(5)->c_str()));
        return register_message;
    }
    else if (kind == MsgKind::Directory) {
        Directory* directory_message = new Directory(kind, sender, target, id);
        directory_message->set_slaves(deserialize_size_t(components->get(4)->c_str()));

        size_t* ports = new size_t[directory_message->slaves_];
        StringArray* port_items = get_components(get_word(components->get(5)->c_str(), '[', ']'));
        for (int i = 0; i < directory_message->slaves_; i++) {
            ports[i] = deserialize_size_t(port_items->get(i)->c_str());
        }
        directory_message->set_ports(ports);

        String** addresses = new String*[directory_message->slaves_];
        StringArray* address_items = get_components(get_word(components->get(6)->c_str(), '[', ']'));
        for (int i = 0; i < directory_message->slaves_; i++) {
            addresses[i] = deserialize_string(address_items->get(i)->c_str());
        }
        directory_message->set_addresses(addresses);

        return directory_message;
    }
    else {
        Message* message = new Message(kind, sender, target, id);
        return message;
    }
}