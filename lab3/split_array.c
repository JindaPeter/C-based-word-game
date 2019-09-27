#include <stdio.h>
#include <stdlib.h>

/* Return a pointer to an array of two dynamically allocated arrays of ints.
   The first array contains the elements of the input array s that are
   at even indices.  The second array contains the elements of the input
   array s that are at odd indices.

   Do not allocate any more memory than necessary.
*/
int **split_array(const int *s, int length) {
    int **split = malloc(2 * sizeof(int*));
    int even_length;
    if (length % 2 == 0){
	even_length = length / 2;
    } else {
	even_length = length / 2 + 1;
    }
    int odd_length = length / 2;
    split[0] = malloc(even_length * sizeof(int));
    split[1] = malloc(odd_length * sizeof(int));
    int i = 0;
    for (i = 0; i < length; i++){
	if (i % 2 == 0){
	    split[0][i / 2] = s[i];
        } else {
	    split[1][i / 2] = s[i];
	}
    }
    return split;
}

/* Return a pointer to an array of ints with size elements.
   - strs is an array of strings where each element is the string
     representation of an integer.
   - size is the size of the array
 */

int *build_array(char **strs, int size) {
    int *original = malloc(size * sizeof(int));
    int i = 0;
    for (i = 0; i < size; i++){
	original[i] = strtol(strs[i], NULL, 10);
    }
    return original; 

}


int main(int argc, char **argv) {
    /* Replace the comments in the next two lines with the appropriate
       arguments.  Do not add any additional lines of code to the main
       function or make other changes.
     */
    int *full_array = build_array(&argv[1], argc - 1);
    int **result = split_array(full_array, argc - 1);

    printf("Original array:\n");
    for (int i = 0; i < argc - 1; i++) {
        printf("%d ", full_array[i]);
    }
    printf("\n");

    printf("result[0]:\n");
    for (int i = 0; i < argc/2; i++) {
        printf("%d ", result[0][i]);
    }
    printf("\n");

    printf("result[1]:\n");
    for (int i = 0; i < (argc - 1)/2; i++) {
        printf("%d ", result[1][i]);
    }
    printf("\n");
    free(full_array);
    free(result[0]);
    free(result[1]);
    free(result);
    return 0;
}
