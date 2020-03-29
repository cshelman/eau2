#pragma once
#include "../src/serializer/buffer.h"
#include "../src/serializer/serial.h"

void test_serialize() {
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

    DataFrame* df = new DataFrame(*s);

    for(size_t i = 0; i < 3 * 1; i++) {
      Row* row = new Row(df->get_schema());
      for(size_t j = 0; j < 100; j++) {
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
      df->add_row(*row);
      delete row;
    }
    
    Buffer* buffer = new Buffer();
    serialize_dataframe(df, buffer);

    DataFrame* new_df = deserialize_dataframe(buffer->val);
    new_df->print();

    delete[] col_types;
    delete[] types;
    delete s;

    printf("serialize test: SUCCESS\n");
}