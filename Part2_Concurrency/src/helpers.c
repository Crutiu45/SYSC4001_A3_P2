// helpers.c
#include "common.h"


// Read rubric.txt into rubric array
void read_rubric_from_file(char rubric[NUM_EXERCISES][256]) {
    FILE *f = fopen("../rubric.txt", "r");
    if (!f) {
        perror("Failed to open rubric.txt");
        exit(1);
    }
    for (int i = 0; i < NUM_EXERCISES; i++) {
        if (fgets(rubric[i], 256, f) == NULL) {
            fprintf(stderr, "Error reading rubric line %d\n", i + 1);
            exit(1);
        }
        rubric[i][strcspn(rubric[i], "\n")] = 0; // strip newline
    }
    fclose(f);
}

// Write rubric array back to rubric.txt
void write_rubric_to_file(char rubric[NUM_EXERCISES][256]) {
    FILE *f = fopen("../rubric.txt", "w");
    if (!f) {
        perror("Failed to write rubric.txt");
        return;
    }
    for (int i = 0; i < NUM_EXERCISES; i++) {
        fprintf(f, "%s\n", rubric[i]);
    }
    fclose(f);
}

// Modify one rubric line in shared memory and file
void modify_rubric(SharedData *shared, int exercise_num) {
    char *comma = strchr(shared->rubric[exercise_num], ',');
    if (comma && *(comma + 1)) {
        char *letter = comma + 1;
        if (*letter >= 'A' && *letter < 'Z') {
            (*letter)++;
        } else if (*letter == 'Z') {
            *letter = 'A';
        }
        shared->rubric_version++;
        write_rubric_to_file(shared->rubric);
    }
}

// Semaphore wait (P) operation
void sem_wait_op(int semid, int sem_num) {
    struct sembuf sb = { sem_num, -1, 0 };
    if (semop(semid, &sb, 1) == -1) {
        perror("sem_wait");
        exit(1);
    }
}

// Semaphore signal (V) operation
void sem_signal_op(int semid, int sem_num) {
    struct sembuf sb = { sem_num, 1, 0 };
    if (semop(semid, &sb, 1) == -1) {
        perror("sem_signal");
        exit(1);
    }
}

// Readers–writers: start reading rubric
void start_reading(int semid, SharedData *shared) {
    sem_wait_op(semid, SEM_READER_COUNT);
    shared->reader_count++;
    if (shared->reader_count == 1) {
        // first reader locks writers out
        sem_wait_op(semid, SEM_RUBRIC_WRITE);
    }
    sem_signal_op(semid, SEM_READER_COUNT);
}

// Readers–writers: end reading rubric
void end_reading(int semid, SharedData *shared) {
    sem_wait_op(semid, SEM_READER_COUNT);
    shared->reader_count--;
    if (shared->reader_count == 0) {
        // last reader lets writers in
        sem_signal_op(semid, SEM_RUBRIC_WRITE);
    }
    sem_signal_op(semid, SEM_READER_COUNT);
}

// Readers–writers: start writing rubric
void start_writing(int semid) {
    sem_wait_op(semid, SEM_RUBRIC_WRITE);
}

// Readers–writers: end writing rubric
void end_writing(int semid) {
    sem_signal_op(semid, SEM_RUBRIC_WRITE);
}
