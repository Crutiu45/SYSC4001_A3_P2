
#include "common.h"

static void build_exam_filename(char *buf, size_t sz, int idx) {
    if (idx == MAX_EXAMS - 1) {
        snprintf(buf, sz, "../exam_files/exam_9999.txt");
    } else {
        snprintf(buf, sz, "../exam_files/exam_%04d.txt", idx + 1);
    }
}

static int load_rubric(Rubric *rubric) {
    FILE *f = fopen("../rubric.txt", "r");
    if (!f) {
        perror("fopen ../rubric.txt");
        return -1;
    }
    for (int i = 0; i < NUM_QUESTIONS; ++i) {
        if (!fgets(rubric->lines[i], RUBRIC_LINE_LEN, f)) {
            rubric->lines[i][0] = '\0';
        }
    }
    fclose(f);
    return 0;
}

static int save_rubric(const Rubric *rubric) {
    FILE *f = fopen("../rubric.txt", "w");
    if (!f) {
        perror("fopen ../rubric.txt for write");
        return -1;
    }
    for (int i = 0; i < NUM_QUESTIONS; ++i) {
        fputs(rubric->lines[i], f);
        size_t len = strlen(rubric->lines[i]);
        if (len == 0 || rubric->lines[i][len - 1] != '\n') {
            fputc('\n', f);
        }
    }
    fclose(f);
    return 0;
}

static int load_exam(ExamState *exam, int idx) {
    char filename[64];
    build_exam_filename(filename, sizeof(filename), idx);

    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("fopen exam");
        return -1;
    }
    if (fscanf(f, "%d", &exam->student_num) != 1) {
        fprintf(stderr, "Failed to read student number from %s\n", filename);
        fclose(f);
        return -1;
    }
    for (int i = 0; i < NUM_QUESTIONS; ++i) {
        exam->question_marked[i] = 0;
    }
    fclose(f);
    return 0;
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

    int shmid = shmget(IPC_PRIVATE, sizeof(SharedData), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    SharedData *shared = (SharedData *)shmat(shmid, NULL, 0);
    if (shared == (void *)-1) {
        perror("shmat");
        return 1;
    }

    if (load_rubric(&shared->rubric) != 0) {
        return 1;
    }
    shared->current_exam_index = 0;
    shared->done = 0;

    if (load_exam(&shared->exam, shared->current_exam_index) != 0) {
        return 1;
    }
    if (shared->exam.student_num == 9999) {
        shared->done = 1;
    }

    for (int i = 0; i < num_TAs; ++i) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            //Child TA process
            int ta_id = i + 1;
            srand(time(NULL) ^ getpid());

            while (!shared->done) {
                int student = shared->exam.student_num;

                for (int q = 0; q < NUM_QUESTIONS; ++q) {
                    int ms = 500 + rand() % 501;
                    usleep(ms * 1000);

                    
                    if (rand() % 2 == 0) {
                        char *line = shared->rubric.lines[q];
                        char *comma = strchr(line, ',');
                        if (comma && comma[1] != '\0' && comma[1] != '\n') {
                            comma[1] = comma[1] + 1;
                            save_rubric(&shared->rubric);
                            printf("TA %d [PID %d] updated rubric line %d\n",
                                   ta_id, getpid(), q + 1);
                        }
                    }
                }

                while (1) {
                    int chosen_q = -1;
                    for (int q = 0; q < NUM_QUESTIONS; ++q) {
                        if (shared->exam.question_marked[q] == 0) {
                            shared->exam.question_marked[q] = 1;
                            chosen_q = q;
                            break;
                        }
                    }

                    if (chosen_q == -1) {
                        break;
                    }

                    int ms2 = 1000 + rand() % 1001; 
                    usleep(ms2 * 1000);
                    printf("TA %d [PID %d] marked student %04d question %d\n",
                           ta_id, getpid(), student, chosen_q + 1);
                }

                if (shared->exam.student_num == 9999) {
                    shared->done = 1;
                    break;
                }

                shared->current_exam_index++;
                if (shared->current_exam_index >= MAX_EXAMS) {
                    shared->done = 1;
                    break;
                }
                if (load_exam(&shared->exam, shared->current_exam_index) != 0) {
                    shared->done = 1;
                    break;
                }
                if (shared->exam.student_num == 9999) {
                    shared->done = 1;
                }
            }

            printf("TA %d [PID %d] exiting (part 2a).\n", ta_id, getpid());
            _exit(0);
        }
    }

    while (wait(NULL) > 0)
        ;

    shmdt(shared);
    shmctl(shmid, IPC_RMID, NULL);

    printf("All TAs finished (no synchronization, race conditions possible!)\n");
    return 0;
}
