#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants that determine that address ranges of different memory regions

#define GLOBALS_START 0x400000
#define GLOBALS_END   0x700000
#define HEAP_START   0x4000000
#define HEAP_END     0x8000000
#define STACK_START 0xfff000000

int main(int argc, char **argv) {
    
    FILE *fp = NULL;

    if(argc == 1) {
        fp = stdin;

    } else if(argc == 2) {
        fp = fopen(argv[1], "r");
        if(fp == NULL){
            perror("fopen");
            exit(1);
        }
    } else {
        fprintf(stderr, "Usage: %s [tracefile]\n", argv[0]);
        exit(1);
    }

    /* Complete the implementation */


    /* Use these print statements to print the ouput. It is important that 
     * the output match precisely for testing purposes.
     * Fill in the relevant variables in each print statement.
     * The print statements are commented out so that the program compiles.  
     * Uncomment them as you get each piece working.
     */
    char t;
    unsigned long address;
    int instructions = 0;
    int modifications = 0;
    int loads = 0;
    int stores = 0;
    int globals = 0;
    int heap = 0;
    int stack = 0;
    while (fscanf(fp, "%c, %lx\n", &t, &address) != EOF){
	if (t == 'I') {
	    instructions += 1;
	} else if (t == 'M') {
	    modifications += 1;
	} else if (t == 'L') {
	    loads += 1;
	} else if (t == 'S') {
	    stores += 1;
	}

	if (t != 'I') { 
	    if ((address >= GLOBALS_START) && (address <= GLOBALS_END)) {
	        globals += 1;
	    } else if ((address >= HEAP_START) && (address <= HEAP_END)) {
		heap += 1;
	    } else if (address >= STACK_START){
		stack += 1;
	    }
	}
    }

    printf("Reference Counts by Type:\n");
    printf("    Instructions: %d\n", instructions);
    printf("    Modifications: %d\n", modifications);
    printf("    Loads: %d\n", loads);
    printf("    Stores: %d\n", stores);
    printf("Data Reference Counts by Location:\n");
    printf("    Globals: %d\n", globals);
    printf("    Heap: %d\n", heap);
    printf("    Stack: %d\n", stack);
    fclose(fp);

    return 0;
}
