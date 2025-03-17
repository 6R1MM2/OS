
#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdlib>
#include <cstdio>

using namespace std;

struct SimClock {
    int seconds;
    int nanoseconds;
};

int main(int argc, char* argv[]) {
    if(argc != 3) {
        cerr << "Usage: " << argv[0] << " <sec_offset> <nano_offset>" << endl;
        return EXIT_FAILURE;
    }
    int sec_offset = atoi(argv[1]);
    int nano_offset = atoi(argv[2]);

    //Ggenerate the same key used by OSS
    key_t key = ftok("oss.cpp", 'S');
    if(key == -1) {
        perror("ftok failed");
        exit(EXIT_FAILURE);
    }


    int shmid = shmget(key, sizeof(SimClock), 0666);
    if(shmid == -1) {
        perror("shmget failed");
        exit(EXIT_FAILURE);
    }

    //attach
    SimClock* clock = (SimClock*) shmat(shmid, nullptr, 0);
    if(clock == (void*) -1) {
        perror("shmat failed");
        exit(EXIT_FAILURE);
    }

    //output clock
    cout << "WORKER: attached to shared memory clock" << endl;
    cout << "WORKER: current clock " 
         << clock->seconds << " seconds, " 
         << clock->nanoseconds << " nanoseconds" << endl;

    //
    int term_seconds = clock->seconds + sec_offset;
    int term_nanoseconds = clock->nanoseconds + nano_offset;
    if(term_nanoseconds >= 1000000000) {
        term_seconds += term_nanoseconds / 1000000000;
        term_nanoseconds %= 1000000000;
    }
    cout << "WORKER: time > " 
         << term_seconds << " seconds, " 
         << term_nanoseconds << " nanoseconds." << endl;

    // Detach
    if(shmdt(clock) == -1) {
        perror("shmdt failed");
    }
    return 0;
}
