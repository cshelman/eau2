//lang::CwC
#pragma once

#include "object.h"
#include "string.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

using namespace std;

/**
 * Array: represents a resizable array of objects.
 */
class Array : public Object {
  public:
    Object*** lobj;
    size_t cap;
    size_t inner_size;
    size_t size;

    Array(Array* arr) {
      cap = arr->cap;
      size = 0;
      inner_size = arr->inner_size;
      lobj = new Object**[cap];
      
      for (int i = 0; i < cap; i++) {
        lobj[i] = new Object*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        lobj[outer_idx][inner_idx] = NULL;
      }

      for(int i = 0; i < arr->get_len(); i++) {
        add(arr->get(i));
      }
    }

    /**
     * Constructs a new empty array.
     */
    Array() : Object() {
      cap = 4;
      size = 0;
      inner_size = 100;
      lobj = new Object**[cap];
      
      for (int i = 0; i < cap; i++) {
          lobj[i] = new Object*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        lobj[outer_idx][inner_idx] = NULL;
      }
    }

    /**
     * Default destructor.
     */
    ~Array() {
      for(int i = 0; i < cap; i++) {
        for(int j = 0; j < inner_size; j++) {
          delete lobj[i][j];
        }
        delete[] lobj[i];
      }
      delete[] lobj;
      cap = 0;
      size = 0;
    }

    /**
     * Returns the length of this Array.
     */
    virtual size_t get_len() {
      return size;
    }

    /**
     * Returns the value stored at index i in this Array.
     * If the given i is < 0 or >= than the length, a nullptr is returned.
     */
    virtual Object* get(size_t i) {
      if (i >= size) return nullptr;
      
      // integer division should truncate this
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;
      return lobj[outer_idx][inner_idx];
    }

    /**
     * Sets the given Object o at index i in this Array.
     * Returns success or failure
     */
    virtual bool set(Object *o, size_t i) {
      if (i >= size) return false;
      
      // integer division should truncate this
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;
      lobj[outer_idx][inner_idx] = o->clone();

      return true;
    }

    /**
     * Determines if this Array is equal to a given Object o.
     */
    virtual bool equals(Object* o) {
      Array* a = dynamic_cast<Array*>(o);
      if (a == nullptr) return false;
      if (get_len() != a->get_len()) return false;

      for (int i = 0; i < get_len(); i++) {
        if (!get(i)->equals(a->get(i))) return false;
      }

      return true;
    }

    /**
    * Adds the given object o at the end of the array.
    */
    virtual void add(Object* o) {
      if (get_len() >= cap * inner_size) {
        cap *= 2;
        Object*** longer = new Object**[cap];

        for (int i = 0; i < cap; i++) {
          if (i < cap/2) {
            longer[i] = lobj[i];
          } else {
            longer[i] = new Object*[inner_size];
            for (int j = 0; j < inner_size; j++) {
              longer[i][j] = NULL;
            }
          }
        }

        //memcpy(longer, loi, (cap/2) * sizeof(int**));
        
        //free memory
        delete[] lobj;

        lobj = longer;
      }
      // integer division should truncate this
      size_t outer_idx = get_len() / inner_size;
      size_t inner_idx = get_len() % inner_size;
      lobj[outer_idx][inner_idx] = o->clone();

      size++;
    }

    virtual Array* copy() {
      Array* arr = new Array(this);
      return arr;
    }
};



/**
 * BoolArray: represents a resizable array of objects.
 */
class BoolArray : public Object {
  public:
    bool*** lob;
    size_t cap;
    size_t inner_size;
    size_t size;

    BoolArray(BoolArray* bool_arr) {
      cap = bool_arr->cap;
      size = 0;
      inner_size = bool_arr->inner_size;
      lob = new bool**[cap];
      
      for (int i = 0; i < cap; i++) {
        lob[i] = new bool*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        lob[outer_idx][inner_idx] = NULL;
      }

      for(int i = 0; i < bool_arr->get_len(); i++) {
        add(bool_arr->get(i));
      }
    }

    /**
     * Constructs a new empty array.
     */
    BoolArray() : Object() {
      cap = 4;
      size = 0;
      inner_size = 100;
      lob = new bool**[cap];
      
      for (int i = 0; i < cap; i++) {
          lob[i] = new bool*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        lob[outer_idx][inner_idx] = NULL;
      }
    }

    /**
     * Default destructor.
     */
    ~BoolArray() {
      // printf("new bool delete\n");
      for(int i = 0; i < cap; i++) {
        for(int j = 0; j < inner_size; j++) {
          delete lob[i][j];
        }
        delete[] lob[i];
      }
      delete[] lob;
      cap = 0;
      size = 0;
    }

    /**
     * Returns the length of this Array.
     */
    virtual size_t get_len() {
      return size;
    }

    /**
     * Returns the value stored at index i in this Array.
     * If the given i is < 0 or >= than the length, a nullptr is returned.
     */
    virtual bool* get(size_t i) {
      if (i >= size) return nullptr;
      
      // integer division should truncate this
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;
      return lob[outer_idx][inner_idx];
    }

    /**
     * Sets the given Object o at index i in this Array.
     * Returns success or failure
     */
    virtual bool set(bool* o, size_t i) {
      bool* new_data = new bool;

      if(o == nullptr) {
        new_data = nullptr;
      } else {
        *new_data = *o;
      }
      
      if (i >= size) return false;
      
      // integer division should truncate this
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;
      lob[outer_idx][inner_idx] = new_data;

      return true;
    }

    /**
     * Determines if this Array is equal to a given Object o.
     */
    virtual bool equals(Object* o) {
      BoolArray* a = dynamic_cast<BoolArray*>(o);
      if (a == nullptr) return false;
      if (get_len() != a->get_len()) return false;

      for (int i = 0; i < get_len(); i++) {
        if (get(i) == a->get(i)) return false;
      }

      return true;
    }

    /**
    * Adds the given object o at the end of the array.
    */
    virtual void add(bool* o) {
      bool* new_data = new bool;

      if(o == nullptr) {
        new_data = nullptr;
      } else {
        *new_data = *o;
      }

      if (get_len() >= cap * inner_size) {
        cap *= 2;
        bool*** longer = new bool**[cap];

        for (int i = 0; i < cap; i++) {
          if (i < cap/2) {
            longer[i] = lob[i];            
          } else {
            longer[i] = new bool*[inner_size];
            for (int j = 0; j < inner_size; j++) {
              longer[i][j] = NULL;
            }
          }
        }

        //memcpy(longer, loi, (cap/2) * sizeof(int**));
        
        //free memory
        delete[] lob;

        lob = longer;
      }
      // integer division should truncate this
      size_t outer_idx = get_len() / inner_size;
      size_t inner_idx = get_len() % inner_size;
      lob[outer_idx][inner_idx] = new_data;

      size++;
    }

    virtual BoolArray* copy() {
      BoolArray* bool_arr = new BoolArray(this);
      return bool_arr;
    }
};

/**
 * IntArray: represents a resizable array of objects.
 */
class IntArray : public Object {
  public:
    int*** loi;
    size_t cap;
    size_t inner_size;
    size_t size;

    IntArray(IntArray* int_arr) {
      cap = int_arr->cap;
      size = 0;
      inner_size = int_arr->inner_size;
      loi = new int**[cap];
      
      for (int i = 0; i < cap; i++) {
        loi[i] = new int*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        loi[outer_idx][inner_idx] = NULL;
      }

      for(int i = 0; i < int_arr->get_len(); i++) {
        add(int_arr->get(i));
      }
    }

    /**
     * Constructs a new empty array.
     */
    IntArray() : Object() {
      cap = 4;
      size = 0;
      inner_size = 100;
      loi = new int**[cap];
      
      for (int i = 0; i < cap; i++) {
          loi[i] = new int*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        loi[outer_idx][inner_idx] = NULL;
      }
    }

    /**
     * Default destructor.
     */
    ~IntArray() {
      // printf("new int delete\n");
      for(int i = 0; i < cap; i++) {
        for(int j = 0; j < inner_size; j++) {
          delete loi[i][j];
        }
        delete[] loi[i];
      }
      delete[] loi;
      cap = 0;
      size = 0;
    }

    /**
     * Returns the length of this Array.
     */
    virtual size_t get_len() {
      return size;
    }

    /**
     * Returns the value stored at index i in this Array.
     * If the given i is < 0 or >= than the length, a nullptr is returned.
     */
    virtual int* get(size_t i) {
      if (i >= size) return nullptr;
      
      // integer division should truncate this
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;
      return loi[outer_idx][inner_idx];
    }

    /**
     * Sets the given Object o at index i in this Array.
     * Returns success or failure
     */
    virtual bool set(int* o, size_t i) {
      int* new_data = new int;

      if(o == nullptr) {
        new_data = nullptr;
      } else {
        *new_data = *o;
      }

      if (i >= size) return false;
      
      // integer division should truncate this
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;
      loi[outer_idx][inner_idx] = new_data;

      return true;
    }

    /**
     * Determines if this Array is equal to a given Object o.
     */
    virtual bool equals(Object* o) {
      IntArray* a = dynamic_cast<IntArray*>(o);
      if (a == nullptr) return false;
      if (get_len() != a->get_len()) return false;

      for (int i = 0; i < get_len(); i++) {
        if (get(i) != a->get(i)) return false;
      }

      return true;
    }

    /**
    * Adds the given object o at the end of the array.
    */
    virtual void add(int* o) {
      int* new_data = new int;

      if(o == nullptr) {
        new_data = nullptr;
      } else {
        *new_data = *o;
      }

      if (get_len() >= cap * inner_size) {
        cap *= 2;
        int*** longer = new int**[cap];

        for (int i = 0; i < cap; i++) {
          if (i < cap/2) {
            longer[i] = loi[i];            
          } else {
            longer[i] = new int*[inner_size];
            for (int j = 0; j < inner_size; j++) {
              longer[i][j] = NULL;
            }
          }
        }

        //memcpy(longer, loi, (cap/2) * sizeof(int**));
        
        //free memory
        delete[] loi;

        loi = longer;
      }

      // integer division should truncate this
      size_t outer_idx = get_len() / inner_size;
      size_t inner_idx = get_len() % inner_size;
      loi[outer_idx][inner_idx] = new_data;
      size++;
    }

    virtual IntArray* copy() {
      IntArray* int_arr = new IntArray(this);
      return int_arr;
    }
};

/**
 * FloatArray: represents a resizable array of objects.
 */
class FloatArray : public Object {
  public:
    float*** lof;
    size_t cap;
    size_t inner_size;
    size_t size;

    FloatArray(FloatArray* float_arr) {
      cap = float_arr->cap;
      size = 0;
      inner_size = float_arr->inner_size;
      lof = new float**[cap];
      
      for (int i = 0; i < cap; i++) {
        lof[i] = new float*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        lof[outer_idx][inner_idx] = NULL;
      }

      for(int i = 0; i < float_arr->get_len(); i++) {
        add(float_arr->get(i));
      }
    }

    /**
     * Constructs a new empty array.
     */
    FloatArray() : Object() {
      cap = 4;
      size = 0;
      inner_size = 100;
      lof = new float**[cap];
      
      for (int i = 0; i < cap; i++) {
          lof[i] = new float*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        lof[outer_idx][inner_idx] = NULL;
      }
    }

    /**
     * Default destructor.
     */
    ~FloatArray() {
      // printf("new float delete\n");
      for(int i = 0; i < cap; i++) {
        for(int j = 0; j < inner_size; j++) {
          delete lof[i][j];
        }
        delete[] lof[i];
      }
      delete[] lof;
      cap = 0;
      size = 0;
    }

    /**
     * Returns the length of this Array.
     */
    virtual size_t get_len() {
      return size;
    }

    /**
     * Returns the value stored at index i in this Array.
     * If the given i is < 0 or >= than the length, a nullptr is returned.
     */
    virtual float* get(size_t i) {
      if (i >= size) return nullptr;
      
      // integer division should truncate this
      
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;

      return lof[outer_idx][inner_idx];
    }

    /**
     * Sets the given Object o at index i in this Array.
     * Returns success or failure
     */
    virtual bool set(float* o, size_t i) {
      float* new_data = new float;

      if(o == nullptr) {
        new_data = nullptr;
      } else {
        *new_data = *o;
      }

      if (i >= size) return false;
      
      // integer division should truncate this
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;
      lof[outer_idx][inner_idx] = new_data;

      return true;
    }

    /**
     * Determines if this Array is equal to a given Object o.
     */
    virtual bool equals(Object* o) {
      FloatArray* a = dynamic_cast<FloatArray*>(o);
      if (a == nullptr) return false;
      if (get_len() != a->get_len()) return false;

      for (int i = 0; i < get_len(); i++) {
        if (get(i) - a->get(i) > .0001) return false;
      }

      return true;
    }

    /**
    * Adds the given object o at the end of the array.
    */
    virtual void add(float* o) {
      float* new_data = new float;

      if(o == nullptr) {
        new_data = nullptr;
      } else {
        *new_data = *o;
      }

      if (get_len() >= cap * inner_size) {
        cap *= 2;
        float*** longer = new float**[cap];

        for (int i = 0; i < cap; i++) {
          if (i < cap/2) {
            longer[i] = lof[i];
          } else {
            longer[i] = new float*[inner_size];
            for (int j = 0; j < inner_size; j++) {
              longer[i][j] = NULL;
            }
          }
        }

        //memcpy(longer, loi, (cap/2) * sizeof(int**));
        
        //free memory
        delete[] lof;

        lof = longer;
      }

      // integer division should truncate this
      size_t outer_idx = get_len() / inner_size;
      size_t inner_idx = get_len() % inner_size;
      lof[outer_idx][inner_idx] = new_data;

      size++;
    }

    virtual FloatArray* copy() {
      FloatArray* float_arr = new FloatArray(this);
      return float_arr;
    }
};

/**
 * StringArray: represents a resizable array of objects.
 */
class StringArray : public Object {
  public:
    String*** los;
    size_t cap;
    size_t inner_size;
    size_t size;

    StringArray(StringArray* str_arr) {
      cap = str_arr->cap;
      size = 0;
      inner_size = str_arr->inner_size;
      los = new String**[cap];
      
      for (int i = 0; i < cap; i++) {
        los[i] = new String*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        los[outer_idx][inner_idx] = NULL;
      }

      for(int i = 0; i < str_arr->get_len(); i++) {
        add(str_arr->get(i));
      }
    }

    /**
     * Constructs a new empty array.
     */
    StringArray() : Object() {
      cap = 4;
      size = 0;
      inner_size = 100;
      los = new String**[cap];
      
      for (int i = 0; i < cap; i++) {
          los[i] = new String*[inner_size];
      }

      for (int i = 0; i < cap * inner_size; i++) {
        size_t outer_idx = i / inner_size;
        size_t inner_idx = i % inner_size;
        los[outer_idx][inner_idx] = NULL;
      }
    }

    /**
     * Default destructor.
     */
    ~StringArray() {
      //printf("new string delete\n");
      for(int i = 0; i < cap; i++) {
        for(int j = 0; j < inner_size; j++) {
          if(los[i][j] != nullptr) {
            // printf("value: %s\n", los[i][j]->c_str());
            // printf("addr: %p\n", los[i][j]);
          }
          
          delete los[i][j];
        }
        // printf("inner array: %p\n", los[i]);
        delete[] los[i];
      }
      delete[] los;
      
      cap = 0;
      size = 0;
    }

    /**
     * Returns the length of this Array.
     */
    virtual size_t get_len() {
      return size;
    }

    /**
     * Returns the value stored at index i in this Array.
     * If the given i is < 0 or >= than the length, a nullptr is returned.
     */
    virtual String* get(size_t i) {
      if (i >= size) return nullptr;
      
      // integer division should truncate this
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;
      return los[outer_idx][inner_idx];
    }

    /**
     * Sets the given Object o at index i in this Array.
     * Returns success or failure
     */
    virtual bool set(String* o, size_t i) {
      if (i >= size) return false;
      
      // integer division should truncate this
      size_t outer_idx = i / inner_size;
      size_t inner_idx = i % inner_size;
      
      String* str;
      if(o != nullptr) {
        str = new String(o->c_str());
      } else {
        str = nullptr;
      }
      los[outer_idx][inner_idx] = str;

      return true;
    }

    /**
     * Determines if this Array is equal to a given Object o.
     */
    virtual bool equals(Object* o) {
      StringArray* a = dynamic_cast<StringArray*>(o);
      if (a == nullptr) return false;
      if (get_len() != a->get_len()) return false;

      for (int i = 0; i < get_len(); i++) {
        if (!get(i)->equals(a->get(i))) return false;
      }

      return true;
    }

    /**
    * Adds the given object o at the end of the array.
    */
    virtual void add(String* o) {
      if (get_len() >= cap * inner_size) {
        cap *= 2;
        String*** longer = new String**[cap];

        for (int i = 0; i < cap; i++) {
          if (i < cap/2) {
            longer[i] = los[i];            
          } else {
            longer[i] = new String*[inner_size];
            for (int j = 0; j < inner_size; j++) {
              longer[i][j] = NULL;
            }
          }
        }

        //memcpy(longer, loi, (cap/2) * sizeof(int**));
        
        //free memory
        delete[] los;

        los = longer;
      }

      // integer division should truncate this
      size_t outer_idx = get_len() / inner_size;
      size_t inner_idx = get_len() % inner_size;

      String* str;
      if(o != nullptr) {
        str = new String(o->c_str());
      } else {
        str = nullptr;
      }
      
      los[outer_idx][inner_idx] = str;

      size++;
    }

    virtual StringArray* copy() {
      StringArray* str_arr = new StringArray(this);
      return str_arr;
    }
};