#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];
  int status;

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  int pipe_fd[2];

  if ((pipe(pipe_fd)) == -1) {
      perror("pipe");
      exit(1);
  }
  int result = fork();
  if (result < 0){
      perror("fork");
      exit(1);
  } else if (result == 0) {
      dup2(pipe_fd[0], STDIN_FILENO);
      if (close(pipe_fd[0]) == -1){
          perror("close reading end from inside child");
          exit(1);
      }
      if (close(pipe_fd[1]) == -1){
          perror("close pipe after writing");
          exit(1);
      }
      execl("./validate", "validate", NULL);
      perror("execl");
      exit(1);
  }else{
      if (close(pipe_fd[0]) == -1){
          perror("close reading of pipe in parent");
          exit(1);
      }
      if ((write(pipe_fd[1], user_id, MAX_PASSWORD)) == -1){
          perror("write");
          exit(1);
      }
      if ((write(pipe_fd[1], password, MAX_PASSWORD)) == -1){
          perror("write");
          exit(1);
      }
      if (close(pipe_fd[1]) == -1){
          perror("close writing end of pipe in parent");
          exit(1);
      }
      if (wait(&status) == -1){
          perror("wait");
          exit(1);
      }
      if (WIFEXITED(status)){
          if (WEXITSTATUS(status) == 0){
              printf(SUCCESS);
          }else if (WEXITSTATUS(status) == 2){
              printf(INVALID);
          }else if (WEXITSTATUS(status) == 3){
              printf(NO_USER);
          }
      }
  }

  return 0;
}
