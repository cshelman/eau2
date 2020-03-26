#include <stdio.h>

#include "buffer.h"
#include "object.h"
#include "string.h"
#include "serial.h"
#include "array.h"
#include "message.h"

int main(int argc, char** argv) {
    // StringArray example
    String* s1 = new String("a", 1);
    String* s2 = new String("bee", 3);
    String* s3 = new String("seasee", 6);
    StringArray* strarr = new StringArray();
    strarr->add(s1);
    strarr->add(s2);
    strarr->add(s3);
    Buffer* strbuf = new Buffer();
    serialize_string_array(strarr, strbuf);
    StringArray* res_strarr = deserialize_string_array(strbuf->val);
    delete strbuf;
    printf("Finished StringArray Example.\n");

    // DoubleArray example
    double d1 = 1.5;
    double d2 = 123.456;
    double d3 = 55555.0;
    double d4 = -10.123;
    DoubleArray* doubarr = new DoubleArray();
    doubarr->add(d1);
    doubarr->add(d2);
    doubarr->add(d3);
    doubarr->add(d4);
    Buffer* doubuff = new Buffer();
    serialize_double_array(doubarr, doubuff);
    DoubleArray* res_doubarr = deserialize_double_array(doubuff->val);
    delete doubuff;
    printf("Finished DoubleArray Example.\n");

    // Basic Message example
    Message* msg = new Message(MsgKind::Nack, 0, 0, 1);
    Buffer* msgbuf = new Buffer();
    serialize_message(msg, msgbuf);
    Message* res_msg = deserialize_message(msgbuf->val);
    printf("Finished Basic Message Example.\n");

    // Directory Message example
    Buffer* dirbuff = new Buffer();
    Directory* dir = new Directory(MsgKind::Directory, 100, 12, 4);
    String* s4 = new String("127.0.0.10", 10);
    String* s5 = new String("127.0.0.11", 10);
    String* addresses[2] = { s4, s5 };
    size_t ports[2] = { 8086, 8087 };
    dir->set_slaves(2); // this must be done first
    dir->set_ports(ports);
    dir->set_addresses(addresses);
    serialize_message(dir, dirbuff);
    Directory* res_dir = dynamic_cast<Directory*>(deserialize_message(dirbuff->val));
    printf("Finished Directory Message Example.\n");

    return 0;
}