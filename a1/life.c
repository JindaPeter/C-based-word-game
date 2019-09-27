#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_state(char *b, int size);
void update_state(char *b, int size);

int main(int argc, char **argv) {

    if (argc != 3) {
    	fprintf(stderr, "Usage: USAGE: life initial n\n");
    	return 1;
    }

    int size = strlen(argv[1]);
    
    int i;
    int times = strtol(argv[2], NULL, 10);
    char initial_state[size];
    for (i = 0; i < size; i++){
	initial_state[i] = argv[1][i];
    }
    
    for (i = 0; i < times; i++){
	print_state(initial_state,size);
	update_state(initial_state, size);
    }
    return 0;
    
}
