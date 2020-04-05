#pragma once

#include "../src/dataframe/schema.h"
#include "../src/dataframe/dataframe.h"
#include "../src/string.h"
#include "../src/serializer/serial.h"
#include <fstream>
#include <map>
#include <string>

using namespace std;

//read through the file
//create a new row in the dataframe for each word
//count the number of occurrences of each word in the df
//print the different words and their counts

DataFrame* parse_file(string filename) {
    int col_len = 1000;

    Schema* s = new Schema();
    s->num_rows_ = col_len;
    DataFrame* df = new DataFrame(*s);

     // filestream variable file 
    fstream file;
    string word;
  
    // opening file
    file.open(filename.c_str());
  
    // extracting words from the file
    int row_count = 0;
    StringColumn* sc = new StringColumn();
    while (file >> word) {        
        if (row_count >= col_len) {
          df->add_column(sc);
          delete sc;
          sc = new StringColumn();
          row_count = 0;
        }
        
        string* str = new string((char*)word.c_str());

        char c = str->back();
        if (c == ',' || c == '.' || c == ';') {
            str->pop_back();
        }

        String* s = new String((char*)str->c_str());
        sc->push_back(s);
        delete str;
        delete s;

        row_count++;
    } 

    String* test = new String("x");
    while (sc->size() < col_len) {
      sc->push_back(test);
    }
    df->add_column(sc);
    delete sc;

    return df;
}

class WordCountRower : public Rower {
public:
  map<string, int>* word_counts;

  WordCountRower() {
    word_counts = new map<string, int>();
  }
  
  bool accept(Row& r) {
    for (int i = 0; i < r.width(); i++) {
        String* word = r.get_string(i);
        assert(word != nullptr);
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
};

