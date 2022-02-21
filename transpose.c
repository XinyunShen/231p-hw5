#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "util.h"

typedef struct {
    pthread_mutex_t* mutex;
    Mat* mat;
    int* row_pos;
    int* col_pos;
    int grain;
} Args ;


void mat_sq_trans_st(Mat *mat){
    int n = mat->n;
    double tmp;
    for(int i = 0; i < n; i++) {
        for(int j = i + 1; j < n; j++) {
            tmp = mat->ptr[i * n + j];
            mat->ptr[i * n + j] = mat->ptr[j * n + i];
            mat->ptr[j * n + i] = tmp;
        }
    }
}

void* thread_task(void* args) {
    Args* info = (Args*) args;
    int* row_ptr = info->row_pos;
    int* col_ptr = info->col_pos;
    int n = info->mat->n;
    while(*row_ptr < n) {
        int row_pos, col_pos;
        {
            pthread_mutex_lock(info->mutex);
            row_pos = *row_ptr;
            col_pos = *col_ptr;
            // Update row and column
            int step = info->grain;
            while (step && *row_ptr < n) {
                if (n - *col_ptr >= step) {
                    *col_ptr += step;
                    step = 0;
                } else {
                    step -= n - *col_ptr;
                    *row_ptr += 1;
                    *col_ptr = *row_ptr + 1;
                }
            }
            pthread_mutex_unlock(info->mutex);
        }

        double tmp;
        int step = info->grain;
        while(step && row_pos < n) {
            if(n - col_pos >= step) {
                for(int j = col_pos; j < col_pos + step; j++) {
                    tmp = info->mat->ptr[row_pos * n + j];
                    info->mat->ptr[row_pos * n + j] = info->mat->ptr[j * n + row_pos];
                    info->mat->ptr[j * n + row_pos] = tmp;
                }
                step = 0;
            }
            else {
                step -= n - col_pos;
                for(int j = col_pos; j < n; j++) {
                    tmp = info->mat->ptr[row_pos * n + j];
                    info->mat->ptr[row_pos * n + j] = info->mat->ptr[j * n + row_pos];
                    info->mat->ptr[j * n + row_pos] = tmp;
                }
                row_pos++;
                col_pos = row_pos + 1;
            }
        }
    }
    return NULL;
}

void mat_sq_trans_mt(Mat *mat, unsigned int grain, unsigned int thread_num){
    if(thread_num == 1) {
        mat_sq_trans_st(mat);
        return;
    }

    pthread_t threads[thread_num - 1];
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, NULL);
    int row_pos = 0, col_pos = 1;
    Args args = {&mutex, mat, &row_pos, &col_pos, grain};

    for(int i = 0; i < thread_num - 1; i++) {
        if(pthread_create(&threads[i], NULL, thread_task, (void*)&args)) {
            printf("Fail to create threads\n");
            exit(1);
        }
    }

    thread_task(&args);

    for(int i = 0; i < thread_num - 1; i++) {
        pthread_join(threads[i], NULL);
    }
    pthread_mutex_destroy(&mutex);
}
