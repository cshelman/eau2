#pragma once

#include <vector>
#include "object.h"
#include "string.h"
#include "array.h"
#include "schema.h"

using namespace std;

/*****************************************************************************
 * Fielder::
 * A field vistor invoked by Row.
 */
class Fielder : public Object {
public:

    /** Called before visiting a row, the argument is the row offset in the
    dataframe. */
    virtual void start(size_t r) {
      printf("row: %d\t", (int)r);
    }

    /** Called for fields of the argument's type with the value of the field. */
    virtual void accept(bool b) {
      printf("bool: %d ", b);
    }

    virtual void accept(float f) {
      printf("float: %.3f ", f);
    }
    virtual void accept(int i) {
      printf("int: %d ", i);
    }

    virtual void accept(String* s) {
      printf("string: %s ", s->c_str());
    }

    /** Called when all fields have been seen. */
    virtual void done() {
      printf("\n");
    }
};
 
/*************************************************************************
 * Row::
 *
 * This class represents a single row of data constructed according to a
 * dataframe's schema. The purpose of this class is to make it easier to add
 * read/write complete rows. Internally a dataframe hold data in columns.
 * Rows have pointer equality.
 */
class Row : public Object {
public:

    vector<String*>* types_;
    vector<int*>* int_arr;
    vector<bool*>* bool_arr;
    vector<float*>* float_arr;
    vector<String*>* string_arr;
    size_t row_index;

    /** Build a row following a schema. */
    Row(Schema& scm) {
      types_ = scm.types_;
      
      int_arr = new vector<int*>();
      bool_arr = new vector<bool*>();
      float_arr = new vector<float*>();
      string_arr = new vector<String*>();

      for(int i = 0; i < scm.types_->size(); i++) {
        int_arr->push_back(nullptr);
        bool_arr->push_back(nullptr);
        float_arr->push_back(nullptr);
        string_arr->push_back(nullptr);
      }
    }

    ~Row() {
      // printf("deleting row++++++++++++++\n");
      delete types_;
      delete int_arr;
      delete bool_arr;
      delete float_arr;
      delete string_arr;
    }

    /** Setters: set the given column with the given value. Setting a column with
      * a value of the wrong type is undefined. */
    void set(size_t col, int val) {
      int* new_int = new int(val);
      int_arr->at(col) = new_int;
    }

    void set(size_t col, float val) {
      float* new_float = new float(val);
      float_arr->at(col) = new_float;
    }

    void set(size_t col, bool val) {
      bool* new_bool = new bool(val);
      bool_arr->at(col) = new_bool;
    }

    /** Acquire ownership of the string. */
    void set(size_t col, String* val) {
      string_arr->at(col) = val->clone();
    }

    /** Set/get the index of this row (ie. its position in the dataframe. This is
     *  only used for informational purposes, unused otherwise */
    void set_idx(size_t idx) {
      row_index = idx;
    }

    size_t get_idx() {
      return row_index;
    }

    /** Getters: get the value at the given column. If the column is not
      * of the requested type, the result is undefined. */
    int get_int(size_t col) {
      return *(int_arr->at(col));
    }

    bool get_bool(size_t col) {
      return *(bool_arr->at(col));
    }

    float get_float(size_t col) {
      return *(float_arr->at(col));
    }

    String* get_string(size_t col) {
      return string_arr->at(col);
    }

    /** Number of fields in the row. */
    size_t width() {
      return types_->size();
    }

     /** Type of the field at the given position. An idx >= width is  undefined. */
    char col_type(size_t idx) {
      return types_->at(idx)->at(0);
    }

    /** Given a Fielder, visit every field of this row. The first argument is
      * index of the row in the dataframe.
      * Calling this method before the row's fields have been set is undefined. */
    void visit(size_t idx, Fielder& f) {
      f.start(idx);
      for(int i = 0; i < types_->size(); i++) {
        if(types_->at(i)->at(0) == 'I') {
          f.accept(this->get_int(i));
        } else if(types_->at(i)->at(0) == 'S') {
          f.accept(this->get_string(i));
        } else if(types_->at(i)->at(0) == 'F') {
          f.accept(this->get_float(i));
        } else if(types_->at(i)->at(0) == 'B') {
          f.accept(this->get_bool(i));
        }
      }
      f.done();
    }
};
 
/*******************************************************************************
 *  Rower::
 *  An interface for iterating through each row of a data frame. The intent
 *  is that this class should subclassed and the accept() method be given
 *  a meaningful implementation. Rowers can be cloned for parallel execution.
 */
class Rower : public Object {
public:
    /** This method is called once per row. The row object is on loan and
        should not be retained as it is likely going to be reused in the next
        call. The return value is used in filters to indicate that a row
        should be kept. */
    virtual bool accept(Row& r) {
      Fielder* fielder = new Fielder();
      r.visit(r.get_idx(), *fielder);
      delete fielder;
      return true;
    }

    /** Once traversal of the data frame is complete the rowers that were
        split off will be joined.  There will be one join per split. The
        original object will be the last to be called join on. The join method
        is reponsible for cleaning up memory. */
    virtual void join_delete(Rower* other) {
      delete other;
    }

    virtual Rower* clone() {
      return new Rower();
    }
};