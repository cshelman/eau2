#include "../src/dataframe.h"
#include "../src/string.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {

    char* col_types = new char[100];
    char* types = new char[4];
    types[0] = 'I';
    types[1] = 'F';
    types[2] = 'B';
    types[3] = 'S';

    for(int i = 0; i < 100; i++) {
      col_types[i] = types[i % 4];
    }

    Schema* s = new Schema();
    for(int i = 0; i < 100; i++) {
      s->add_column(col_types[i], nullptr);
    }

    DataFrame df(*s);

    for(size_t i = 0; i <  1000 * 1000; i++) {
      printf("starting row: %lu\n", i);
      Row* row = new Row(df.get_schema());
      for(size_t j = 0; j < 100; j++) {
        printf("\tstarting item: %lu\n", j);
        if(types[j%4] == 'I') {
          row->set(j,(int)(j + i));
        } else if(types[j%4] == 'F') {
          row->set(j, (float)(j + i));
        } else if(types[j%4] == 'B') {
          row->set(j, true);
        } else if(types[j%4] == 'S') {
          String* str = new String("WHYYY");
          row->set(j, str);
          delete str;
        }
      }
      df.add_row(*row);
      delete row;
    }
    
    df.print();
    delete[] col_types;
    delete[] types;
    delete s;
}
