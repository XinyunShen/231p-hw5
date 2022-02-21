/**
 ** run example: ./hw5.bin 12 1 4 1
 **/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "util.h"

bool isSymmetric(Mat* origin, Mat* tran) {
    int n = origin->n;
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            if(origin->ptr[i * n + j] !=  tran->ptr[j * n + i]) {
                return false;
            }
        }
    }
    return true;
}

int main(int argc, char **argv){
    //help message
    if(argc != 5){
        printf("USAGE: %s n c t verb\n",argv[0]);
        printf("n       : Size of the square matrix\n");
        printf("c       : Coarseness used by the multi-thread transposer\n");
        printf("t       : Number of threads used by the multi-thread transposer.\n");
        printf("verb    : 0=prints only execution time.\n");
        printf("          1=also print matrices.\n");
        exit(1);
    }

    //get the command line arguments
    unsigned int n, c, t, verb;
    n = atoi(argv[1]);
    c = atoi(argv[2]);
    t = atoi(argv[3]);
    verb = atoi(argv[4]);
    if(n < 2 || 10000 < n){
        printf("The size of the matrix 'n' (%d) must be >= 2 and <= 10,000.\n", n);
        exit(1);
    }
    if(c < 1 || (n*n-n)/2 < c){
        printf("The coarseness level 'c' (%d) must be >= 1 and <= %d.\n", \
               c, (n*n-n)/2);
        exit(1);
    }
    if(t < 1){
        printf("The number of threads 't' (%d) must be >= 1.\n", t);
        exit(1);
    }

    //create and fill 3 matrices
    Mat mats[3];
    Mat origin[3];
    for(int i=0; i<3; i++){
        mat_init(&mats[i], n, n);
        mat_rand(&mats[i]);
        mat_init(&origin[i], n, n);
        mat_copy(&origin[i], &mats[i]);
    }


    //print original matrix
    if(verb){
        printf("Original0:\n");
        mat_print(&mats[0]);
        printf("Original1:\n");
        mat_print(&mats[1]);
        printf("Original2:\n");
        mat_print(&mats[2]);
    }

    //run and measure time taken
    double ta, tb, tc, td;
    ta = stopwatch();
    mat_sq_trans_st(&mats[0]);
    tb = stopwatch();
    mat_sq_trans_mt(&mats[1], 1, t);
    tc = stopwatch();
    mat_sq_trans_mt(&mats[2], c, t);
    td = stopwatch();

    // Check the correctness of results
    for(int i = 0; i < 3; i++) {
        if(!isSymmetric(&origin[i], &mats[i])) {
            printf("%d Fail!!!\n", i);
        }
        else {
            printf("%d Succeed!!!\n", i);
        }
    }

    //print resulting matrices
    if(verb){
        printf("\nTransposed, single thread:\n");
        mat_print(&mats[0]);
        printf("\nTransposed, multi thread fine-grain:\n");
        mat_print(&mats[1]);
        printf("\nTransposed, multi thread coarse-grain:\n");
        mat_print(&mats[2]);
    }

    //print execution time
    printf("%.8f, %.8f, %.8f\n", tb-ta, tc-tb, td-tc);
    
    return 0;
}
