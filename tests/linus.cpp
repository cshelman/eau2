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

    size_t BYTES_TO_READ = SIZE_MAX;
    // DataFrame* users_df = parseSor((char*)"data/users.ltgt", 0, BYTES_TO_READ);
    DataFrame* commits_df = parseSor((char*)"data/commits.ltgt", 0, BYTES_TO_READ);
    // DataFrame* projects_df = parseSor((char*)"data/projects.ltgt", 0, BYTES_TO_READ);

    // char temp;
    // printf("PRESS X TO PUT...");
    // cin >> temp;
    // printf("\n");
    
    Key* commits_key = new Key("commits");
    serv->put(commits_key, commits_df);

    // printf("PRESS X TO COUNT...");
    // cin >> temp;
    // printf("\n");

    int ARR_SIZE = 3000000;
    int LINUS = 4967; // Linus' ID
    int DEGREE = 1;

    bool* uids = new bool[ARR_SIZE]; //set linus to true
    uids[LINUS] = true;


    for (int i = 0; i < DEGREE; i++) {
        printf("\nDEGREE IS %d\n", i+1);
        FindProjectsRower* fpr = new FindProjectsRower(uids, ARR_SIZE, ARR_SIZE);
        serv->set_rower(fpr);
        serv->run_rower(commits_key, fpr);
        bool* pids = fpr->project_ids;
        printf("\tDone getting pids\n");

        // for (int j = 0; j < ARR_SIZE; j++) {
        //     if (pids[j]) {
        //         printf("pid %d\n", j);
        //     }
        // }
        // printf("\n\n");

        FindUsersRower* fur = new FindUsersRower(pids, ARR_SIZE, ARR_SIZE);
        serv->set_rower(fur);
        serv->run_rower(commits_key, fur);
        uids = fur->user_ids;
        printf("\tDone getting uids\n");
        
        size_t count = 0;
        for (int j = 0; j < ARR_SIZE; j++) {
            if (uids[j]) {
                count++;
            }
        }
        printf("\tDegree %d users: %ld\n", i, count);

        delete fpr;
        delete fur;
    }

    
    // printf("fpr->p_size: %ld\n", fpr->p_size);
    // for (int i = 0; i < fpr->p_size; i++) {
    //     printf("%d", find_projects[i]);
    // }
    // printf("\n");

    // printf("PRESS X TO SHUTDOWN...");
    // cin >> temp;
    // printf("\n");

    serv->shutdown();

    delete serv;
    delete commits_key;
    // delete users_df;
    // delete projects_df;
    delete commits_df;
    delete uids;

    printf("Linus test: SUCCESS\n");
    return 0;
}