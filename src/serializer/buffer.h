#pragma once

#include <string.h>

#define INITIAL_SIZE 128

class Buffer {
public:
    char* val;
    size_t cap;
    size_t size;

    /* Default constructor
     */
    Buffer() {
        val = new char[INITIAL_SIZE];
        memset(val, '\0', INITIAL_SIZE + 1);
        cap = INITIAL_SIZE;
        size = 0;
    }

    /* Default destructor
     */
    ~Buffer() {
        delete[] val;
        cap = 0;
        size = 0;
    }

    /* Steals a given char* and adds it to the buffer
     * Resizes the buffer as needed
     */
    void add(char* s) {
        size_t len = strlen(s);
        if (size + len >= cap) {
            cap *= 2;
            // if doubling the cap is not enough...
            if (cap < size + len) {
                cap = size + len + 1;
            }
            char* longer = new char[cap];
            memset(longer, '\0', cap + 1);
            memcpy(longer, val, size * sizeof(char));
            delete[] val;
            val = longer;
        }
        
        memcpy(&val[size], s, len * sizeof(char));

        size += len;
    }

    void add(const char* s) {
        char temp[strlen(s) + 1];
        strcpy(temp, s);
        add(temp);
    }
};