#pragma once

#include "../dataframe/dataframe.h"
#include "../util/string.h"
#include <map>
#include <string>
#include <sstream>
#include <iomanip>
#include <bitset>

using namespace std;

class FindUsersRower : public Rower {
public:
  bool* project_ids;
  bool* user_ids;
  size_t size;
  size_t u_size;

  FindUsersRower(char* r) {

    string* sr = new string(r);
    int start_pos = sr->find(":") + 1;
    int end_pos = sr->find(":", start_pos);

    size = end_pos - start_pos;
    project_ids = new bool[size];
    for (int i = start_pos; i < end_pos; i++) {
      project_ids[i - start_pos] = atoi(sr->substr(i, 1).c_str());
    }

    u_size = sr->size() - end_pos - 1;
    user_ids = new bool[u_size];
    for (int i = end_pos + 1; i < sr->size(); i++) {
      user_ids[i - end_pos - 1] = atoi(sr->substr(i, 1).c_str());
    }

    delete sr;
  }

  FindUsersRower(bool* pids, size_t s, size_t us) {
    project_ids = pids;
    size = s;
    u_size = us;
    user_ids = new bool[u_size];
    memset(user_ids, '\0', u_size);
  }
  
  /*
    DataFrame 3 commits:
    cols: PID(int) UID[writer](int) UID[committer](int)
  */
  bool accept(Row& r) {
    if (project_ids[r.get_int(0)]) {
        user_ids[r.get_int(1)] = true;
        user_ids[r.get_int(2)] = true;
    }

    return true;
  }

  void join_delete(Rower* other) {
    FindUsersRower* r = dynamic_cast<FindUsersRower*>(other);
    for (int i = 0; i < r->u_size; i++) {
        user_ids[i] = user_ids[i] || r->user_ids[i];
    }

    delete other;
  }

  Rower* clone() {
    return new FindUsersRower(project_ids, size, u_size);
  }

  string* serialize() {
    // set 1st byte for the type
    string* ss = new string("1");
    ss->append(":");

    for (int i = 0; i < size; i++) {
        ss->append(to_string(project_ids[i]));
    }
    
    ss->append(":");

    for (int i = 0; i < u_size; i++) {
        ss->append(to_string(user_ids[i]));
    }

    return ss;
  }

  Rower* deserialize(char* r) {
    return new FindUsersRower(r);    
  }
};
