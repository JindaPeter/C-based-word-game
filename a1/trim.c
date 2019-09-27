#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Reads a trace file produced by valgrind and an address marker file produced
 * by the program being traced. Outputs only the memory reference lines in
 * between the two markers
 */

int main(int argc, char **argv) {
    
    if(argc != 3) {
         fprintf(stderr, "Usage: %s tracefile markerfile\n", argv[0]);
         exit(1);
    }

    // Addresses should be stored in unsigned long variables
    
    unsigned long start_marker, end_marker;
    FILE *f2;
    f2 = fopen(argv[2], "r");
    fscanf(f2, "%lx %lx", &start_marker, &end_marker);
    fclose(f2);
	
    FILE* f1 = fopen(argv[1], "r");
    char t;
    unsigned long address;
    int n;
    while (fscanf(f1, "%c %lx, %d\n", &t, &address, &n) != EOF && (address != start_marker)){continue; }
    while(fscanf(f1, "%c %lx, %d\n", &t, &address, &n) != EOF && (address != end_marker)){
        printf("%c,%#lx\n", t, address);
    }
    fclose(f1);
    


    /* For printing output, use this exact formatting string where the
     * first conversion is for the type of memory reference, and the second
     * is the address
     */
    // printf("%c,%#lx\n", VARIABLES TO PRINT GO HERE);

    return 0;
}
