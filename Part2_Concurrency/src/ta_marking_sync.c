// ta_marking_sync.c

#include "common.h"

// Semaphore wrappers for simplicity
void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}
void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_TAs>\n", argv[0]);
        return 1;
    }

    int num_TAs = atoi(argv[1]);
    if (num_TAs < 2 || num_TAs > MAX_TAS) {
        printf("Number of TAs must be between 2 and %d\n", MAX_TAS);
        return 1;
    }

    // Semaphores: (rubric_semaphore = 1 for writers)
    int rubric_sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);
    semctl(rubric_sem, 0, SETVAL, 1); // initialized to 1

    for (int i = 0; i < num_TAs; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }
        if (pid == 0) { // child TA process
            srand(getpid());

            for (int exam_num = 1; exam_num <= 21; exam_num++) {
                char exam_filename[64];
                if (exam_num <= 20)
                    sprintf(exam_filename, "../exam_files/exam_%04d.txt", exam_num);
                else
                    sprintf(exam_filename, "../exam_files/exam_9999.txt");
                FILE *exam = fopen(exam_filename, "r+");
                if (!exam) continue;
                char student_num[10];
                fgets(student_num, sizeof(student_num), exam); // Read student number
                fseek(exam, 0, SEEK_SET);

                // Read rubric concurrently
                FILE *rubric = fopen("../rubric.txt", "r");
                if (!rubric) continue;
                printf("TA %d [PID %d] marking EXAM: %s (Student %s)\n", i+1, getpid(), exam_filename, student_num);

                int sleep_time = rand() % 2 + 1;
                sleep(sleep_time);

                // Random chance to modify rubric, protected by semaphore for writers
                if ((rand() % 10) < 2) {
                    printf("TA %d [PID %d] wants to modify rubric...\n", i+1, getpid());
                    sem_wait(rubric_sem);
                    printf("TA %d [PID %d] modifying rubric file...\n", i+1, getpid());
                    sleep(1); // Simulate rubric modification
                    sem_signal(rubric_sem);
                }

                fclose(exam);
                fclose(rubric);

                // Signal finish if 9999
                if (atoi(student_num) == 9999) {
                    printf("TA %d [PID %d] reached termination exam (9999), exiting.\n", i+1, getpid());
                    break;
                }
            }
            exit(0);
        }
    }

    // Parent waits for all children
    while (wait(NULL) > 0);
    semctl(rubric_sem, 0, IPC_RMID, 0);
    printf("All TAs finished (synchronized using semaphore for rubric writers!)\n");
    return 0;
}
