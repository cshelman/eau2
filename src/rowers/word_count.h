#pragma once

#include "../dataframe/dataframe.h"
#include "../string.h"
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
            printf("editing existing count for %s: \n", s->c_str());
            int count = word_counts->at(*s) + 1;
            word_counts->at(*s) = count;
        }
        else {
            printf("inserting new count for %s: \n", s->c_str());
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
};
