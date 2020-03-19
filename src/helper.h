#pragma once
//lang::Cpp

#include "dataframe.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <chrono>
#include <fstream>
#include <sstream>

using namespace std::chrono;
using namespace std;

class Clock {
public:
  high_resolution_clock::time_point start;
  high_resolution_clock::time_point end;

  // Timing functions
  void clock_start() {
    start = high_resolution_clock::now();
  }

  void clock_stop() {
    end = high_resolution_clock::now();
  }

  size_t get_time() {
    duration<double, milli> duration = end - start;
    return duration.count();
  }
};

/** Helper class providing some C++ functionality and convenience
 *  functions. This class has no data, constructors, destructors or
 *  virtual functions. Inheriting from it is zero cost.
 */
class Sys {
 public:


  // Printing functions
  Sys& p(char* c) { std::cout << c; return *this; }
  Sys& p(bool c) { std::cout << c; return *this; }
  Sys& p(float c) { std::cout << c; return *this; }  
  Sys& p(int i) { std::cout << i;  return *this; }
  Sys& p(size_t i) { std::cout << i;  return *this; }
  Sys& p(const char* c) { std::cout << c;  return *this; }
  Sys& p(char c) { std::cout << c;  return *this; }
  Sys& pln() { std::cout << "\n";  return *this; }
  Sys& pln(int i) { std::cout << i << "\n";  return *this; }
  Sys& pln(char* c) { std::cout << c << "\n";  return *this; }
  Sys& pln(bool c) { std::cout << c << "\n";  return *this; }  
  Sys& pln(char c) { std::cout << c << "\n";  return *this; }
  Sys& pln(float x) { std::cout << x << "\n";  return *this; }
  Sys& pln(size_t x) { std::cout << x << "\n";  return *this; }
  Sys& pln(const char* c) { std::cout << c << "\n";  return *this; }

  // Copying strings
  char* duplicate(const char* s) {
    char* res = new char[strlen(s) + 1];
    strcpy(res, s);
    return res;
  }
  char* duplicate(char* s) {
    char* res = new char[strlen(s) + 1];
    strcpy(res, s);
    return res;
  }

  // Function to terminate execution with a message
  void exit_if_not(bool b, char* c) {
    if (b) return;
    p("Exit message: ").pln(c);
    exit(-1);
  }
  
  // Definitely fail
//  void FAIL() {
  void myfail(){
    pln("Failing");
    exit(1);
  }

  // Some utilities for lightweight testing
  void OK(const char* m) { pln(m); }
  void t_true(bool p) { if (!p) myfail(); }
  void t_false(bool p) { if (p) myfail(); }
};
