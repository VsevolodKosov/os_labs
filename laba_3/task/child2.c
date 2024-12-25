#include "main.h"

int main() {
    int shmid;
    struct SharedData *shmaddr;

    shmid = shmget(SHM_KEY, sizeof(struct SharedData), 0666);
    if (shmid < 0) 
        exit(EXIT_FAILURE);

    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1)
        exit(EXIT_FAILURE);

    int cur = 0, write_index = 0;
    int prev_was_space = 0;  

    while (shmaddr->message[cur] != '\0') {
        if (shmaddr->message[cur] == ' ') {
            if (prev_was_space) {
                cur++;
                continue;
            }
            prev_was_space = 1; 
        } else {
            prev_was_space = 0;  
        }


        shmaddr->message[write_index++] = shmaddr->message[cur++];
    }


    shmaddr->message[write_index] = '\0';


    shmaddr->flag = 3;

  
    if (shmdt(shmaddr) < 0)
        exit(EXIT_FAILURE);

    return 0;
}
