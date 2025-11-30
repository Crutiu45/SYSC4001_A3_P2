// common.h

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <errno.h>

// Configuration constants
#define NUM_EXAMS        20    // exam_0001.txt to exam_0020.txt
#define TERMINATION_EXAM 9999  // Special exam number to signal termination
#define NUM_EXERCISES    5     // Number of exercises in rubric
#define MAX_TAS          10    // Maximum number of TAs

// Semaphore indices for ta_marking_sync.c
#define SEM_EXAM_PILE    0     // Protects shared exam pile index
#define SEM_RUBRIC_WRITE 1     // Exclusive rubric writing
#define SEM_READER_COUNT 2     // Protects reader_count variable
#define NUM_SEMS         3     // Total number of semaphores

// Shared memory structure used by both programs
typedef struct {
    int  current_exam_index;                // Which exam to process next
    char rubric[NUM_EXERCISES][256];        // In-memory rubric (5 lines)
    int  rubric_version;                    // Tracks rubric changes
    int  reader_count;                      // Number of TAs currently reading rubric
} SharedData;

// Rubric management
void read_rubric_from_file(char rubric[NUM_EXERCISES][256]);
void write_rubric_to_file(char rubric[NUM_EXERCISES][256]);
void modify_rubric(SharedData *shared, int exercise_num);

// Semaphore operation wrappers
void sem_wait_op(int semid, int sem_num);
void sem_signal_op(int semid, int sem_num);

// Readers–writers synchronization functions
void start_reading(int semid, SharedData *shared);
void end_reading(int semid, SharedData *shared);
void start_writing(int semid);
void end_writing(int semid);

#endif // COMMON_H
