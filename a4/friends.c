#include "friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// wrapper function to error check malloc
void *Malloc(int size) {
    void *pt = malloc(size);
    if (pt == NULL) {
        perror("malloc");
        exit(1);
    }
    return pt;
}

/*
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 */
User *find_user(const char *name, const User *head) {
    while (head != NULL && strcmp(name, head->name) != 0) {
        head = head->next;
    }

    return (User *)head;
}

// create a new user and add to the list pointed to by user_ptr_add
// slightly modified to remove return values and instead truncate the name if it is too long
User *create_user(const char *name, User **user_ptr_add) {
    // get name of user
    char n_name[strlen(name) + 1]; 
    strncpy(n_name, name, strlen(name) + 1);
    // truncate name (shouldn't happen because we checked for this earlier)
    if (strlen(name) >= MAX_NAME) {
        n_name[MAX_NAME - 1] = '\0';
    }

    User *new_user = Malloc(sizeof(User));

    strncpy(new_user->name, n_name, MAX_NAME); // name has max length MAX_NAME - 1

    for (int i = 0; i < MAX_NAME; i++) {
        new_user->profile_pic[i] = '\0';
    }

    new_user->first_post = NULL;
    new_user->next = NULL;
    for (int i = 0; i < MAX_FRIENDS; i++) {
        new_user->friends[i] = NULL;
    }

    // Add user to list
    User *prev = NULL;
    User *curr = *user_ptr_add;
    while (curr != NULL) {
        prev = curr;
        curr = curr->next;
    }
    if (*user_ptr_add == NULL) {
        *user_ptr_add = new_user;
    }

    else {
        prev->next = new_user;
    }

    return new_user;
}

/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents, Client *connections) {
    if (target == NULL || author == NULL) { // one of the users does not exist
        return 2;
    }

    // check if users aren't friends
    int friends = 0;
    for (int i = 0; i < MAX_FRIENDS && target->friends[i] != NULL; i++) {
        if (strcmp(target->friends[i]->name, author->name) == 0) {
            friends = 1;
            break;
        }
    }

    if (friends == 0) {
        return 1;
    }

    // Create post
    Post *new_post = Malloc(sizeof(Post));
    strncpy(new_post->author, author->name, MAX_NAME);
    new_post->contents = contents;
    new_post->date = Malloc(sizeof(time_t));
    time(new_post->date);
    new_post->next = target->first_post;
    target->first_post = new_post;

    // inform recipient of post
    char msg[5 + sizeof(new_post->author) + sizeof(new_post->contents) + 2]; // From <author>: <contents>\r\n
    sprintf(msg, "From %s: %s\r\n", new_post->author, new_post->contents);
    Write(find_client_soc(target, connections), msg, strlen(msg));

    return 0;
}

/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv, int soc) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);    
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            char msg[BUF_SIZE];
            sprintf(msg, "Too many arguments!\r\n");
            Write(soc, msg, strlen(msg));
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }

    return cmd_argc;
}

/*
 * Make two users friends with each other.  This is symmetric - a pointer to
 * each user must be stored in the 'friends' array of the other.
 *
 * New friends must be added in the first empty spot in the 'friends' array.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the two users are already friends.
 *   - 2 if the users are not already friends, but at least one already has
 *     MAX_FRIENDS friends.
 *   - 3 if the same user is passed in twice.
 *   - 4 if at least one user does not exist.
 *
 * Do not modify either user if the result is a failure.
 * NOTE: If multiple errors apply, return the *largest* error code that applies.
 */
int make_friends(User *user1, const char *name2, User *head, Client *connections) {
    User *user2 = find_user(name2, head);

    if (user1 == NULL || user2 == NULL) { // one doesn't exist
        return 4;
    } else if (user1 == user2) { // Same user
        return 3;
    }

    int i, j;
    for (i = 0; i < MAX_FRIENDS; i++) {
        if (user1->friends[i] == NULL) { // Empty spot
            break;
        } else if (user1->friends[i] == user2) { // Already friends.
            return 1;
        }
    }

    for (j = 0; j < MAX_FRIENDS; j++) {
        if (user2->friends[j] == NULL) { // Empty spot
            break;
        }
    }

    if (i == MAX_FRIENDS || j == MAX_FRIENDS) { // Too many friends.
        return 2;
    }

    user1->friends[i] = user2;
    user2->friends[j] = user1;

    // inform both users that they are now friends
    char msg[BUF_SIZE];
    sprintf(msg, "You have been friended by %s.\r\n", user1->name);
    Write(find_client_soc(user2, connections), msg, strlen(msg));

    sprintf(msg, "You are now friends with %s.\r\n", user2->name);
    Write(find_client_soc(user1, connections), msg, strlen(msg));

    return 0;
}

/* 
 * Read and process commands
 * Return:  -1 for quit command
 *          0 otherwise
 */
int process_args(Client **client_user, int cmd_argc, char **cmd_argv, User **user_list_ptr, Client *connections) {
    User *user_list = *user_list_ptr;
    Client *curr_client = *client_user;
    User *curr_user = curr_client->user;
    int soc = curr_client->client_soc;
    char msg[BUF_SIZE];

    if (cmd_argc <= 0) {
        return 0;
    } else if (curr_user == NULL) {  // user has just entered their username
        int type = login(client_user, user_list_ptr, &curr_user, cmd_argv[0]);
        curr_user = curr_client->user;
        if (type == 0) {
            // say welcome back message and stuff
            // write to user
            sprintf(msg, "Welcome back.\r\n");
            Write(curr_client->client_soc, msg, strlen(msg));
        } else {
            // write to user
            sprintf(msg, "Welcome.\r\n");
            Write(curr_client->client_soc, msg, strlen(msg));
        }

        // this message is printed to both user types
        // write to user
        sprintf(msg, "Go ahead and enter user commands>\r\n");
        Write(curr_client->client_soc, msg, strlen(msg));
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return -1;
    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
        char *users = list_users(user_list);
        char user_list[strlen(users)];
        // write to user
        sprintf(user_list, "%s\r\n", users);
        Write(soc, user_list, strlen(user_list));
        free(users);

    } else if (strcmp(cmd_argv[0], "make_friends") == 0 && cmd_argc == 2) {
        switch (make_friends(curr_user, cmd_argv[1], user_list, connections)) {
            case 1:
                // write to user
                sprintf(msg, "You are already friends\r\n");
                Write(soc, msg, strlen(msg));
                break;
            case 2:
                // write to user
                sprintf(msg, "At least one of you entered has the max number of friends\r\n");
                Write(soc, msg, strlen(msg));
                break;
            case 3:
                // write to user
                sprintf(msg, "You can't friend yourself bozo\r\n");
                Write(soc, msg, strlen(msg));
                break;
            case 4:
                // write to user
                sprintf(msg, "The user you entered does not exist\r\n");
                Write(soc, msg, strlen(msg));
                break;
        }
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc >= 3) {
        // first determine how long a string we need
        int space_needed = 0;
        for (int i = 3; i < cmd_argc; i++) {
            space_needed += strlen(cmd_argv[i]) + 1;
        }

        // allocate the space
        char *contents = Malloc(space_needed);

        // copy in the bits to make a single string
        strcpy(contents, cmd_argv[2]);
        for (int i = 3; i < cmd_argc; i++) {
            strcat(contents, " ");
            strcat(contents, cmd_argv[i]);
        }

        User *author = curr_user;
        User *target = find_user(cmd_argv[1], user_list);
        switch (make_post(author, target, contents, connections)) {
            case 1:
                // write to user
                sprintf(msg, "You can only post to your friends\r\n");
                Write(soc, msg, strlen(msg));
                break;
            case 2:
                // write to user
                sprintf(msg, "The user you want to post to does not exist\r\n");
                Write(soc, msg, strlen(msg));
                break;
        }
    } else if (strcmp(cmd_argv[0], "profile") == 0 && cmd_argc == 2) {
        User *user = find_user(cmd_argv[1], user_list);
        char *res = print_user(user);
        if (res == NULL) {
            // write to user
            sprintf(msg, "User not found\r\n");
            Write(soc, msg, strlen(msg));
        } else {
            char profile[strlen(res) + 2];
            // write to user
            sprintf(profile, "%s\r\n", res);
            Write(soc, profile, strlen(profile));
            free(res);
        }
    } else {
        // write to user
        sprintf(msg, "Incorrect syntax\r\n");
        Write(soc, msg, strlen(msg));
    }
    return 0;
}

/*
 *  Print a post.
 *  Use localtime to print the time and date.
 */
char *print_post(const Post *post) {
    if (post == NULL) {
        return NULL;
    }
    int total_size = 0;
    // Print author
    int size1 = strlen(post->author) + 8; // add "From: \n" part
    char part1[size1]; // part 1 of the final result
    snprintf(part1, size1, "From: %s\n", post->author);
    total_size += size1;

    // Print date
    int size2 = strlen(asctime(localtime(post->date))) + 8;
    char part2[size2];
    snprintf(part2, size2, "Date: %s\n", asctime(localtime(post->date)));
    total_size += size2;

    // Print message
    int size3 = strlen(post->contents) + 2;
    char part3[size3];
    snprintf(part3, size3, "%s\n", post->contents);
    total_size += size3; 

    // create resulting string
    char *result = Malloc(total_size + 1); // make room for null terminator here
    strncpy(result, part1, size1);
    strncat(result, part2, size2);
    strncat(result, part3, size3);
    result[total_size] = '\0';

    return result;
}

/*
 * Print a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return a dynamically allocated string of the user's profile, or NULL if it doesn't exist
 */
char *print_user(const User *user) {
    if (user == NULL) {
        return NULL;
    }
    int size = 0;

    // get size of name
    size += strlen("Name: \n\n") + strlen(user->name);
    size += strlen("------------------------------------------\n");

    // get size of friend list.
    size += strlen("Friends:\n");
    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
        size += 1 + strlen(user->friends[i]->name); // + 1 for newline
    }
    size += strlen("------------------------------------------\n");

    // get size of post list.
    size += strlen("Posts:\n");
    const Post *curr = user->first_post;
    while (curr != NULL) {
        size += strlen(print_post(curr));
        curr = curr->next;
        if (curr != NULL) {
            size += strlen("\n===\n\n");
        }
    }
    size += strlen("------------------------------------------\n");

    // now, create the resulting dynamically allocated string
    char *result = Malloc(size + 1);
    // add header ("Name: <name here>")
    strcpy(result, "Name: ");
    strcat(result, user->name);
    strcat(result, "\n\n");

    // add friends section
    strcat(result, "Friends:\n");
    // add each friend to list
    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
        strcat(result, user->friends[i]->name);
        strcat(result, "\n");
    }
    strcat(result, "------------------------------------------\n");

    // add posts section
    strcat(result, "Posts:\n");
    // loop through posts again
    curr = user->first_post;
    while (curr != NULL) {
        char *p = print_post(curr);
        strcat(result, p);
        free(p);
        curr = curr->next;
        if (curr != NULL) {
            strcat(result, "\n===\n\n");
        }
    }
    strcat(result, "------------------------------------------\r\n");
    result[size] = '\0'; // null terminate
    
    return result;
}

/*
 * Print the usernames of all users in the list starting at curr.
 */
char *list_users(const User *curr) {
    const User *curr1 = curr; // because we loop twice
    int size = 0;
    while (curr1 != NULL) {
        // get size of string to add to counter
        size += 2; // for the new line and tab
        size += strlen(curr1->name);
        curr1 = curr1->next;
    }
    // dynamically allocate string to be returned
    size += sizeof("User List\r\n"); // size of "User List\r\n"
    char *result = Malloc(size + 1);
    strncpy(result, "User List\r\n", size);  // add heading to result
    
    // add rest of names
    while (curr != NULL) {
        // obtain name with proper list formatting
        char name[strlen(curr->name) + 2];
        snprintf(name, strlen(curr->name) + 5, "\t%s\r\n", curr->name);

        strcat(result, name); // add name to result
        curr = curr->next;
    }
    result[strlen(result) - 1] = '\0'; // null terminate

    return result;
}