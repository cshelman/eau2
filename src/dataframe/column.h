#pragma once

#include "../util/string.h"
#include <cstdarg>
#include <vector>

using namespace std;

class IntColumn;
class StringColumn;
class FloatColumn;
class BoolColumn;

/**************************************************************************
 * Column ::
 * Represents one column of a data frame which holds values of a single type.
 * This abstract class defines methods overriden in subclasses. There is
 * one subclass per element type. Columns are mutable, equality is pointer
 * equality. */
class Column {
public:
    char type;

    /** Type converters: Return same column under its actual type, or
     *  nullptr if of the wrong type. */
    virtual IntColumn* as_int() {
        printf("invalid as_int\n");
        exit(1);
    }

    virtual BoolColumn* as_bool() {
        printf("invalid as_bool\n");
        exit(1);
    }

    virtual FloatColumn* as_float() {
        printf("invalid as_float\n");
        exit(1);
    }

    virtual StringColumn* as_string() {
        printf("invalid as_string\n");
        exit(1);
    }

    /** Type appropriate push_back methods. Calling the wrong method is
      * undefined behavior. **/
    virtual void push_back(int val) {
        printf("invalid int push back\n");
        exit(1);
    }

    virtual void push_back(bool val) {
        printf("invalid bool push back\n");
        exit(1);
    }

    virtual void push_back(float val) {
        printf("invalid float push back\n");
        exit(1);
    }

    virtual void push_back(String* val) {
        printf("invalid val push back\n");
        exit(1);
    }

    virtual void append(Column* val) {
        printf("invalid col append\n");
        exit(1);
    }

    virtual Column* get_subset(int start, int end) {
        printf("invalid get subset\n");
        exit(1);
    }

    /** Returns the number of elements in the column. */
    virtual size_t size() = 0;

    /** Return the type of this column as a char: 'S', 'B', 'I' and 'F'. */
    char get_type() {
        return type;
    }

    virtual Column* copy() = 0;
};
 
/*************************************************************************
 * IntColumn::
 * Holds int values.
 */
class IntColumn : public Column {
public:
    vector<int>* arr;

    IntColumn(IntColumn* col) {
        type = col->type;
        arr = new vector<int>(*col->arr);
    }

    IntColumn() {
        type = 'I';
        arr = new vector<int>();
    }

    IntColumn(vector<int>* iv) {
        type = 'I';
        arr = new vector<int>(*iv);
    }

    IntColumn(int n, ...) {
        type = 'I';
        arr = new vector<int>();

        va_list args;
        va_start(args, n);

        for (int i = 0; i < n; i++) {
            arr->push_back(va_arg(args, int));
        }

        va_end(args);
    }

    ~IntColumn() {
        arr->clear();
        delete arr;
    }

    int get(size_t idx) {
        return arr->at(idx);
    }

    Column* get_subset(int start, int end) {
        vector<int>::const_iterator first = arr->begin() + start;
        vector<int>::const_iterator last = arr->begin() + end;

        vector<int>* iv = new vector<int>(first, last);
        IntColumn* ic = new IntColumn(iv);
        delete iv;
        return ic;
    }

    IntColumn* as_int() {
        return this;
    }

    void push_back(int val) {
        arr->push_back(val);
    }

    void append(Column* c) {
        IntColumn* ic = c->as_int();
        arr->insert(arr->end(), ic->arr->begin(), ic->arr->end());
    }

    /** Set value at idx. An out of bound idx is undefined.  */
    void set(size_t idx, int val) {
        arr->at(idx) = val;
    }

    size_t size() {
        return arr->size();
    }

    Column* copy() {
        IntColumn* int_col = new IntColumn(this);
        return int_col;
    }
};
 
/*************************************************************************
 * FloatColumn::
 * Holds float values.
 */
class FloatColumn : public Column {
public:
    vector<float>* arr;

    FloatColumn(FloatColumn* col) {
        type = col->type;
        arr = new vector<float>(*col->arr);
    }

    FloatColumn() {
        type = 'F';
        arr = new vector<float>();
    }

    FloatColumn(vector<float>* fv) {
        type = 'F';
        arr = new vector<float>(*fv);
    }

    FloatColumn(int n, ...) {
        type = 'F';
        arr = new vector<float>();

        va_list args;
        va_start(args, n);

        for (int i = 0; i < n; i++) {
            arr->push_back(va_arg(args, float));
        }

        va_end(args);
    }

    ~FloatColumn() {
        arr->clear();
        delete arr;
    }

    float get(size_t idx) {
        return arr->at(idx);
    }

    Column* get_subset(int start, int end) {
        vector<float>::const_iterator first = arr->begin() + start;
        vector<float>::const_iterator last = arr->begin() + end;

        vector<float>* fv = new vector<float>(first, last);
        FloatColumn* fc = new FloatColumn(fv);
        delete fv;
        return fc;
    }

    FloatColumn* as_float() {
        return this;
    }

    void push_back(float val) {
        arr->push_back(val);
    }

    void append(Column* c) {
        FloatColumn* fc = c->as_float();
        arr->insert(arr->end(), fc->arr->begin(), fc->arr->end());
    }

    /** Set value at idx. An out of bound idx is undefined.  */
    void set(size_t idx, float val) {
        arr->at(idx) = val;
    }

    size_t size() {
        return arr->size();
    }

    Column* copy() {
        FloatColumn* float_col = new FloatColumn(this);
        return float_col;
    }
};

/*************************************************************************
 * BoolColumn::
 * Holds bool values.
 */
class BoolColumn : public Column {
public:
    vector<bool>* arr;

    BoolColumn(BoolColumn* col) {
        type = col->type;
        arr = new vector<bool>(*col->arr);
    }

    BoolColumn() {
        type = 'B';
        arr = new vector<bool>();
    }

    BoolColumn(vector<bool>* bv) {
        type = 'B';
        arr = new vector<bool>(*bv);
    }

    BoolColumn(int n, ...) {
        type = 'B';
        arr = new vector<bool>();

        va_list args;
        va_start(args, n);

        for (int i = 0; i < n; i++) {
            arr->push_back(va_arg(args, bool));
        }

        va_end(args);
    }

    ~BoolColumn() {
        arr->clear();
        delete arr;
    }

    void push_back(bool val) {
        arr->push_back(val);
    }

    void append(Column* c) {
        BoolColumn* bc = c->as_bool();
        arr->insert(arr->end(), bc->arr->begin(), bc->arr->end());
    }

    bool get(size_t idx) {
        return arr->at(idx);
    }

    Column* get_subset(int start, int end) {
        vector<bool>::const_iterator first = arr->begin() + start;
        vector<bool>::const_iterator last = arr->begin() + end;

        vector<bool>* bv = new vector<bool>(first, last);
        BoolColumn* bc = new BoolColumn(bv);
        delete bv;
        return bc;
    }

    BoolColumn* as_bool() {
        return this;
    }

    /** Set value at idx. An out of bound idx is undefined.  */
    void set(size_t idx, bool val) {
        arr->at(idx) = val;
    }

    size_t size() {
        return arr->size();
    }

    Column* copy() {
        BoolColumn* bool_col = new BoolColumn(this);
        return bool_col;
    }
};

/*************************************************************************
 * StringColumn::
 * Holds string pointers. The strings are external.  Nullptr is a valid
 * value.
 */
class StringColumn : public Column {
public:
    vector<String*>* arr;

    StringColumn(StringColumn* col) {
        type = col->type;
        arr = new vector<String*>();
        for (int i = 0; i < col->arr->size(); i++) {
            if (col->arr->at(i) != nullptr) {
                arr->push_back(col->arr->at(i)->clone());
            }
            else {
                arr->push_back(nullptr);
            }
        }
    }

    StringColumn() {
        type = 'S';
        arr = new vector<String*>();
    }

    StringColumn(vector<String*>* sv) {
        type = 'S';
        arr = new vector<String*>();
        for (int i = 0; i < sv->size(); i++) {
            if (sv->at(i) != nullptr) {
                arr->push_back(sv->at(i)->clone());
            }
            else {
                arr->push_back(nullptr);
            }
        }
    }

    StringColumn(int n, ...) {
        type = 'S';
        arr = new vector<String*>();

        va_list args;
        va_start(args, n);

        for (int i = 0; i < n; i++) {
            String* s = new String(va_arg(args, String*)->c_str());
            arr->push_back(s);
        }

        va_end(args);
    }

    ~StringColumn() {
        for (int i = 0; i < arr->size(); i++) {
            delete arr->at(i);
        }
        delete arr;
    }

    StringColumn* as_string() {
        return this;
    }

    void push_back(String* val) {
        if (val == nullptr) {
            arr->push_back(nullptr);
        }
        else {
            arr->push_back(val->clone());
        }
    }

    void append(Column* c) {
        StringColumn* sc = c->as_string();
        arr->insert(arr->end(), sc->arr->begin(), sc->arr->end());
    }

    /** Returns the string at idx; undefined on invalid idx.*/
    String* get(size_t idx) {
        return arr->at(idx);
    }
    
    Column* get_subset(int start, int end) {
        vector<String*>::const_iterator first = arr->begin() + start;
        vector<String*>::const_iterator last = arr->begin() + end;

        vector<String*>* sv = new vector<String*>(first, last);
        StringColumn* sc = new StringColumn();
        delete sv;
        return sc;
    }

    /** Acquire ownership fo the string.  Out of bound idx is undefined. */
    void set(size_t idx, String* val) {
        arr->at(idx) = val;
    }

    size_t size() {
        return arr->size();
    }

    Column* copy() {
        StringColumn* str_col = new StringColumn(this);
        return str_col;
    }
};