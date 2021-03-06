#pragma once

#include "../dataframe/dataframe.h"
#include "../util/string.h"
#include <stdio.h>
#include <map>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

class FindProjectsRower : public Rower {
public:
  bool* user_ids;
  bool* project_ids;
  size_t size;
  size_t p_size;

  FindProjectsRower(char* r) {
    string* sr = new string(r);
    int start_pos = sr->find(":") + 1;
    int end_pos = sr->find(":", start_pos);

    size = end_pos - start_pos;
    user_ids = new bool[size];
    for (int i = start_pos; i < end_pos; i++) {
      user_ids[i - start_pos] = atoi(sr->substr(i, 1).c_str());
    }

    p_size = sr->size() - end_pos - 1;
    project_ids = new bool[p_size];
    for (int i = end_pos + 1; i < sr->size(); i++) {
      project_ids[i - end_pos - 1] = atoi(sr->substr(i, 1).c_str());
    }
    delete sr;
  }

  FindProjectsRower(bool* uids, size_t s, size_t ps) {
    size = s;
    user_ids = new bool[size];
    memcpy(user_ids, uids, size * sizeof(bool));

    p_size = ps;
    project_ids = new bool[p_size];
    memset(project_ids, '\0', p_size);
  }

  ~FindProjectsRower() {
    delete[] user_ids;
    delete[] project_ids;
  }
  
  bool accept(Row& r) {
    if (user_ids[r.get_int(1)] || user_ids[r.get_int(2)]) {
        project_ids[r.get_int(0)] = true;
    }
    return true;
  }

  void join_delete(Rower* other) {
    FindProjectsRower* r = dynamic_cast<FindProjectsRower*>(other);
    for (int i = 0; i < r->p_size; i++) {
        project_ids[i] = project_ids[i] || r->project_ids[i];
    }

    delete r;
  }

  Rower* clone() {
    return new FindProjectsRower(user_ids, size, p_size);
  }

  string* serialize() {

    string* ss = new string("0");
    ss->append(":");

    for (int i = 0; i < size; i++) {
        ss->append(to_string(user_ids[i]));
    }
    
    ss->append(":");

    for (int i = 0; i < p_size; i++) {
        ss->append(to_string(project_ids[i]));
    }

    return ss;
  }

  Rower* deserialize(char* r) {
    return new FindProjectsRower(r);
  }
};
