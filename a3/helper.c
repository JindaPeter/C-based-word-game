#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "helper.h"


int get_file_size(char *filename) {
    struct stat sbuf;

    if ((stat(filename, &sbuf)) == -1) {
       perror("stat");
       exit(1);
    }

    return sbuf.st_size;
}

/* A comparison function to use for qsort */
int compare_freq(const void *rec1, const void *rec2) {

    struct rec *r1 = (struct rec *) rec1;
    struct rec *r2 = (struct rec *) rec2;

    if (r1->freq == r2->freq) {
        return 0;
    } else if (r1->freq > r2->freq) {
        return 1;
    } else {
        return -1;
    }
}

void error_message(){

    /* print the error message if the incorrect number of 
    command-line arguments or incorrect options are provided */
    fprintf(stderr, "Usage: psort -n <number of processes> -f <inputfile> -o <outputfile>\n");
    exit(1);
}

/* The task for each child process: open the file, use fseek to get to the correct location
in the file, and begin reading at that point. Then use qsort to sort the data, then close the file.*/
struct rec* sort(char *f, int loc, int num_of_process){
    FILE *fp;
    int error;
    fp = fopen(f, "rb");
    if (fp == NULL){
        perror("fopen");
        exit(0);
    }
    struct rec *child = malloc(num_of_process * sizeof(struct rec));
    fseek(fp, loc * sizeof(struct rec), SEEK_SET);
    fread(child, sizeof(struct rec), num_of_process, fp);
    qsort(child, num_of_process, sizeof(struct rec), compare_freq);
    error = fclose(fp);
    if (error != 0){
        perror("close");
        exit(1);
    }
    return child;
}

/* write the rec with the smallest freq to the output file, one rec each call.*/
int merge(struct rec *r, int len, FILE* f){
    int least_index = 0;
    int i = 0;
    int error;
    while (least_index == 0 && i < len){
        if (r[i].freq != -1){
            least_index = i;
        }
        i++;
    }
    for (i = 0; i < len; i++){
        if (r[i].freq < r[least_index].freq && r[i].freq != -1){
            least_index = i;
        }
    }
    error = fwrite(&(r[least_index]), sizeof(struct rec), 1, f);
    if (error != 1) {
        perror("fwrite");
        exit(1);
    }
    return least_index;
}

/*check if all the freq of struct rec in the array is -1, if it is, then end the while loop.*/
int check_end(struct rec *r, int len){
    for (int i = 0; i < len; i++){
        if (r[i].freq != -1) { // if one freq of the rec array is -1, then return 0
            return 0;
        }
    }
    return 1; // else return 1
}


