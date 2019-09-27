#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "family.h"

/* Number of word pointers allocated for a new family.
   This is also the number of word pointers added to a family
   using realloc, when the family is full.
*/
static int family_increment = 0;


/* Set family_increment to size, and initialize random number generator.
   The random number generator is used to select a random word from a family.
   This function should be called exactly once, on startup.
*/
void init_family(int size) {
    family_increment = size;
    srand(time(0));
}


/* Given a pointer to the head of a linked list of Family nodes,
   print each family's signature and words.

   Do not modify this function. It will be used for marking.
*/
void print_families(Family* fam_list) {
    int i;
    Family *fam = fam_list;
    
    while (fam) {
        printf("***Family signature: %s Num words: %d\n",
               fam->signature, fam->num_words);
        for(i = 0; i < fam->num_words; i++) {
            printf("     %s\n", fam->word_ptrs[i]);
        }
        printf("\n");
        fam = fam->next;
    }
}


/* Return a pointer to a new family whose signature is 
   a copy of str. Initialize word_ptrs to point to 
   family_increment+1 pointers, numwords to 0, 
   maxwords to family_increment, and next to NULL.
*/
Family *new_family(char *str) {
    Family *new_family = malloc(sizeof(struct fam));
    if (new_family == NULL){
	perror("malloc");
	exit(1);
    }
    new_family->signature = malloc((strlen(str) + 1) * sizeof(char));
    if (new_family->signature == NULL){
	perror("malloc");
	exit(1);
    }
    strcpy(new_family->signature, str);
    new_family->word_ptrs = malloc((family_increment + 1) * sizeof(char *));
    if (new_family->word_ptrs == NULL){
	perror("malloc");
	exit(1);
    }
    int i;
    for (i = 0; i < family_increment + 1; i++){
	new_family->word_ptrs[i] = NULL;
    }
    new_family->num_words = 0;
    new_family->max_words = family_increment;
    new_family->next = NULL;
    return new_family;
}


/* Add word to the next free slot fam->word_ptrs.
   If fam->word_ptrs is full, first use realloc to allocate family_increment
   more pointers and then add the new pointer.
*/
void add_word_to_family(Family *fam, char *word) {
    if (fam->num_words == fam->max_words){
	fam->word_ptrs = realloc(fam->word_ptrs, (fam->max_words + family_increment + 1) * sizeof(char *));
        if (fam->word_ptrs == NULL){
	    perror("realloc");
	    exit(1);
        }
	int i;
	for (i = fam->max_words; i < fam->max_words + family_increment + 1; i++){
	    fam->word_ptrs[i] = NULL;
	}
	fam->max_words += family_increment;
    }
    fam->word_ptrs[fam->num_words] = word;
    fam->num_words += 1;
    return;
}


/* Return a pointer to the family whose signature is sig;
   if there is no such family, return NULL.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_family(Family *fam_list, char *sig) {
    Family *current = fam_list;
    while (current != NULL){
	if (strcmp(current->signature, sig) == 0){
	    return current;
	}
	current = current->next;
    }
    return NULL;
}


/* Return a pointer to the family in the list with the most words;
   if the list is empty, return NULL. If multiple families have the most words,
   return a pointer to any of them.
   fam_list is a pointer to the head of a list of Family nodes.
*/
Family *find_biggest_family(Family *fam_list) {
    if (fam_list == NULL){
    	return NULL;
    }
    Family *current = fam_list;
    Family *most_words_family = current;
    while (current != NULL){
	if (current->num_words > most_words_family->num_words){
	    most_words_family = current;
	}
	current = current->next;
    }
    return most_words_family;
}


/* Deallocate all memory rooted in the List pointed to by fam_list. */
void deallocate_families(Family *fam_list) {
    Family *current = fam_list;
    Family *pre_current;
    while (current != NULL){
	free(current->signature);
	free(current->word_ptrs);
	pre_current = current;
	current = current->next;
	free(pre_current);
    }
    return;
}


/* Generate and return a linked list of all families using words pointed to
   by word_list, using letter to partition the words.

   Implementation tips: To decide the family in which each word belongs, you
   will need to generate the signature of each word. Create only the families
   that have at least one word from the current word_list.
*/
Family *generate_families(char **word_list, char letter) {
    int length = strlen(word_list[0]) + 1;
    char sig[length];
    int index;
    for (index = 0; index < length; index++){
	sig[index] = '\0';
    }
    Family *fam = NULL;
    Family *current = NULL;
    int i = 0;
    int j;
    while (word_list[i] != NULL){
	for (j = 0; j < strlen(word_list[i]); j++){
	    if (word_list[i][j] != letter){
		sig[j] = '-';
	    } else {
		sig[j] = letter;
	    }
	}
	if (!fam){
	    fam = new_family(sig);
	    current = fam;
	}
	if (find_family(fam, sig) == NULL){
	    current->next = new_family(sig);
	    current = current->next;
	} else {
	    add_word_to_family(find_family(fam, sig), word_list[i]);
	}
	i++;
    }
    return fam;
}


/* Return the signature of the family pointed to by fam. */
char *get_family_signature(Family *fam) {
    return fam->signature;
}


/* Return a pointer to word pointers, each of which
   points to a word in fam. These pointers should not be the same
   as those used by fam->word_ptrs (i.e. they should be independently malloc'd),
   because fam->word_ptrs can move during a realloc.
   As with fam->word_ptrs, the final pointer should be NULL.
*/
char **get_new_word_list(Family *fam) {
    char **new_word_ptrs;
    new_word_ptrs = malloc((fam->num_words + 1) * sizeof(char *));
    if (new_word_ptrs == NULL){
	perror("malloc");
	exit(1);
    }
    new_word_ptrs[fam->num_words] = NULL;
    int i;
    for (i = 0; i < fam->num_words; i++) {
	new_word_ptrs[i] = fam->word_ptrs[i];
    }
    return new_word_ptrs;
}


/* Return a pointer to a random word from fam. 
   Use rand (man 3 rand) to generate random integers.
*/
char *get_random_word_from_family(Family *fam) {
    int i = rand() % fam->num_words;
    return fam->word_ptrs[i];
}
