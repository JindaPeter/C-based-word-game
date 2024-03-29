#include <netinet/in.h>

#define MAX_NAME 30  
#define MAX_MSG 128
#define MAX_WORD 20
#define MAX_BUF 256
#define MAX_GUESSES 26
#define NUM_LETTERS 26
#define WELCOME_MSG "Welcome to our word game. What is your name? "
#define INVALID_GUESS "Invalid guess, please input a lowercase letter:"
#define INVALID_NAME "Invalid name, try another one:"
#define EXISTED_NAME "This name is already existed, try another one:"
#define YOUR_GUESS "Your guess?\n"

struct client {
    int fd;
    struct in_addr ipaddr;
    struct client *next;
    char name[MAX_NAME];
    char inbuf[MAX_BUF];  // Used to hold input from the client
    char *in_ptr;         // A pointer into inbuf to help with partial reads
};

// Information about the dictionary used to pick random word
struct dictionary {
    FILE *fp;
    int size;
};

struct game_state {
    char word[MAX_WORD];      // The word to guess
    char guess[MAX_WORD];     // The current guess (for example '-o-d')
    int letters_guessed[NUM_LETTERS]; // Index i will be 1 if the corresponding
                                      // letter has been guessed; 0 otherwise
    int guesses_left;         // Number of guesses remaining
    struct dictionary dict;
    
    struct client *head;
    struct client *has_next_turn;
};


void init_game(struct game_state *game, char *dict_name);
int get_file_length(char *filename);
char *status_message(char *msg, struct game_state *game);
