#pragma once

#include "string.h"
#include "object.h"
#include <cstdarg>
#include <vector>
#include "array.h"

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
class Column : public Object {
public:
    char type;

    /** Type converters: Return same column under its actual type, or
     *  nullptr if of the wrong type.  */
    virtual IntColumn* as_int() {
        return nullptr;
    }

    virtual BoolColumn* as_bool() {
        return nullptr;
    }

    virtual FloatColumn* as_float() {
        return nullptr;
    }

    virtual StringColumn* as_string() {
        return nullptr;
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
        arr = col->arr;
    }

    IntColumn() {
        type = 'I';
        arr = new vector<int>();
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
        delete arr;
    }

    int get(size_t idx) {
        return arr->at(idx);
    }

    IntColumn* as_int() {
        return this;
    }

    void push_back(int val) {
        arr->push_back(val);
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
        arr = col->arr;
    }

    FloatColumn() {
        type = 'F';
        arr = new vector<float>();
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
        delete arr;
    }

    float get(size_t idx) {
        return arr->at(idx);
    }

    FloatColumn* as_float() {
        return this;
    }

    void push_back(float val) {
        arr->push_back(val);
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
        arr = col->arr;
    }

    BoolColumn() {
        type = 'B';
        arr = new vector<bool>();
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
        delete arr;
    }

    void push_back(bool val) {
        arr->push_back(val);
    }

    bool get(size_t idx) {
        return arr->at(idx);
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
        arr = col->arr;
    }

    StringColumn() {
        type = 'S';
        arr = new vector<String*>();
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
        delete arr;
    }

    StringColumn* as_string() {
        return this;
    }

    void push_back(String* val) {
        arr->push_back(val);
    }

    /** Returns the string at idx; undefined on invalid idx.*/
    String* get(size_t idx) {
        return arr->at(idx);
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

/**
 * ColumnArray: represents a resizable array of objects.
 */
// class ColumnArray : public Object {
//   public:
//     Column*** loc;
//     size_t cap;
//     size_t inner_size;
//     size_t size;

//     ColumnArray(ColumnArray* col_arr) {
//       cap = col_arr->cap;
//       size = 0;
//       inner_size = col_arr->inner_size;
//       loc = new Column**[cap];
      
//       for (int i = 0; i < cap; i++) {
//         loc[i] = new Column*[inner_size];
//       }

//       for (int i = 0; i < cap * inner_size; i++) {
//         size_t outer_idx = i / inner_size;
//         size_t inner_idx = i % inner_size;
//         loc[outer_idx][inner_idx] = NULL;
//       }

//       for(int i = 0; i < col_arr->get_len(); i++) {
//         Column* copy = col_arr->get(i)->copy();
//         add(copy);
//       }
//     }

//     /**
//      * Constructs a new empty array.
//      */
//     ColumnArray() : Object() {
//       cap = 4;
//       size = 0;
//       inner_size = 100;
//       loc = new Column**[cap];
      
//       for (int i = 0; i < cap; i++) {
//           loc[i] = new Column*[inner_size];
//       }

//       for (int i = 0; i < cap * inner_size; i++) {
//         size_t outer_idx = i / inner_size;
//         size_t inner_idx = i % inner_size;
//         loc[outer_idx][inner_idx] = NULL;
//       }
//     }

//     /**
//      * Default destructor.
//      */
//     ~ColumnArray() {
//       for(int i = 0; i < cap; i++) {
//         for(int j = 0; j < inner_size; j++) {
//           delete loc[i][j];
//         }
//         delete[] loc[i];
//       }
//       delete[] loc;
//       cap = 0;
//       size = 0;
//     }

//     /**
//      * Returns the length of this Array.
//      */
//     virtual size_t get_len() {
//       return size;
//     }

//     /**
//      * Returns the value stored at index i in this Array.
//      * If the given i is < 0 or >= than the length, a nullptr is returned.
//      */
//     virtual Column* get(size_t i) {
//       if (i >= size) return nullptr;
      
//       // integer division should truncate this
//       size_t outer_idx = i / inner_size;
//       size_t inner_idx = i % inner_size;
//       return loc[outer_idx][inner_idx];
//     }

//     /**
//      * Sets the given Object o at index i in this Array.
//      * Returns success or failure
//      */
//     virtual bool set(Column* o, size_t i) {
//       if (i >= size) return false;
      
//       // integer division should truncate this
//       size_t outer_idx = i / inner_size;
//       size_t inner_idx = i % inner_size;
//       loc[outer_idx][inner_idx] = o;

//       return true;
//     }

//     /**
//      * Determines if this Array is equal to a given Object o.
//      */
//     virtual bool equals(Object* o) {
//       ColumnArray* a = dynamic_cast<ColumnArray*>(o);
//       if (a == nullptr) return false;
//       if (get_len() != a->get_len()) return false;

//       for (int i = 0; i < get_len(); i++) {
//         if (!get(i)->equals(a->get(i))) return false;
//       }

//       return true;
//     }

//     /**
//     * Adds the given object o at the end of the array.
//     */
//     virtual void add(Column* o) {
//       if (get_len() >= cap * inner_size) {
//         cap *= 2;
//         Column*** longer = new Column**[cap];

//         for (int i = 0; i < cap; i++) {
//           if (i < cap/2) {
//             longer[i] = loc[i];            
//           } else {
//             longer[i] = new Column*[inner_size];
//             for (int j = 0; j < inner_size; j++) {
//               longer[i][j] = NULL;
//             }
//           }
//         }

//         //memcpy(longer, loi, (cap/2) * sizeof(int**));
        
//         //free memory
//         delete[] loc;

//         loc = longer;
//       }
//       // integer division should truncate this
//       size_t outer_idx = get_len() / inner_size;
//       size_t inner_idx = get_len() % inner_size;
//       loc[outer_idx][inner_idx] = o;

//       size++;
//     }
// };