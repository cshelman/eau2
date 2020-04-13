#pragma once

#include "../dataframe/dataframe.h"
#include "../util/string.h"
#include <map>
#include <string>

using namespace std;

class WordCountRower : public Rower {
public:
  map<string, int>* word_counts;

  WordCountRower(char* c) {
    word_counts = new map<string, int>();

    string* str = new string(c);
    int pos = str->find("`:`");

    if (pos == string::npos) {
      printf("Invalid WordCountRower to deserialize - no colon found.\n");
      exit(1);
    }

    string* vs_as_str = new string(str->substr(1, pos));
    string* vi_as_str = new string(str->substr(pos + 3, str->size() - pos));
    
    vector<String*>* vs = deserialize_str_vector((char*)vs_as_str->c_str());
    vector<int>* vi = deserialize_int_vector((char*)vi_as_str->c_str());

    if (vs->size() != vi->size()) {
      printf("Invalid WordCountRower to deserialize - sizes dont match.\n");
      exit(1);
    }
    
    for (int i = 0; i < vs->size(); i++) {
      string temp(vs->at(i)->c_str());
      word_counts->insert({temp, vi->at(i)});
    }
  }

  WordCountRower() {
    word_counts = new map<string, int>();
  }
  
  bool accept(Row& r) {
    for (int i = 0; i < r.width(); i++) {
        String* word = r.get_string(i);
        if (word == nullptr) {
            continue;
        }
        string* s = new string(word->c_str());
        
        if (word_counts->count(*s) > 0) {
            int count = word_counts->at(*s) + 1;
            word_counts->at(*s) = count;
        }
        else {
            word_counts->insert({*s, 1});
        }
    }
    return true;
  }

  void join_delete(Rower* other) {
    WordCountRower* other_wcr = dynamic_cast<WordCountRower*>(other);
    for (auto const &count : *other_wcr->word_counts) {
      if (word_counts->count(count.first) > 0) {
        word_counts->at(count.first) = word_counts->at(count.first) + count.second;
      }
      else {
        word_counts->insert({count.first, count.second});
      }
    }

    delete other;
  }

  Rower* clone() {
    return new WordCountRower();
  }

  void print() {
    for (auto const &count : *word_counts) {
      printf("%d\t|\t%s\n", count.second, count.first.c_str());
    }
  }

  string* serialize() {
    vector<String*>* vs = new vector<String*>();
    vector<int>* vi = new vector<int>();
    for (auto const &count : *word_counts) {
      String* s = new String((char*)count.first.c_str());
      vs->push_back(s);
      vi->push_back(count.second);
    }

    string* serialized = new string("2");
    serialized->append(serialize_str_vector(vs));
    serialized->append("`:`");
    serialized->append(serialize_int_vector(vi));
    return serialized;
  }

  Rower* deserialize(char* c) {
    return new WordCountRower(c);
  }
};
