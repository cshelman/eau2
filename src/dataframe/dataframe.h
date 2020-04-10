#pragma once

#include "row.h"
#include "schema.h"
#include "column.h"
#include "row.h"
#include "../util/string.h"
#include <vector>
#include <thread>

using namespace std;

/****************************************************************************
 * DataFrame::
 *
 * A DataFrame is table composed of columns of equal length. Each column
 * holds values of the same type (I, S, B, F). A dataframe has a schema that
 * describes it.
 */
class DataFrame {
public:
    vector<Column*>* col_arr;
    Schema* schema_;

    /** Create a data frame with the same columns as the given df but with no rows or rownmaes */
    DataFrame(DataFrame& df) {
      col_arr = df.col_arr;
      schema_->types_ = df.schema_->types_;
    }

    /** Create a data frame from a schema and columns. All columns are created
      * empty. */
    DataFrame(Schema& schema) {
      col_arr = new vector<Column*>();
      schema_ = new Schema(schema);

      for (int i = 0; i < schema_->width(); i++) {
        char t = schema_->col_type(i);
        switch (t) {
          case 'S': {
            StringColumn* col = new StringColumn();
            col_arr->push_back(col); 
            break;
          }
          case 'B': {
            BoolColumn* col = new BoolColumn();
            col_arr->push_back(col); 
            break;
          }
          case 'I': {
            IntColumn* col = new IntColumn();
            col_arr->push_back(col); 
            break;
          }
          case 'F': {
            FloatColumn* col = new FloatColumn();
            col_arr->push_back(col); 
            break;
          }
          default:
            exit(1);
        }
        
      }
    }

    ~DataFrame() {
      delete col_arr;
      delete schema_;
    }

    /** Returns the dataframe's schema. Modifying the schema after a dataframe
      * has been created in undefined. */
    Schema& get_schema() {
      return *schema_;
    }

    /** Adds a column this dataframe, updates the schema, the new column
      * is external, and appears as the last column of the dataframe, the
      * name is optional and external. A nullptr column is undefined. */
    void add_column(Column* col) {
      col_arr->push_back(col->copy());
      schema_->add_column(col->get_type());
    }    

    /** Return the value at the given column and row. Accessing rows or
     *  columns out of bounds, or request the wrong type is undefined.*/
    int get_int(size_t col, size_t row) {
      IntColumn* int_col = static_cast<IntColumn*>(col_arr->at(col));
      return int_col->get(row);
    }

    bool get_bool(size_t col, size_t row) {
      BoolColumn* bool_col = static_cast<BoolColumn*>(col_arr->at(col));
      return bool_col->get(row);
    }
    
    float get_float(size_t col, size_t row) {
      FloatColumn* float_col = static_cast<FloatColumn*>(col_arr->at(col));
      return float_col->get(row);
    }
    
    String* get_string(size_t col, size_t row) {
      StringColumn* string_col = static_cast<StringColumn*>(col_arr->at(col));
      return string_col->get(row);
    }
  
    /** Return the offset of the given column name or -1 if no such col. */
    int get_col(String& col) {
      return schema_->col_idx(col.c_str());
    }

    /** Return the offset of the given row name or -1 if no such row. */
    int get_row(String& col) {
      return schema_->row_idx(col.c_str());
    }

    /** Set the value at the given column and row to the given value.
      * If the column is not of the right type or the indices are out of
      * bound, the result is undefined. */
    void set(size_t col, size_t row, int val) {
      IntColumn* int_col = static_cast<IntColumn*>(col_arr->at(col));
      int_col->set(row, val);
    }

    void set(size_t col, size_t row, bool val) {
      BoolColumn* bool_col = static_cast<BoolColumn*>(col_arr->at(col));
      bool_col->set(row, val);
    }

    void set(size_t col, size_t row, float val) {
      FloatColumn* float_col = static_cast<FloatColumn*>(col_arr->at(col));
      float_col->set(row, val);
    }

    void set(size_t col, size_t row, String* val) {
      StringColumn* string_col = static_cast<StringColumn*>(col_arr->at(col));
      string_col->set(row, val);
    }


    /** Set the fields of the given row object with values from the columns at
      * the given offset.  If the row is not form the same schema as the
      * dataframe, results are undefined.
      */
    void fill_row(size_t idx, Row& row) {
      for (int i = 0; i < col_arr->size(); i++) {
        char t = schema_->col_type(i);
        switch (t) {
          case 'S': {
            StringColumn* string_col = col_arr->at(i)->as_string();

            row.set(i, string_col->get(idx));
            break;
          }
          case 'B': {
            BoolColumn* bool_col = col_arr->at(i)->as_bool();
            row.set(i, bool_col->get(idx));
            break;
          }
          case 'I': {
            IntColumn* int_col = col_arr->at(i)->as_int();
            row.set(i, int_col->get(idx));
            break;
          }
          case 'F': {
            FloatColumn* float_col = col_arr->at(i)->as_float();
            row.set(i, float_col->get(idx));
            break;
          }
          default:
            exit(1);
        }
      }
    }

    /** Add a row at the end of this dataframe. The row is expected to have
     *  the right schema and be filled with values, otherwise undefined.  */
    void add_row(Row& row) {

      for (int i = 0; i < col_arr->size(); i++) {
        char t = schema_->col_type(i);
        switch (t) {
          case 'S': 
            col_arr->at(i)->as_string()->push_back(row.get_string(i));
            break;
          case 'B': 
            col_arr->at(i)->as_bool()->push_back(row.get_bool(i));
            break;
          case 'I': 
            col_arr->at(i)->as_int()->push_back(row.get_int(i));
            break;
          case 'F': 
            col_arr->at(i)->as_float()->push_back(row.get_float(i));
            break;
          default:
            exit(1);
        }
      }
      
      schema_->add_row(nullptr);
    }

    /** The number of rows in the dataframe. */
    size_t nrows() {
      return schema_->length();
    }

    /** The number of columns in the dataframe.*/
    size_t ncols() {
      return schema_->width();
    }

    /** Visit rows in order */
    void map(Rower& r) {
      for (int i = 0; i < nrows(); i++) {
        Row* cur_row = new Row(*schema_);
        fill_row(i, *cur_row);
        cur_row->set_idx(i);
        r.accept(*cur_row);
        delete cur_row;
      }
    }

    /** Create a new dataframe, constructed from rows for which the given Rower
      * returned true from its accept method. */
    DataFrame* filter(Rower& r) {
      DataFrame* new_frame = new DataFrame(*schema_);

      for (int i = 0; i < nrows(); i++) {
        Row* cur_row = new Row(*schema_);
        fill_row(i, *cur_row);
        if (r.accept(*cur_row)) {
          new_frame->add_row(*cur_row);
        }
        delete cur_row;
      }
      return new_frame;
    }

    /** Print the dataframe in SoR format to standard output. */
    void print() {
      Rower* r = new Rower();
      map(*r);
      delete r;
    }

    bool equals(DataFrame* dfin) {
      // check schema equality
      if (!schema_->equals(dfin->schema_)) {
        return false;
      }

      EqualityRower* rower = new EqualityRower();

      for (int i = 0; i < nrows(); i++) {
        Row* cur_row = new Row(*schema_);
        fill_row(i, *cur_row);
        cur_row->set_idx(i);

        Row* dfin_row = new Row(*dfin->schema_);
        fill_row(i, *dfin_row);
        dfin_row->set_idx(i);

        rower->accept(*cur_row, *dfin_row);
        delete cur_row;
        delete dfin_row;
      }

      bool is_equal = rower->is_equal;
      delete rower;
      return is_equal;
    }
    
    void apply_rower(Rower* r, size_t rower_num, size_t rower_rows, size_t num_rowers) {
        size_t end_row = 0;
        if ((rower_num + 1) * rower_rows < nrows()) {
            end_row = (rower_num + 1) * rower_rows;
        }
        else {
            end_row = nrows();
        }

        for (int i = rower_num * rower_rows; i < end_row; i++) {
            Row* cur_row = new Row(*schema_);
            fill_row(i, *cur_row);
            cur_row->set_idx(i);
            r->accept(*cur_row);
            delete cur_row;
        }
    }

    /** This method clones the Rower and executes the map in parallel. Join is
     *  used at the end to merge the results. */
    void pmap(Rower& r) {
        size_t rower_rows = 128;
        size_t num_rowers = (nrows() / rower_rows) + 1;

        thread* threads[num_rowers];
        Rower* rowers[num_rowers];
        for (int i = 0; i < num_rowers; i++) {
            if (i == 0) {
              rowers[i] = &r;
            } else {
              rowers[i] = r.clone();
            }
            threads[i] = new thread(&DataFrame::apply_rower, this, rowers[i], i, rower_rows, num_rowers);
        }

        for (int i = 0; i < num_rowers; i++) {
            threads[i]->join();
            if (i != 0) {
                rowers[0]->join_delete(rowers[i]);
            }
        }
    }
};