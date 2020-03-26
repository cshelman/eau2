#pragma once

#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

enum class MsgKind { Ack, Nack, Put,
                    Reply,  Get, WaitAndGet, Status,
                    Kill,   Register,  Directory };

class Message : public Object {
public:
   MsgKind kind_;  // the message kind
   size_t sender_; // the index of the sender node
   size_t target_; // the index of the receiver node
   size_t id_;     // an id t unique within the node
   
   Message(MsgKind kind, size_t sender, size_t target, size_t id) {
      kind_ = kind;
      sender_ = sender;
      target_ = target;
      id_ = id;
   }

   ~Message() {}
};

class Ack : public Message {};

class Status : public Message {
public:
   String* msg_; // owned

   Status(MsgKind kind, size_t sender, size_t target, size_t id) : Message(kind, sender, target, id) {}
   
   ~Status() {
      delete msg_;
   }

   void set_message(String* msg) {
      msg_ = msg->clone();
   }
};

class Register : public Message {
public:
    sockaddr_in slave_; // not owned
    size_t port_;

    Register(MsgKind kind, size_t sender, size_t target, size_t id) : Message(kind, sender, target, id) {}

    void set_port(size_t port) {
       port_ = port;
    }

    void set_slave(sockaddr_in slave) {
       slave_ = slave;
    }
};

class Directory : public Message {
public:
   size_t slaves_;
   size_t* ports_;  // owned
   String** addresses_;  // owned; strings owned

   Directory(MsgKind kind, size_t sender, size_t target, size_t id) : Message(kind, sender, target, id) {}
   
   // this MUST be called first
   void set_slaves(size_t slaves) {
     slaves_ = slaves;
   }

   // undefined behavior if called before set_slaves()
   void set_ports(size_t* ports) {
      ports_ = new size_t[slaves_];
      for (int i = 0; i < slaves_; i++) {
         ports_[i] = ports[i];
      }
   }

   // undefined behavior if called before set_slaves()
   void set_addresses(String** addresses) {
      addresses_ = new String*[slaves_];
      for (int i = 0; i < slaves_; i++) {
         addresses_[i] = addresses[i]->clone();
      }
   }
};