#include "../src/util/sys.h"
#include "../src/util/data_adapter.h"
#include "../src/network/network_server.h"
#include "../src/network/server.h"
#include "../src/dataframe/dataframe.h"
#include "../src/rowers/find_projects.h"
#include "../src/rowers/find_users.h"

int main(int argc, char** argv) {
    NetworkServer* net_serv = new NetworkServer(argv[1], atoi(argv[2]));
    Server* serv = new Server(net_serv);
    
    // ------------------- DEGREES OF LINUS CONFIG VALUES ---------------------
    size_t BYTES_TO_READ = 10000000;
    int LINUS = 4967;
    int DEGREE = 4;
    char* COMMITS_PATH = (char*)"data/commits.ltgt";
    char* USERS_PATH = (char*)"data/users.ltgt";
    char* PROJECTS_PATH = (char*)"data/projects.ltgt";
    // ------------------------------------------------------------------------

    size_t uid_size = 0;
    size_t pid_size = 0;
    DataFrame* commits_df = parseSor(COMMITS_PATH, 0, BYTES_TO_READ, &uid_size, &pid_size);
    Key* commits_key = new Key("commits");
    serv->put(commits_key, commits_df);
    delete commits_df;
    
    DataFrame* users_df = parseSor(USERS_PATH, 0, BYTES_TO_READ);
    Key* users_key = new Key("users");
    serv->put(users_key, users_df);
    delete users_df;

    DataFrame* projects_df = parseSor(PROJECTS_PATH, 0, BYTES_TO_READ);
    Key* projects_key = new Key("projects");
    serv->put(projects_key, projects_df);
    delete projects_df;

    uid_size++;
    pid_size++;

    bool* uids = new bool[uid_size];
    memset(uids, '\0', uid_size);
    uids[LINUS] = true;

    for (int i = 0; i < DEGREE; i++) {
        printf("\nSTART DEGREE %d\n", i+1);
        FindProjectsRower* fpr = new FindProjectsRower(uids, uid_size, pid_size);
        serv->set_rower(fpr);
        serv->run_rower(commits_key, fpr);
        bool* pids = fpr->project_ids;

        FindUsersRower* fur = new FindUsersRower(pids, pid_size, uid_size);
        serv->set_rower(fur);
        serv->run_rower(commits_key, fur);
        memcpy(uids, fur->user_ids, uid_size * sizeof(bool));
        
        size_t count = 0;
        for (int j = 0; j < uid_size; j++) {
            if (uids[j]) {
                count++;
            }
        }
        
        printf("\tDegree %d: %ld users found\n", i+1, count);
        delete fpr;
        delete fur;
    }

    serv->shutdown();

    delete serv;
    delete net_serv;
    delete commits_key;
    delete users_key;
    delete projects_key;

    printf("Linus: SUCCESS\n");
    return 0;
}