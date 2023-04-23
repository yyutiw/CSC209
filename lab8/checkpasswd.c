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

void Close(int fd) {
  if (close(fd) == -1) {
    perror("close");
    exit(1);
  }
}

void Write(int fd) {
  if (write(fd) == -1) {
    perror("write");
    exit(1);
  }
}

int main(void) {
  char user_id[MAXLINE];
  char password[MAXLINE];

  /* The user will type in a user name on one line followed by a password 
     on the next.
     DO NOT add any prompts.  The only output of this program will be one 
	 of the messages defined above.
     Please read the comments in validate carefully
   */

  if(fgets(user_id, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  if(fgets(password, MAXLINE, stdin) == NULL) {
      perror("fgets");
      exit(1);
  }
  
  int fd[2];
  int status;
  if (pipe(fd) != 0) {
    perror("pipe");
    exit(1);
  }

  int result = fork();  
  if (result < 0) {
    perror(fork);
    exit(1);
  } else if (result > 0) { // parent
    Close(fd[0]);

    Write(fd[1], user_id, MAX_PASSWORD);
    Write(fd[1], password, MAX_PASSWORD);

    Close(fd[1]);

    if (wait(&status) == -1) {
      perror("wait");
      exit(1);
    }

    if (WIFEXITED(status)) {
      if (WEXITSTATUS(status) == 0) {
        printf(SUCCESS);
      } else if (WEXITSTATUS(status) == 1) {
        exit(1);
      } else if (WEXITSTATUS(status) == 2) {
        printf(INVALID);
      } else {
        printf(NO_USER);
      }
    }
    
  } else {
    // redirect input?
    if (dup2(fd[0], fileno(stdin)) == -1){
      perror("dup2");
      exit(1);
    }
    Close(fd[1]);
    Close(fd[0]);

    execl("./validate", "validate",  NULL);
    perror("execl");
    exit(1);
  }

  return 0;
}
