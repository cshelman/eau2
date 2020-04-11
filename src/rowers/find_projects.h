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
    if (strlen(r) <= 2) {
        printf("Invalid rower instantiation\n");
        exit(1);
    }

    // printf("parsing: %s\n", r);

    char char_len[10];
    char_len[9] = '\0';
    size_t len = 0;
    memcpy(char_len, &r[1], 9);
    // printf("about to sscanf(%s)\n", char_len);
    if (1 != sscanf(char_len, "%zu", &len)) printf("bad sscanf 1\n");

    char char_plen[10];
    char_plen[9] = '\0';
    size_t plen = 0;
    memcpy(char_plen, &r[10], 9);
    // printf("about to sscanf(%s)\n", char_plen);
    if (1 != sscanf(char_plen, "%zu", &plen)) printf("bad sscanf 2\n");
    
    bool* uids = new bool[len];
    for (int i = 0; i < len; i++) {
      uids[i] = (r[(i/8) + 19] >> i % 8) & 1;
    }

    bool* pids = new bool[plen];
    for (int i = 0; i < plen; i++) {
        pids[i] = (r[(i/8) + 19 + len] >> i % 8) & 1;
    }

    user_ids = uids;
    project_ids = pids;
    size = len;
    p_size = plen;
  }

  FindProjectsRower(bool* uids, size_t s, size_t ps) {
    user_ids = uids;
    size = s;
    p_size = ps;
    project_ids = new bool[p_size];
    memset(project_ids, '\0', p_size);
  }
  
  /*
  DataFrame 3 commits:
    cols: PID(int) UID[writer](int) UID[committer](int)
  */
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

    delete other;
  }

  Rower* clone() {
    return new FindProjectsRower(user_ids, size, p_size);
  }

  char* serialize() {
    char* bitmap = new char[((size + p_size) / 8) + 20];
    memset(bitmap, '\0', ((size + p_size) / 8) + 20);
    bitmap[0] = '0';

    stringstream ss;
    ss << setw(9) << setfill('0') << to_string(size);
    ss << setw(9) << setfill('0') << to_string(p_size);
    string s(ss.str());

    char* len_as_c = (char*)s.c_str();

    memcpy(&bitmap[1], len_as_c, 18);
    
    for (int i = 0; i < size; i++) {
        bitmap[(i/8) + 19] |= (user_ids[i] << i % 8);
    }

    for (int i = 0; i < p_size; i++) {
        bitmap[(i/8) + 19 + size] |= (project_ids[i] << i % 8);
    }
    
    return bitmap;
  }

  Rower* deserialize(char* r) {
    return new FindProjectsRower(r);
  }
};
