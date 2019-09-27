#include <stdio.h>

void print_state(char *b, int size){
    int i;
    for (i = 0; i < size; i++){
	printf("%c", b[i]);
    }
    printf("\n");
}

void update_state(char *b, int size){
    int i;
    char c[size];
    for (i = 0; i < size; i++){
    	if (i == 0 || i == (size - 1)){
            c[i] = b[i];
	}else{
	    if ((b[i - 1] == '.' && b[i + 1] == '.') || (b[i - 1] == 'X' && b[i + 1] == 'X')){
	        c[i] = '.';
	    } else {
	        c[i] = 'X';
	    }
	}
    }
    for (i = 0; i < size; i++){
        b[i] = c[i];
    }
}

