#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "pmake.h"

Rule *exists_in_rules(Rule *rules, char *target);
char *args_to_string(char **args, char *buffer, int size);

// wrapper function to error check close
void Close(int fd) {
  if (close(fd) < 0) {
    perror("close");
    exit(1);
  }
}
// run all actions in a rule
void run_args(Rule *rule_pt, int pflag) {
  Action *act = rule_pt->actions;
  int act_count = 0;
  while (act != NULL) {
    act_count ++;
    int result = fork();
    if (result < 0) {
      perror("fork");
      exit(1);
    } else if (result == 0) { // child
      // convert array of args into string
      char *line = malloc(sizeof(char *) * sizeof(act->args) + 1);
      strncpy(line, args_to_string(act->args, line, MAXLINE), strlen(args_to_string(act->args, line, MAXLINE)));
      line[strlen(line)] = '\0';
      printf("%s\n", line);
      free(line);

      // run command
      execvp(act->args[0], act->args);
      perror("execvp");
      exit(1);
    }
    act = act->next_act;
  }
  // should be in parent process, wait for all children
  int status;
  int exit_parent = 0, exit_status = 0;
  for (int i = 0; i < act_count; i++) {
    wait(&status); // wait for child
    if (WIFEXITED(status) == 1 && WEXITSTATUS(status) != 0) {
      exit_parent = 1;
      exit_status = (WEXITSTATUS(status));
    }
  }
  if (exit_parent == 1) { // exit with nonzero status if any child did
    exit(exit_status);
  }
}
// check recency of a Dependency d compared to a target name
// return 1 if the rule needs to be run (d is newer), and 0 otherwise
int check_dep(char *target, Dependency *d) {
  // get time modified
  struct stat depstat, tarstat;
  stat(d->rule->target, &depstat);
  stat(target, &tarstat);

  // dependency is newer
  if (depstat.st_mtim.tv_sec > tarstat.st_mtim.tv_sec || 
  (depstat.st_mtim.tv_sec == tarstat.st_mtim.tv_sec && depstat.st_mtim.tv_nsec > tarstat.st_mtim.tv_nsec)) {
    return 1; // signal to run the rule
  }

  return 0;
}

/* Evaluate the rule in rules corresponding to the given target.
   If target is NULL, evaluate the first rule instead.
   If pflag is 0, evaluate each dependency in sequence.
   If pflag is 1, then evaluate each dependency in parallel (by creating one 
   new process per dependency). In this case, the parent process will wait until
   all child processes have terminated before checking dependency modified times
   to decide whether to execute the actions.
 */
void run_make(char *target, Rule *rules, int pflag) {
  Rule *rule_pt;
  int run_target = 0, child_run_target = 0; // whether the rule needs to be executed
  int fd[2]; // pipe for parallel processes
  // obtain rule pointer corresponding to target
  if (target == NULL) { // use first rule
    rule_pt = rules;
  } else { // rule is in the makefile
    rule_pt = exists_in_rules(rules, target);
  }
  // target doesn't exist in rules
  if (rule_pt == NULL) {
    if (access(target, F_OK) != 0) { // target doesn't exist
      printf("Target \"%s\" does not exist.\n", target);
      exit(1);
    }
  } else {
    // run_make on all dependencies
    Dependency *curr_d = rule_pt->dependencies;
    int result = 0; // default 0 so dependencies are run in sequence if pflag == 0
    int dep_count = 0; // count dependencies
    if (pflag == 1 && pipe(fd) == -1) { // create pipe
      perror("pipe");
      exit(1);
    }
    // go through all dependencies in sequence
    while (curr_d != NULL) {
      dep_count ++;
      if (pflag == 1) { // run in parallel
        result = fork();
      }
      if (result < 0) { // error
        perror("fork");
        exit(1);
      } else if (result == 0) { // child
        run_make(curr_d->rule->target, rules, pflag); // call make on dependency
        run_target = check_dep(target, curr_d);
        if (pflag == 1) {
          Close(fd[0]); // close read end of child
          // write run_target value to parent process
          if (write(fd[1], &run_target, sizeof(int)) < 0) {
            perror("write");
            exit(1);
          } 
          Close(fd[1]);
          exit(0);
        }
      }
      curr_d = curr_d->next_dep;
    }
    if (pflag == 1) { // should be in parent process now
      int status, exit_parent =  0, exit_status = 0; // check exit values
      Close(fd[1]); // close write end
      // read run_target value from child
      if (read(fd[0], &child_run_target, sizeof(int)) < 0) {
        perror("read");
        exit(1);
      }
      Close(fd[0]); // close read end since we're done with it
      if (child_run_target == 1) {
        run_target = 1;
      }
      for (int i = 0; i < dep_count; i++) { // wait for all children to end
        wait(&status);
        // check if any child terminated with nonzero exit status
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
          exit_parent = 1;
          exit_status = WEXITSTATUS(status);
        }
      }
      // wait for all processes to end before exiting in parent
      if (exit_parent == 1) {
        exit(exit_status);
      }
    }
  }
  // target doesn't exist in files
  if (run_target == 0 && access(target, F_OK) != 0) {
    run_target = 1;
  }

  // run rule if necessary
  if (run_target == 1) {
    run_args(rule_pt, pflag);
  }
}