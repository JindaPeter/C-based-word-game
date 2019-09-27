#include <stdio.h>
#include <sys/stat.h>
#include <getopt.h>
#include <unistd.h>
#include <stdlib.h> 
#include "helper.h"
#include <sys/wait.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    struct timeval starttime, endtime;
    double timediff;
    
    /* get the time when the main function starts*/
    if ((gettimeofday(&starttime, NULL)) == -1) {
        perror("gettimeofday");
        exit(1);
    }
    if (argc != 7){
        error_message(); //print the error message and exit
    }
    int opt;
    int number_of_processes;
    int error;
    char *input_f;
    char *output_f;
    /* get arguments from the command line */
    while ((opt = getopt(argc, argv, "n:f:o:")) != -1){
        switch(opt)
        {
            case 'n':
                number_of_processes = strtol(optarg, NULL, 10); // the number of child processes
                if (number_of_processes == 0){  // if number_of_processes is 0, then set it to 1(1 child)
                    number_of_processes = 1;
                }
                break;
            case 'f':
                input_f = optarg; //input file name
                break;
            case 'o':
                output_f = optarg; //output file name
                break;
            case '?':
                error_message(); //if the opt is not in 'nfo', then print the error message and exit
        }
    }
    
    /* get the numer of words in the input binary file*/
    int num_of_words = (int)get_file_size(input_f)/sizeof(struct rec);
    
    /* if the number of child processes exceed the number of words, then exit*/
    if (number_of_processes > num_of_words){
        number_of_processes = num_of_words; //If the number of child processes is larger than the number of words, 												then set it to the number of words
    }
    /* int array to store the number of words each child process should sort*/
    int child_processes[number_of_processes];
    int i;

    /*The algorithm is at first evenly divide words into child processes, then distribute the remaing n words
    to the first n child processes e.g. 8 words for 3 children: firstly distribute 2 words for each child,
    then distribute the remaining 2 words to the first 2 child, 1 word each. */
    for (i = 0; i < number_of_processes; i++){
        child_processes[i] = num_of_words/number_of_processes;
    }
    int remaining = num_of_words % number_of_processes;
    if (remaining != 0){
        for (i = 0; i < remaining; i++){
            child_processes[i]++;
        }
    }

    int result = 1;
    int location;
    int j, k;
    int status;
    int pipe_fd[number_of_processes][2]; // we will need one pipe for each child process
    for (i = 0; i < number_of_processes; i++){
        if (result > 0){
            // call pipe before we fork
            if ((pipe(pipe_fd[i])) == -1) {
                perror("pipe");
                exit(1);
            }
            // call fork
            result = fork();
            if (result == -1) { //case: a system call error
                //handle the error
                perror("fork");
                exit(1);
            } else if (result == 0){  //case: a child process
                // child does their work here
                // child only writes to the pipe so close reading end
                if (close(pipe_fd[i][0]) == -1 ) {
                    perror("close reading end from inside child");
                    exit(1);
                }
                // before we forked the parent had opened the reading ends to
                // all previously forked children -- so close those
                int child_no;
                for (child_no = 0; child_no < i; child_no++) {
                    if (close(pipe_fd[child_no][0]) == -1) {
                        perror("close reading ends of previously forked children");
                        exit(1);
                    }

                }
                
                location = 0; // the location of the input file where this child process should start reading
                for (j = 0; j < i; j++){
                    location += child_processes[j];
                }
                /* helper function sort() returns a sorted struct rec array (using qsort) */
                struct rec *child_rec = sort(input_f, location, child_processes[i]);
                // write one record at a time (in sorted order) to the pipe connecting the child to the parent
                for (k = 0; k < child_processes[i]; k++){
                    if (write(pipe_fd[i][1], &child_rec[k], sizeof(struct rec)) != sizeof(struct rec)) {
                        perror("write from child to pipe");
                        exit(1);
                    }
                }
                //free hte dynamic space
                free(child_rec);
                // I'm done with the pipe so close it
                if (close(pipe_fd[i][1]) == -1) {
                    perror("close pipe after writing");
                    exit(1);
                }
                // exit so I don't fork my own children on next loop iteration
                exit(0);
            } else {
                // close the end of the pipe that I don't want open
                if (close(pipe_fd[i][1]) == -1) {
                    perror("close writing end of pipe in parent");
                    exit(1);
                }
            }
        }
    }
    // Only the parent gets here
    /*build a struct rec array, reading 1 word from each child process*/
    struct rec current_rec[number_of_processes];
    for (i = 0; i < number_of_processes; i++){
        if (read(pipe_fd[i][0], &current_rec[i], sizeof(struct rec)) == -1) {
            perror("reading from pipe from a child");
            exit(1);
        }
    }

    int out_index; //the index of the smallest freq
    FILE *out_fp;
    out_fp = fopen(output_f, "wb"); // open the output file.

    /* use two helper functions:
    check_end: check if all the freq of struct rec in the array is -1, if it is, then end the while loop.
    merge: write the rec with the smallest freq to the output file, one rec each call.
    */
    while (check_end(current_rec, number_of_processes) != 1){
        out_index = merge(current_rec, number_of_processes, out_fp);
        if (read(pipe_fd[out_index][0], &current_rec[out_index], sizeof(struct rec)) == 0) {
            current_rec[out_index].freq = -1;
        }
    }
    /* parent waits until all the children terminate */
    for (i = 0; i < number_of_processes; i++) {
        if (wait(&status) == -1) {
            perror("wait");
            exit(1);
        }
        if (WIFEXITED(status) == 0) {
            fprintf(stderr, "Child terminated abnormally\n");
        }
    }
         
    error = fclose(out_fp); // close the output file.
    if (error != 0) {
        fprintf(stderr, "fclose failed\n");
        return 1;
    }
    /* print the time it takes the main function to run */
    if ((gettimeofday(&endtime, NULL)) == -1) {
            perror("gettimeofday");
            exit(1);
        }

        timediff = (endtime.tv_sec - starttime.tv_sec) +
            (endtime.tv_usec - starttime.tv_usec) / 1000000.0;
        fprintf(stdout, "%.4f\n", timediff);
    
    return 0;
}

