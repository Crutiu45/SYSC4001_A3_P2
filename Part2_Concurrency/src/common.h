
#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>

#define MAX_EXAMS 21       // exam_0001.txt ... exam_0020.txt, exam_9999.txt
#define MAX_TAS 10         // up to 10 TAs


#define NUM_QUESTIONS 5
#define RUBRIC_LINE_LEN 128

typedef struct {
    char lines[NUM_QUESTIONS][RUBRIC_LINE_LEN];
} Rubric;


typedef struct {
    int student_num;                    
    int question_marked[NUM_QUESTIONS]; 
} ExamState;


typedef struct {
    Rubric rubric;          
    ExamState exam;         
    int current_exam_index; 
    int done;               
} SharedData;


union semun {
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
};

#endif
