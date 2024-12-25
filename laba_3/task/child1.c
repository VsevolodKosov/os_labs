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

    int cur = 0;
    while (shmaddr->message[cur] != '\0') {
        shmaddr->message[cur] = tolower(shmaddr->message[cur]);
        cur++;
    }

    shmaddr->flag = 2;

    if (shmdt(shmaddr) < 0)
        exit(EXIT_FAILURE);

    return 0;
}