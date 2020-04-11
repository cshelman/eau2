#pragma once

#include "../dataframe/dataframe.h"
#include "../util/string.h"
#include <map>
#include <string>
#include <sstream>
#include <iomanip>

using namespace std;

class FindUsersRower : public Rower {
public:
  bool* project_ids;
  bool* user_ids;
  size_t size;
  size_t u_size;

  FindUsersRower(char* r) {
    if (strlen(r) <= 2) {
        printf("Invalid rower instantiation\n");
        exit(1);
    }

    printf("deserializing %s\n", r);

    char char_len[10];
    char_len[9] = '\0';
    size_t len = 0;
    memcpy(char_len, &r[1], 9);
    if (1 != sscanf(char_len, "%zu", &len)) printf("bad sscanf 1\n");
    printf("len: %ld\n", len);

    char char_ulen[10];
    char_ulen[9] = '\0';
    size_t ulen = 0;
    memcpy(char_ulen, &r[10], 9);
    if (1 != sscanf(char_ulen, "%zu", &ulen)) printf("bad sscanf 2\n");
    printf("ulen: %ld\n", ulen);
    
    bool* pids = new bool[len];
    for (int i = 0; i < len; i++) {
      pids[i] = (r[(i/8) + 19] >> i % 8) & 1;
      printf("adding pid: %d\n", pids[i]);
    }

    bool* uids = new bool[ulen];
    for (int i = 0; i < ulen; i++) {
        uids[i] = (r[(i/8) + 19 + len] >> i % 8) & 1;
        printf("adding uid: %d\n", uids[i]);
    }

    user_ids = uids;
    project_ids = pids;
    size = len;
    u_size = ulen;
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

  char* serialize() {
    char* bitmap = new char[((size + u_size) / 8) + 25];
    memset(bitmap, '\0', ((size + u_size) / 8) + 20);
    bitmap[0] = '0';
    bitmap[((size + u_size) / 8) + 24] = 'x';

    stringstream ss;
    ss << setw(9) << setfill('0') << to_string(size);
    ss << setw(9) << setfill('0') << to_string(u_size);
    string s(ss.str());

    char* len_as_c = (char*)s.c_str();

    memcpy(&bitmap[1], len_as_c, 18);
    
    for (int i = 0; i < size; i++) {
        bitmap[(i/8) + 19] |= (project_ids[i] << i % 8);
    }

    for (int i = 0; i < u_size; i++) {
        bitmap[(i/8) + 19 + size] |= (user_ids[i] << i % 8);
        printf("serialized: %d\n", (bitmap[(i/8) + 19 + size] >> i % 8) & 1);
    }
    
    return bitmap;
  }

  Rower* deserialize(char* r) {
    return new FindUsersRower(r);    
  }
};
