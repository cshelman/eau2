#pragma once

#include "../src/rowers/word_count.h"
#include "../src/dataframe/schema.h"
#include "../src/dataframe/dataframe.h"
#include "../src/util/string.h"
#include "../src/serializer/serial.h"
#include <fstream>
#include <map>
#include <string>
#include <algorithm>

using namespace std;

//read through the file
//create a new row in the dataframe for each word
//count the number of occurrences of each word in the df
//print the different words and their counts

DataFrame* parse_file(string filename) {
    int col_len = 500;

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

        transform(str->begin(), str->end(), str->begin(), ::tolower);
        String* s = new String((char*)str->c_str());
        sc->push_back(s);
        delete str;
        delete s;

        row_count++;
    } 

    while (sc->size() < col_len) {
      sc->push_back(nullptr);
    }
    df->add_column(sc);
    delete sc;

    return df;
}


