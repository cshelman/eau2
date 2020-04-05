#pragma once

#include "../src/dataframe/schema.h"
#include "../src/dataframe/dataframe.h"
#include "../src/string.h"
#include <fstream>
#include <map>
#include <string>

using namespace std;

//read through the file
//create a new row in the dataframe for each word
//count the number of occurrences of each word in the df
//print the different words and their counts

DataFrame* parse_file(string filename) {
    Schema* s = new Schema();
    s->add_column('S');
    DataFrame* df = new DataFrame(*s);

     // filestream variable file 
    fstream file;
    string word;
  
    // opening file
    file.open(filename.c_str());
  
    // extracting words from the file
    while (file >> word) {
        Row* row = new Row(df->get_schema());
        string* str = new string((char*)word.c_str());

        char c = str->back();
        if (c == ',' || c == '.' || c == ';') {
            str->pop_back();
        }

        String* s = new String((char*)str->c_str());
        row->set(0, s);
        df->add_row(*row);
        delete str;
        delete s;
        delete row;
    } 

    return df;
}

class WordCountRower : public Rower {
public:
  map<string, int>* word_counts;

  WordCountRower() {
    word_counts = new map<string, int>();
  }
  
  bool accept(Row& r) {
    String* word = r.get_string(0);
    assert(word != nullptr);
    string* s = new string(word->c_str());
    
    if (word_counts->count(*s) > 0) {
        int count = word_counts->at(*s) + 1;
        word_counts->at(*s) = count;
    }
    else {
        word_counts->insert({*s, 1});
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
        // string* ns = new string(count.first.c_str());
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
      printf("%s\t|   count: %d\n", count.first.c_str(), count.second);
    }
  }
};

