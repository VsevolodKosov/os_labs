#include "main.h"

int main() {
    int shmid;
    struct SharedData *shmaddr;
    int child1, child2;

    shmid = shmget(SHM_KEY, sizeof(struct SharedData), IPC_CREAT | 0666);
    if (shmid < 0)
        exit(EXIT_FAILURE);

    shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (void *)-1)
        exit(EXIT_FAILURE);

    read(STDIN_FILENO, shmaddr->message, sizeof(shmaddr->message));
    shmaddr->flag = 1;


    child1 = fork();
    if (child1 == -1)
        exit(EXIT_FAILURE);

    if (child1 == 0) {
        execl("./out/child1", "child1", NULL);
        exit(EXIT_FAILURE);
    }

    waitpid(child1, NULL, 0);

    while (shmaddr->flag != 2) {
        printf("Parent process waiting...\n");
    }

    child2 = fork();
    if (child2 == -1) {
        exit(EXIT_FAILURE);
    }

    if (child2 == 0) {
        execl("./out/child2", "child2", NULL);
        exit(EXIT_FAILURE);
    }
    waitpid(child2, NULL, 0);

    printf("%s\n", shmaddr->message);

    if (shmdt(shmaddr) < 0)
        exit(EXIT_FAILURE);

    if (shmctl(shmid, IPC_RMID, NULL) < 0)
        exit(EXIT_FAILURE);

    return 0;
}