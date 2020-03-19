#pragma once

#include "object.h"
#include "string.h"
#include "array.h"

/*************************************************************************
 * Schema::
 * A schema is a description of the contents of a data frame, the schema
 * knows the number of columns and number of rows, the type of each column,
 * optionally columns and rows can be named by strings.
 * The valid types are represented by the chars 'S', 'B', 'I' and 'F'.
 */
class Schema : public Object {
public:
    size_t num_cols_;
    size_t num_rows_;
    StringArray* types_;
    StringArray* col_names_;
    StringArray* row_names_;

    /** Copying constructor */
    Schema(Schema& from) {
        num_cols_ = from.num_cols_;
        num_rows_ = from.num_rows_;
        types_ = new StringArray(from.types_);
        col_names_ = new StringArray(from.col_names_);
        row_names_ = new StringArray(from.row_names_);
    }
 
    /** Create an empty schema **/
    Schema() {
        num_cols_ = 0;
        num_rows_ = 0;
        types_ = new StringArray();
        col_names_ = new StringArray();
        row_names_ = new StringArray();
    }
 
    /** Create a schema from a string of types. A string that contains
      * characters other than those identifying the four type results in
      * undefined behavior. The argument is external, a nullptr argument is
      * undefined. **/
    Schema(const char* types) {
        num_cols_ = strlen(types);
        num_rows_ = 0;
        types_ = new StringArray();
        col_names_ = new StringArray();
        row_names_ = new StringArray();

        for (int i = 0; i < strlen(types); i++) {
            String* str = new String(&types[i], 1);
            types_->add(str);
            col_names_->add(nullptr);
            delete str;
        }
    }

    ~Schema() {
        delete types_;
        delete col_names_;
        delete row_names_;
    }
 
    /** Add a column of the given type and name (can be nullptr), name
      * is external. Names are expectd to be unique, duplicates result
      * in undefined behavior. */
    void add_column(char typ, String* name) {
        num_cols_++;
        String* str_typ = new String(&typ, 1);
        types_->add(str_typ);
        col_names_->add(name);
    }

    /** Add a row with a name (possibly nullptr), name is external.  Names are
     *  expectd to be unique, duplicates result in undefined behavior. */
    void add_row(String* name) {
        num_rows_++;
        row_names_->add(name);
    }
 
    /** Return name of row at idx; nullptr indicates no name. An idx >= width
      * is undefined. */
    String* row_name(size_t idx) {
        return row_names_->get(idx);
    }

    /** Return name of column at idx; nullptr indicates no name given.
      *  An idx >= width is undefined.*/
    String* col_name(size_t idx) {
        return col_names_->get(idx);
    }

    /** Return type of column at idx. An idx >= width is undefined. */
    char col_type(size_t idx) {
        return types_->get(idx)->at(0);
    }

    /** Given a column name return its index, or -1. */
    int col_idx(const char* name) {
        if (name != nullptr) {
            String* str_name = new String(name, strlen(name));
            for (int i = 0; i < col_names_->get_len(); i++) {
                if (str_name->equals(col_names_->get(i))) {
                    return i;
                }
            }
        }
        return -1;
    }

    /** Given a row name return its index, or -1. */
    int row_idx(const char* name) {
        if (name != nullptr) {
            String* str_name = new String(name, strlen(name));
            for (int i = 0; i < row_names_->get_len(); i++) {
                if (str_name->equals(row_names_->get(i))) {
                    return i;
                }
            }
        }
        return -1;
    }

    /** The number of columns */
    size_t width() {
        return num_cols_;
    }

    /** The number of rows */
    size_t length() {
        return num_rows_;
    }
};