#include "../src/dataframe/dataframe.h"
#include "../src/util/string.h"

#include <stdlib.h>
#include <stdio.h>

void test_get_schema() {
    Schema* s = new Schema();
    s->add_column('I');
    s->add_column('I');
    s->add_column('B');
    s->add_column('B');
    s->add_column('S');
    s->add_column('F');
    s->add_column('S');

    DataFrame* df = new DataFrame(*s);
    Schema* ns = &df->get_schema();

    assert(s->equals(ns));

    delete s;
    delete ns;
}

void test_add_column() {
    Schema* s = new Schema();
    DataFrame* df = new DataFrame(*s);

    IntColumn* ic = new IntColumn();
    FloatColumn* fc = new FloatColumn();
    BoolColumn* bc = new BoolColumn();
    StringColumn* sc = new StringColumn();

    df->add_column(ic);
    df->add_column(fc);
    df->add_column(bc);
    df->add_column(sc);

    assert(df->ncols() == 4);
    assert(df->get_schema().width() == 4);
    assert(df->get_schema().col_type(0) == 'I');

    delete s;
    delete df;
    delete ic;
    delete fc;
    delete bc;
    delete sc;
}

void test_large_dataframe() {
    int NUM_ROWS = 100000;
    int NUM_COLS = 100;

    char* col_types = new char[NUM_COLS];
    char* types = new char[4];
    types[0] = 'I';
    types[1] = 'F';
    types[2] = 'B';
    types[3] = 'S';

    for (int i = 0; i < NUM_COLS; i++) {
      col_types[i] = types[i % 4];
    }

    Schema* s = new Schema();
    for (int i = 0; i < NUM_COLS; i++) {
      s->add_column(col_types[i]);
    }

    DataFrame df(*s);

    for (size_t i = 0; i < NUM_ROWS; i++) {
      Row* row = new Row(df.get_schema());
      for (size_t j = 0; j < NUM_COLS; j++) {
        if (types[j%4] == 'I') {
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
    s->num_rows_ = NUM_ROWS;

    assert(df.nrows() == NUM_ROWS);
    assert(df.ncols() == NUM_COLS);
    assert(s->equals(&df.get_schema()));
    
    delete[] col_types;
    delete[] types;
    delete s;
}

int main(int argc, char **argv) {
    test_get_schema();
    test_add_column();
    test_large_dataframe();

    printf("dataframe test SUCCESS\n");
}
