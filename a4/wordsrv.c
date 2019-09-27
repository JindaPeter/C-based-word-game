#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "socket.h"
#include "gameplay.h"


#ifndef PORT
    #define PORT 56212
#endif
#define MAX_QUEUE 5


void add_player(struct client **top, int fd, struct in_addr addr);
void remove_player(struct client **top, int fd);

/* These are some of the function prototypes that we used in our solution
 * You are not required to write functions that match these prototypes, but
 * you may find the helpful when thinking about operations in your program.
 */
/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf);
void announce_turn(struct game_state *game);
void announce_winner(struct game_state *game, struct client *winner);
/* Move the has_next_turn pointer to the next active client */
void advance_turn(struct game_state *game);
int partial_read(char *name, struct client **p, int s);
void broadcast_turn_info(struct game_state *game);
void handle_disconnect_player(struct game_state *game, struct client **p, int fd);


/* The set of socket descriptors for select to monitor.
 * This is a global variable because we need to remove socket descriptors
 * from allset when a write to a socket fails.
 */
fd_set allset;

/* Send the message in outbuf to all clients */
void broadcast(struct game_state *game, char *outbuf){
    struct client *p;
    for(p = game->head; p != NULL; p = p->next) {
        write(p->fd, outbuf, strlen(outbuf));
    }
}

/* Send the message to tell what does this turn belong to*/
void announce_turn(struct game_state *game){
    char *guess_info = YOUR_GUESS;
    char turn_info[MAX_BUF];
    sprintf(turn_info, "It's %s's turn.\r\n", (game->has_next_turn)->name);
    printf("It's %s's turn.\n", (game->has_next_turn)->name);
    struct client *p;
    for(p = game->head; p != NULL; p = p->next) {
        if (p->fd == (game->has_next_turn)->fd) {
            write(p->fd, guess_info, strlen(guess_info));
        } else {
            write(p->fd, turn_info, strlen(turn_info));
        }
    }
}

/* Annouce the winner of the game*/
void announce_winner(struct game_state *game, struct client *winner) {
    char over_info[MAX_BUF];
    sprintf(over_info, "The word was %s\r\n", game->word);
    char *winner_msg = "Game over! You win!\r\n\r\nLet's start a new game.\r\n";
    char other_msg[MAX_BUF];
    sprintf(other_msg, "Game over! %s won!\r\n\r\nLet's start a new game.\r\n", winner->name);
    struct client *p;
    for(p = game->head; p != NULL; p = p->next) {
        if (p->fd == (game->has_next_turn)->fd) {
            write(p->fd, over_info, strlen(over_info));
            write(p->fd, winner_msg, strlen(winner_msg));
        } else {
            write(p->fd, over_info, strlen(over_info));
            write(p->fd, other_msg, strlen(other_msg));
        }
    }
}

/* Move the turn to the next player.*/
void advance_turn(struct game_state *game){
    if ((game->has_next_turn)->next == NULL) {
        game->has_next_turn = game->head;
    } else {
        game->has_next_turn = (game->has_next_turn)->next;
    }
}

/* Add a client to the head of the linked list
 */
void add_player(struct client **top, int fd, struct in_addr addr) {
    struct client *p = malloc(sizeof(struct client));

    if (!p) {
        perror("malloc");
        exit(1);
    }

    printf("Adding client %s\n", inet_ntoa(addr));

    p->fd = fd;
    p->ipaddr = addr;
    p->name[0] = '\0';
    p->in_ptr = p->inbuf;
    p->inbuf[0] = '\0';
    p->next = *top;
    *top = p;
}

/* Removes client from the linked list and closes its socket.
 * Also removes socket descriptor from allset
 */
void remove_player(struct client **top, int fd) {
    struct client **p;

    for (p = top; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    // Now, p points to (1) top, or (2) a pointer to another client
    // This avoids a special case for removing the head of the list
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %d %s\n", fd, inet_ntoa((*p)->ipaddr));
        FD_CLR((*p)->fd, &allset);
        close((*p)->fd);
        free(*p);
        *p = t;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n",
                 fd);
    }
}

/* Partial read the input given by the clients, store it in the
inbuf of the client, return 1 when "\r\n" is found and return 0
when "\r\n" is not found. */
int partial_read(char *name, struct client **p, int s) {
    if (strlen(name) == 0) {
        return 0;
    }
    if (strcmp(name, "\r\n") == 0){
        printf("[%d] Read %d bytes\n", (*p)->fd, s);
        printf("[%d] Found newline %s\n", (*p)->fd, (*p)->inbuf);
        return 1;
    } else if (strstr(name, "\r\n") == NULL) {
        int l = strlen((*p)->inbuf);
        strcat((*p)->inbuf, name);
        ((*p)->inbuf)[s + l] = '\0';
        printf("[%d] Read %d bytes\n", (*p)->fd, s);
        return 0;
    } else {
        int l = strlen((*p)->inbuf);
        strcat((*p)->inbuf, name);
        ((*p)->inbuf)[s + l - 2] = '\0';
        printf("[%d] Read %d bytes\n", (*p)->fd, s);
        printf("[%d] Found newline %s\n", (*p)->fd, (*p)->inbuf);
        return 1;
    }
}

/* Broadcast the information of the turn to all the players.*/
void broadcast_turn_info(struct game_state *game) {
    char *turn_info = malloc(500 * sizeof(char));
    if (!turn_info) {
        perror("malloc");
        exit(1);
    }
    turn_info = status_message(turn_info, game);
    broadcast(game, turn_info);
    free(turn_info);
}

/* Deal with the disconnected player.*/
void handle_disconnect_player(struct game_state *game, struct client **p, int fd) {
    char bye_info[MAX_BUF];
    sprintf(bye_info, "Goodbye %s\r\n", (*p)->name);
    broadcast(game, bye_info);

    //check if the disconnected player is in the head of the game.
    if ((*p)->fd == game->head->fd){
        game->head = game->head -> next;
    }else{
        //remove this player from the linked list
        struct client *cur = game->head;
        for (struct client *m = game->head -> next; m != NULL; m = m-> next){
            if ((*p)->fd != m-> fd){
                cur -> next = m;
                cur = cur->next;
            }
        }
        cur->next = NULL;
    }
    //handle the player in the next turn
    if (fd == game->has_next_turn -> fd) {
        if (game->has_next_turn -> next == NULL) {
            game->has_next_turn = game->head;
        }else{
            game->has_next_turn = game->has_next_turn->next;
        }
    }
    //if still exists players in the game, announce turn info.
    if (game->has_next_turn != NULL) {
        announce_turn(game);
    }
}

int main(int argc, char **argv) {
    struct sigaction sa;
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    if(sigaction(SIGPIPE, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    int clientfd, maxfd, nready;
    struct client *p;
    fd_set rset;

    if(argc != 2){
        fprintf(stderr,"Usage: %s <dictionary filename>\n", argv[0]);
        exit(1);
    }

    // Create and initialize the game state
    struct game_state game;

    srandom((unsigned int)time(NULL));
    // Set up the file pointer outside of init_game because we want to 
    // just rewind the file when we need to pick a new word
    game.dict.fp = NULL;
    game.dict.size = get_file_length(argv[1]);

    init_game(&game, argv[1]);

    // head and has_next_turn also don't change when a subsequent game is
    // started so we initialize them here.
    game.head = NULL;
    game.has_next_turn = NULL;

    /* A list of client who have not yet entered their name.  This list is
     * kept separate from the list of active players in the game, because
     * until the new playrs have entered a name, they should not have a turn
     * or receive broadcast messages.  In other words, they can't play until
     * they have a name.
     */
    struct client *new_players = NULL;

    struct sockaddr_in *server = init_server_addr(PORT);
    int listenfd = set_up_server_socket(server, MAX_QUEUE);

    // initialize allset and add listenfd to the
    // set of file descriptors passed into select
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);
    // maxfd identifies how far into the set to search
    maxfd = listenfd;

    while (1) {
        // make a copy of the set before we pass it into select
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            perror("select");
            continue;
        }

        if (FD_ISSET(listenfd, &rset)){
            printf("A new client is connecting\n");
            clientfd = accept_connection(listenfd);

            FD_SET(clientfd, &allset);
            if (clientfd > maxfd) {
                maxfd = clientfd;
            }
            printf("Connection from %s\n", inet_ntoa(server->sin_addr));
            add_player(&new_players, clientfd, server->sin_addr);
            char *greeting = WELCOME_MSG;
            if(write(clientfd, greeting, strlen(greeting)) == -1) {
                fprintf(stderr, "Write to client %s failed\n", inet_ntoa(server->sin_addr));
                remove_player(&(new_players), clientfd);
            };
        }

        /* Check which other socket descriptors have something ready to read.
         * The reason we iterate over the rset descriptors at the top level and
         * search through the two lists of clients each time is that it is
         * possible that a client will be removed in the middle of one of the
         * operations. This is also why we call break after handling the input.
         * If a client has been removed the loop variables may not longer be 
         * valid.
         */
        int cur_fd;
        for(cur_fd = 0; cur_fd <= maxfd; cur_fd++) {
            if(FD_ISSET(cur_fd, &rset)) {
                // Check if this socket descriptor is an active player
                for(p = game.head; p != NULL; p = p->next) {
                    if(cur_fd == p->fd) {
                        //TODO - handle input from an active client

                        //Read the input from the player
                        char guess_input[MAX_WORD];
                        int num_guesses = read(cur_fd, guess_input, MAX_WORD);
                        if (num_guesses == -1) {
                            perror("read");
                            exit(1);
                        }

                        //Put the null terminator
                        guess_input[num_guesses] = '\0';

                        //check if the player disconnects
                        if (num_guesses == 0) {
                            //handle the disconnected player
                            handle_disconnect_player(&game, &p, cur_fd);
                            printf("Disconnect from %s\n", inet_ntoa(server->sin_addr));
                            printf("Removing client %d %s\n", cur_fd, inet_ntoa(server->sin_addr));
                            free(p);
                            break;
                        }
                        //check if there is network newline in the client inbuf
                        if (partial_read(guess_input, &p, num_guesses) == 1){

                            int len_input = strlen(p->inbuf);
                            //copy the first element in inbuf to a char
                            char guess = p->inbuf[0];
                            //initialize the inbuf for later use
                            for (int i = 0; i < strlen(p->inbuf); i++){
                                p->inbuf[i] = '\0';
                            }

                            //check if it is this player's turn
                            if (game.has_next_turn->fd != p->fd){
                                char *not_your_turn = "It's not your turn.\r\n";
                                write(cur_fd, not_your_turn, strlen(not_your_turn));
                                printf("Player %s tried to guess out of turn.\n", p->name);
                                continue;
                            }

                            //check the length of the guess
                            if (len_input == 0){
                                char *empty_guess = "Your guess should be a lowercase letter from a to z:\r\n";
                                write(cur_fd, empty_guess, strlen(empty_guess));
                                continue;
                            }
                            if (len_input > 1){
                                char *too_long = "Your guess should be only one letter:\r\n";
                                write(cur_fd, too_long, strlen(too_long));
                                continue;
                            }

                            //check if the guess is digit
                            if (isdigit(guess)){
                                char *no_digit = "Your guess should be a lowercase letter from a to z:\r\n";
                                write(cur_fd, no_digit, strlen(no_digit));
                                continue;
                            }

                            //check if the guess is between a and z
                            if ((guess < 'a') || (guess > 'z')){
                                char *lower_case = "Your guess should be a lowercase letter from a to z:\r\n";
                                write(cur_fd, lower_case, strlen(lower_case));
                                continue;
                            }

                            //check if the guess been guessed before
                            if (game.letters_guessed[guess-97] == 1){
                                char *have_guessed = "This letter has been guessed, please guess another one:\r\n";
                                write(cur_fd, have_guessed, strlen(have_guessed));
                                continue;
                            }
                            game.letters_guessed[guess-97] = 1;
                            int signal = 1;

                            //record what letters have been guessed
                            for (int i = 0; i < strlen(game.word); i++){
                                if (guess == (game.word)[i]){
                                    (game.guess)[i] = guess;
                                    signal = 0;
                                }
                            }
                            int over = 1;

                            //the guess is not in the word
                            if (signal == 1) {
                                char wrong_guess[MAX_BUF];
                                sprintf(wrong_guess, "%c is not in the word\r\n", guess);
                                printf("Letter %c is not in the word\n", guess);
                                write(cur_fd, wrong_guess, strlen(wrong_guess));
                                game.guesses_left--;
                                advance_turn(&game);
                                /* if there is no more guesses, then game over, 
                                a new game starts*/
                                if (game.guesses_left <= 0) {
                                    char game_over[MAX_BUF];
                                    sprintf(game_over, "No more guesses. The word was %s\r\n\r\nLet's start a new game.\r\n", game.word);
                                    broadcast(&game, game_over);
                                    printf("Game over. No one won.\n");
                                    over = 0;
                                    init_game(&game, argv[1]);
                                    printf("New game\n");
                                    broadcast_turn_info(&game);
                                    announce_turn(&game);
                                }
                            }

                            //check if the game is over and annouce the winner
                            if (strchr(game.guess, '-') == NULL) {
                                announce_winner(&game, p);
                                printf("Game over. %s won!\n", p->name);
                                //start a new game
                                init_game(&game, argv[1]);
                                printf("New game\n");
                                broadcast_turn_info(&game);
                                announce_turn(&game);
                                break;
                            }

                            //broadcast the guess of the current player
                            if (over == 1){
                                char guess_info[MAX_BUF];
                                sprintf(guess_info, "%s guesses: %c\r\n", p->name, guess);
                                broadcast(&game, guess_info);
                                broadcast_turn_info(&game);
                                announce_turn(&game);
                            }
                        }
                    break;
                    }
                }

                // Check if any new players are entering their names
                for(p = new_players; p != NULL; p = p->next) {
                    if(cur_fd == p->fd) {
                        // TODO - handle input from an new client who has
                        // not entered an acceptable name.

                        //Read the input name from the player
                        char name[MAX_NAME];
                        int num_read = read(cur_fd, name, MAX_NAME);
                        if (num_read == -1) {
                            perror("read");
                            exit(1);
                        }

                        //Put the null terminator
                        name[num_read] = '\0';
                        int flag = 1;

                        //check if there is network newline in the input
                        if (partial_read(name, &p, num_read) == 1){
                                strcpy(p->name, p->inbuf);

                                //initialize the client inbuf
                                for (int i = 0; i < strlen(p->inbuf); i++) {
                                    (p->inbuf)[i] = '\0';
                                }

                                //empty name
                                if (strlen(p->name) == 0){
                                    char *msg1 = "Empty name, try another one:\r\n";
                                    write(cur_fd, msg1, strlen(msg1));
                                    flag = 0;
                                }

                                //name has been already used
                                for (struct client *k = game.head; k != NULL; k = k->next) {
                                    if (strcmp(p->name, k->name) == 0) {
                                        char *msg2 = "This name is already existed, try another one:\r\n";
                                        write(cur_fd, msg2, strlen(msg2));
                                        p->name[0] = '\0';
                                        flag = 0;
                                    }
                                }

                                /* if the name is not valid, 
                                then ask the client to type again*/
                                if (flag == 0) {
                                    break;
                                }

                                //put the new player to the head of the game
                                p->next = game.head;
                                game.head = p;

                                //check if this is the first player
                                if (game.has_next_turn == NULL){
                                    game.has_next_turn = p;
                                }

                                //broadcast the new player and turn information
                                char new_player_msg[MAX_BUF];
                                sprintf(new_player_msg, "%s has just joined.\r\n", p->name);
                                printf("%s has just joined.\n", p->name);
                                broadcast(&game, new_player_msg);
                                char *turn_info = malloc(sizeof(char)*MAX_BUF);
                                if (!turn_info) {
                                    perror("malloc");
                                    exit(1);
                                }
                                turn_info = status_message(turn_info, &game);
                                write(cur_fd, turn_info, strlen(turn_info));
                                free(turn_info);

                                //remove this player from the linked list
                                struct client *cur = new_players;
                                struct client *pre = new_players;
                                while (cur && cur->fd != cur_fd) {
                                    pre = cur;
                                    cur = cur->next;
                                }
                                if (cur == pre) {
                                    new_players = NULL;
                                }
                                if (cur != NULL) {
                                pre->next = cur->next;
                                }
                                //broadcast whose turn it is
                                announce_turn(&game);
                        }
                        break;
                    }
                }
            }
        }
    }
    return 0;
}


