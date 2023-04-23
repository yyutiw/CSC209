#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pmake.h"

// wrapper function to error check malloc
void *Malloc(int size) {
    void *pt = malloc(size);
    if (pt == NULL) {
        perror("malloc");
        exit(1);
    }
    return pt;
}
// make a rule with the given target in the heap
// return a pointer to the rule
Rule *make_rule(char *target) {
    Rule *r = Malloc(sizeof(Rule));
    // initialize values
    r->target = Malloc(strlen(target) + 1);
    strncpy(r->target, target, strlen(target));
    r->target[strlen(target)] = '\0';
    r->dependencies = NULL;
    r->actions = NULL;
    r->next_rule = NULL;

    return r;
}
/*
 returns a pointer to the rule if the rule exists in the linked list rules
 returns NULL otherwise
*/
Rule *exists_in_rules(Rule *rules, char *target) {
    Rule *curr = rules;
    while (curr != NULL) { // search for target rule
        if (strcmp(curr->target, target) == 0) {
            return curr; // rule found
        }
        curr = curr->next_rule;
    }
    // couldn't find a rule with that target
    return NULL;
}
/*
 find the last action in the linked list of actions
 head should not be NULL
*/
Action *find_last(Action *head) {
    Action *curr = head;
    while (curr->next_act != NULL) {
        curr = curr->next_act;
    }
    return curr;
}
// convert a space-separated line of strings into an array
char **str_to_args(char *str) {
    char **result = Malloc(sizeof(char *));
    char *word;
    char s[2] = " ";
    word = strtok(str, s);
    int count = 0;
    while (word != NULL) {
        result[count] = Malloc(sizeof(char) * (strlen(word) + 1));
        strncpy(result[count], word, strlen(word));
        result[count][(strlen(word))] = '\0';
        count ++;
        result = realloc(result, sizeof(char *) * (count + 1));
        word = strtok(NULL, s);
    } 

    return result;
}

/* Read from the open file fp, and create the linked data structure
   that represents the Makefile contained in the file.
   See the top of pmake.h for the specification of Makefile contents.
 */
Rule *parse_file(FILE *fp) {
    Rule *head = NULL;
    Rule *new_rules = head; // so head can be saved
    Rule *not_in_rules = NULL; // keep track of dependencies that may or may not be targets
    char line[MAXLINE] = "";

    // read entire file
    while (fgets(line, MAXLINE, fp) != NULL) {
        // remove end of line
        line[strcspn(line, "\n")] = '\0';
        line[strcspn(line, "\r")] = '\0';
        if (is_comment_or_empty(line) == 1 || strlen(line) == 0) { // comment or empty line
            continue;
        } else if (line[0] == '\t') { // action line, should only occur if a rule exists
            // create new action
            const char s[2] = "\t";
            char *act = strtok(line, s);

            Action *a = Malloc(sizeof(Action));
            a->args = str_to_args(act);
            a->next_act = NULL;
            
            // check if the rule has actions yet
            // if not, set this action as its head
            // otherwise traverse the list and add to the end
            if (new_rules->actions == NULL) {
                new_rules->actions = a;
            } else {
                Action *last = find_last(new_rules->actions);
                last->next_act = a;
            }
        } else { // target line
            // locate target
            char *colon = strchr(line, ':'); // find location of colon in line
            int target_size = strlen(line) - strlen(colon); // might be wrong, length of target
            
            // obtain target
            char target[target_size];
            strncpy(target, line, target_size - 1);
            target[target_size - 1] = '\0';

            // check if head is NULL
            // if it is, make a new rule
            if (head == NULL) {
                head = make_rule(target);
                new_rules = head;
            } else {
                // check if the rule already exists in not_in_rules
                // if it does, add it to new_rules
                Rule *rule_pt = exists_in_rules(not_in_rules, target);
                if (rule_pt != NULL) {
                    new_rules->next_rule = rule_pt;
                } else { // if not, make a new rule
                    new_rules->next_rule = make_rule(target);
                }

                new_rules = new_rules->next_rule; // we are working with a new rule now
            }

            // add dependencies to rule
            Dependency *d = NULL;
            Dependency *curr_d = d;
            Rule *dep_rule;
            char *strdep;
            const char s[2] = " ";
            strdep = strtok(colon + 1, s); // obtain a dependency name
            while (strdep != NULL) {
                // check if rule exists with dependency name ...
                dep_rule = exists_in_rules(head, strdep); // ... in rules LL
                if (dep_rule == NULL) dep_rule = exists_in_rules(not_in_rules, strdep); // ... in other rules
                if (dep_rule == NULL) { // does not exist in either
                    dep_rule = make_rule(strdep); // make a new rule
                } 
                // add new dependency
                if (d == NULL) { // first dependency
                    d = Malloc(sizeof(Dependency));
                    d->next_dep = NULL;
                    curr_d = d;
                } else { // not first dependency
                    curr_d->next_dep = Malloc(sizeof(Dependency));
                    curr_d = curr_d->next_dep;
                }
                // initialize dependency values
                curr_d->rule = dep_rule;
                curr_d->next_dep = NULL;

                strdep = strtok(NULL, s); // obtain a dependency name
            }
            new_rules->dependencies = d;
        }
    }
    return head;
}


/******************************************************************************
 * These helper functions are provided for you. Do not modify them.
 *****************************************************************************/
/* Print the list of actions */
void print_actions(Action *act) {
    while(act != NULL) {
        if(act->args == NULL) {
            fprintf(stderr, "ERROR: action with NULL args\n");
            act = act->next_act;
            continue;
        }
        printf("\t");

        int i = 0;
        while(act->args[i] != NULL) {
            printf("%s ", act->args[i]) ;
            i++;
        }
        printf("\n");
        act = act->next_act;
    }
}

/* Print the list of rules to stdout in makefile format. If the output
   of print_rules is saved to a file, it should be possible to use it to
   run make correctly.
 */
void print_rules(Rule *rules){
    Rule *cur = rules;

    while (cur != NULL) {
        if (cur->dependencies || cur->actions) {
            // Print target
            printf("%s : ", cur->target);

            // Print dependencies
            Dependency *dep = cur->dependencies;
            while (dep != NULL){
                if(dep->rule->target == NULL) {
                    fprintf(stderr, "ERROR: dependency with NULL rule\n");
                }
                printf("%s ", dep->rule->target);
                dep = dep->next_dep;
            }
            printf("\n");

            // Print actions
            print_actions(cur->actions);
        }
        cur = cur->next_rule;
    }
}


/* Return 1 if the line is a comment line, as defined on the assignment handout.
   Return 0 otherwise.
 */
int is_comment_or_empty(const char *line) {
    for (int i = 0; i < strlen(line); i++){
        if (line[i] == '#') {
            return 1;
        }
        if (line[i] != '\t' && line[i] != ' ') {
            return 0;
        }
    }
    return 1;
}

/* Convert an array of args to a single space-separated string in buffer.
   Returns buffer.  Note that memory for args and buffer should be allocted
   by the caller.
 */
char *args_to_string(char **args, char *buffer, int size) {
    buffer[0] = '\0';
    int i = 0;
    while (args[i] != NULL) {
        strncat(buffer, args[i], size - strlen(buffer));
        strncat(buffer, " ", size - strlen(buffer));
        i++;
    }
    return buffer;
}