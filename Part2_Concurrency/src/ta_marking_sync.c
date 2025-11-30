// ta_marking_sync.c
#include "common.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <num_TAs>\n", argv[0]);
        return 1;
    }

    int num_TAs = atoi(argv[1]);
    if (num_TAs < 2) {
        printf("Number of TAs must be at least 2\n");
        return 1;
    }

    // Create shared memory
    int shmid = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    SharedData *shared = (SharedData *)shmat(shmid, NULL, 0);
    if (shared == (void *)-1) {
        perror("shmat");
        exit(1);
    }

    // Initialize shared memory
    shared->current_exam_index = 0;
    shared->rubric_version = 0;
    shared->reader_count = 0;
    read_rubric_from_file(shared->rubric);

    // Create semaphore set
    int semid = semget(IPC_PRIVATE, NUM_SEMS, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    // Initialize semaphores
    semctl(semid, SEM_EXAM_PILE,    SETVAL, 1);  // exam pile mutex
    semctl(semid, SEM_RUBRIC_WRITE, SETVAL, 1);  // rubric writer lock
    semctl(semid, SEM_READER_COUNT, SETVAL, 1);  // reader_count lock

    printf("=== Part 2.b: TA Marking WITH Synchronization ===\n");
    printf("Starting %d TAs with semaphore protection.\n\n", num_TAs);

    // Fork TA processes
    for (int ta_id = 0; ta_id < num_TAs; ta_id++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) { // Child TA process
            srand(time(NULL) ^ (getpid() << 16));

            while (1) {
                // Synchronized access to shared exam index
                sem_wait_op(semid, SEM_EXAM_PILE);
                int exam_index = shared->current_exam_index++;
                sem_signal_op(semid, SEM_EXAM_PILE);

                // 20 regular exams + 1 termination exam
                if (exam_index >= NUM_EXAMS + 1) {
                    break;
                }

                // Build exam filename
                char exam_filename[128];
                if (exam_index < NUM_EXAMS) {
                    sprintf(exam_filename, "../exam_files/exam_%04d.txt",
                            exam_index + 1);
                } else {
                    sprintf(exam_filename, "../exam_files/exam_9999.txt");
                }

                // Open exam file
                FILE *exam_file = fopen(exam_filename, "r+");
                if (!exam_file) {
                    fprintf(stderr,
                            "TA %d [PID %d] failed to open %s\n",
                            ta_id + 1, getpid(), exam_filename);
                    continue;
                }

                char student_num[16];
                if (fgets(student_num, sizeof(student_num), exam_file) == NULL) {
                    fclose(exam_file);
                    continue;
                }
                student_num[strcspn(student_num, "\n")] = 0;

                // Random exercise 1–5
                int exercise_to_mark = (rand() % NUM_EXERCISES) + 1;

                printf("TA %d [PID %d] marking EXAM: %s (Student %s) - Exercise %d\n",
                       ta_id + 1, getpid(), exam_filename, student_num,
                       exercise_to_mark);

                // Readers–writers: start reading rubric
                start_reading(semid, shared);
                printf("TA %d [PID %d] reading rubric for exercise %d: %s\n",
                       ta_id + 1, getpid(), exercise_to_mark,
                       shared->rubric[exercise_to_mark - 1]);
                usleep(500000 + (rand() % 500000)); // 0.5–1.0 s read time
                end_reading(semid, shared);

                // Simulate marking time
                usleep(500000 + (rand() % 1000000)); // 0.5–1.5 s

                // Append mark to exam file
                fseek(exam_file, 0, SEEK_END);
                fprintf(exam_file,
                        "Exercise %d: Marked by TA %d (PID %d)\n",
                        exercise_to_mark, ta_id + 1, getpid());
                fclose(exam_file);

                // Random chance to modify rubric (20%)
                if ((rand() % 100) < 20) {
                    printf("TA %d [PID %d] wants to modify rubric...\n",
                           ta_id + 1, getpid());

                    int rubric_line = rand() % NUM_EXERCISES;

                    // Exclusive writer access
                    start_writing(semid);
                    printf("TA %d [PID %d] modifying rubric line %d (exercise %d)...\n",
                           ta_id + 1, getpid(), rubric_line + 1, rubric_line + 1);

                    modify_rubric(shared, rubric_line);

                    sleep(3); // simulate ~3 s modification time
                    end_writing(semid);

                    printf("TA %d [PID %d] finished modifying rubric.\n",
                           ta_id + 1, getpid());
                }

                // Termination exam
                if (atoi(student_num) == TERMINATION_EXAM) {
                    printf("TA %d [PID %d] reached termination exam (9999), exiting.\n",
                           ta_id + 1, getpid());
                    shmdt(shared);
                    exit(0);
                }
            }

            shmdt(shared);
            exit(0);
        }
    }

    // Parent waits for all TAs
    while (wait(NULL) > 0)
        ;

    printf("\nAll TAs finished (synchronized using semaphores for rubric readers/writers!)\n");

    // Cleanup
    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID, 0);

    return 0;
}
