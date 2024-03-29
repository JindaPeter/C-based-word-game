#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/* Write random integers (in binary) to a file with the name given by the command-line
 * argument.  This program creates a data file for use by the time_reads program.
 */

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: write_test_file filename\n");
        exit(1);
    }

    FILE *fp;
    if ((fp = fopen(argv[1], "w")) == NULL) {
        perror("fopen");
        exit(1);
    }
    
    int error;
    int r;
    for (int i = 0; i < 100; i++){
        r = random() % 100;
        error = fwrite(&r, sizeof(int), 1, fp);
        if (error != 1) {
            perror("fwrite");
        return 1;
    }
    }
    fclose(fp);
    return 0;
}
