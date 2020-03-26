#pragma once

#include <stdlib.h>
#include <stdio.h>

/**
 * StringArray: represents a resizable StringArray of strings.
 */
class StringArray {
  public:
    String** los;
    size_t cap;
    size_t size;

    /**
     * Constructs a new empty StringArray.
     */
    StringArray() {
      cap = 4;
      size = 0;
      los = new String*[cap];
    }

    /**
     * Constructs a new StringArray with the same type, values, and length
     * as in the given StringArray a.
     */
    StringArray(StringArray* a) {
      cap = a->cap;
      size = a->size;
      los = new String*[cap];
      for (int i = 0; i < size; i++) {
        los[i] = a->los[i]->clone();
      }
    }

    /**
     * Default destructor.
     */
    ~StringArray() {
      delete[] los;
      cap = 0;
      size = 0;
    }

    /**
     * Returns the length of this StringArray.
     */
    virtual size_t get_len() {
      return size;
    }

    /**
     * Returns the value stored at index i in this StringArray.
     * If the given i is < 0 or >= than the length, a nullptr is returned.
     */
    virtual String* get(size_t i) {
      if (i >= size) return nullptr;
      return los[i];
    }

    /**
     * Sets the given String o at index i in this StringArray.
     * If the given i is < 0 or >= than the length, return nullptr.
     * Else, the function will return the String that was previously at index i.
     */
    virtual String* set(String* o, size_t i) {
      if (i >= size) return nullptr;
      String* copy = los[i]->clone();
      los[i] = o;
      return copy;
    }

    /**
     * Determines if this StringArray is equal to a given StringArray o.
     */
    virtual bool equals(StringArray* o) {
      if (get_len() != o->get_len()) return false;

      for (int i = 0; i < get_len(); i++) {
        if (!get(i)->equals(o->get(i))) return false;
      }

      return true;
    }

    /**
    * Inserts the given String o at the given index.
    * If the given i is < 0 or >= than the length, nothing happens.
    */
    virtual void insert(String* o, size_t index) {
      if (index > get_len()) {
        return;
      }
      else if (get_len() >= cap) {
        cap *= 2;
        String** longer = new String*[cap];
        memcpy(longer, los, get_len() * sizeof(String*));
        delete los;
        los = longer;
      }

      if (get_len() == 0 || index == get_len()) {
        los[index] = o;
      }
      else {
        for (int i = get_len() + 1; i > index; i--) {
          los[i] = los[i - 1];
        }
        los[index] = o;
      }
      size++;
    }

    /**
    * Adds the given String o at the end of the StringArray.
    */
    virtual void add(String* o) {
      insert(o, get_len());
    }

    /**
    * Removes the String at index.
    */
    virtual void remove(size_t index) {
      if (index >= get_len()) {
        return;
      }
      else {
        for (int i = index; i < get_len() - 1; i++) {
          los[i] = los[i + 1];
        }
      }
      size--;
    }
};




class DoubleArray {
  public:
    double* lod;
    size_t cap;
    size_t size;

    /**
     * Constructs a new empty DoubleArray.
     */
    DoubleArray() {
      cap = 4;
      size = 0;
      lod = new double[cap];
    }

    /**
     * Constructs a new DoubleArray with the same type, values, and length
     * as in the given DoubleArray a.
     */
    DoubleArray(DoubleArray* a) {
      cap = a->cap;
      size = a->size;
      lod = new double[cap];
      for (int i = 0; i < size; i++) {
        lod[i] = a->lod[i];
      }
    }

    /**
     * Default destructor.
     */
    ~DoubleArray() {
      delete[] lod;
      cap = 0;
      size = 0;
    }

    /**
     * Returns the length of this DoubleArray.
     */
    virtual size_t get_len() {
      return size;
    }

    /**
     * Returns the value stored at index i in this DoubleArray.
     * If the given i is < 0 or >= than the length, crash.
     */
    virtual double get(size_t i) {
      if (i >= size) {
        printf("index out of bounds getting in DoubleArray\n");
        exit(1);
      }
      return lod[i];
    }

    /**
     * Sets the given String o at index i in this DoubleArray.
     * If the given i is < 0 or >= than the length, crash.
     * Else, the function will return the String that was previously at index i.
     */
    virtual double set(double o, size_t i) {
      if (i >= size) {
        printf("index out of bounds setting in DoubleArray\n");
        exit(1);
      }
      double copy = lod[i];
      lod[i] = o;
      return copy;
    }

    /**
     * Determines if this DoubleArray is equal to a given String o.
     */
    virtual bool equals(DoubleArray* o) {
      if (get_len() != o->get_len()) return false;

      for (int i = 0; i < get_len(); i++) {
        if (get(i) != o->get(i)) return false;
      }

      return true;
    }

    /**
    * Inserts the given String o at the given index.
    * If the given i is < 0 or >= than the length, nothing happens.
    */
    virtual void insert(double o, size_t index) {
      if (index > get_len()) {
        return;
      }
      else if (get_len() >= cap) {
        cap *= 2;
        double* longer = new double[cap];
        memcpy(longer, lod, get_len() * sizeof(double));
        delete lod;
        lod = longer;
      }

      if (get_len() == 0 || index == get_len()) {
        lod[index] = o;
      }
      else {
        for (int i = get_len() + 1; i > index; i--) {
          lod[i] = lod[i - 1];
        }
        lod[index] = o;
      }
      size++;
    }

    /**
    * Adds the given String o at the end of the DoubleArray.
    */
    virtual void add(double o) {
      insert(o, get_len());
    }

    /**
    * Removes the String at index.
    */
    virtual void remove(size_t index) {
      if (index >= get_len()) {
        return;
      }
      else {
        for (int i = index; i < get_len() - 1; i++) {
          lod[i] = lod[i + 1];
        }
      }
      size--;
    }
};