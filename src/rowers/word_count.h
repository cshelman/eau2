#pragma once

#include "../dataframe/dataframe.h"
#include "../util/string.h"
#include <map>
#include <string>

using namespace std;

class WordCountRower : public Rower {
public:
  map<string, int>* word_counts;

  WordCountRower() {
    word_counts = new map<string, int>();
  }
  
  bool accept(Row& r) {
    // printf("starting wcr accept\n");
    for (int i = 0; i < r.width(); i++) {
        String* word = r.get_string(i);
        // printf("on word: %s\n", (char*)word->c_str());
        if (word == nullptr) {
            continue;
        }
        string* s = new string(word->c_str());
        
        if (word_counts->count(*s) > 0) {
            // printf("editing existing count for %s: \n", s->c_str());
            int count = word_counts->at(*s) + 1;
            word_counts->at(*s) = count;
        }
        else {
            // printf("inserting new count for %s: \n", s->c_str());
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

  char* serialize() {
    vector<String*>* vs = new vector<String*>();
    vector<int>* vi = new vector<int>();
    for (auto const &count : *word_counts) {
      String* s = new String((char*)count.first.c_str());
      vs->push_back(s);
      vi->push_back(count.second);
    }

    string* serialized = new string(serialize_str_vector(vs));
    serialized->append("`:`");
    serialized->append(serialize_int_vector(vi));
    return (char*)serialized->c_str();
  }

  Rower* deserialize(char* r) {
    WordCountRower* wcr = new WordCountRower();
    map<string, int>* wc = new map<string, int>();

    string* str = new string(r);
    int pos = str->find("`:`");

    if (pos == string::npos) {
      printf("Invalid WordCountRower to deserialize.\n");
      exit(1);
    }

    string* vs_as_str = new string(str->substr(0, pos));
    string* vi_as_str = new string(str->substr(pos + 3, str->size() - pos));

    // printf("serialized str vector: \n%s\n", (char*)vs_as_str->c_str());
    // printf("serialized int vector: \n%s\n", (char*)vi_as_str->c_str());
    
    vector<String*>* vs = deserialize_str_vector((char*)vs_as_str->c_str());
    vector<int>* vi = deserialize_int_vector((char*)vi_as_str->c_str());

    if (vs->size() != vi->size()) {
      printf("Invalid WordCountRower to deserialize.\n");
      exit(1);
    }
    
    for (int i = 0; i < vs->size(); i++) {
      string temp(vs->at(i)->c_str());
      wc->insert({temp, vi->at(i)});
    }

    wcr->word_counts = wc;
    return wcr;
  }
};
