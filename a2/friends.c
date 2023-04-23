#include "friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * print the ascii photo specified in profile_pic
 * this function assumes that update_pic works correctly so the file exists and can be opened/read
 */
void print_pic(const char *profile_pic);
/**
 * remove the target from the user's friend list
 * function assumes that the target is in the user's friend list already
 */
void remove_friend(User *user, User *target);

/*
 * Create a new user with the given name.  Insert it at the tail of the list
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 if successful
 *   - 1 if a user by this name already exists in this list
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator)
 */
int create_user(const char *name, User **user_ptr_add)
{
    // check if name is in array already
    if (find_user(name, *user_ptr_add) != NULL)
    {
        return 1;
    }

    // create new user
    User *new_user = malloc(sizeof(User));
    if (new_user == NULL)
    {
        perror("malloc");
        exit(1);
    }

    if (strlen(name) >= MAX_NAME)
    { // name is too big
        return 2;
    }
    // name is not too big, proceed
    strcpy(new_user->name, name);
    new_user->name[strlen(name)] = '\0'; // add null terminator

    // initialize other stuff because it was causing me great pain
    for (int i = 0; i < MAX_FRIENDS; i++)
    {
        new_user->friends[i] = 0;
    }
    strcpy(new_user->profile_pic, "");
    new_user->first_post = 0;

    // add user to list
    User *curr = *user_ptr_add;
    if (curr == NULL)
    { // new_user is the first to be made
        *user_ptr_add = new_user;
        return 0;
    }
    // otherwise, traverse to last item
    while (curr->next != NULL)
    {
        curr = curr->next;
    }
    // add new_user
    new_user->next = curr->next; // should be null
    curr->next = new_user;

    return 0;
}

/*
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 *
 * NOTE: You'll likely need to cast a (const User *) to a (User *)
 * to satisfy the prototype without warnings.
 */
User *find_user(const char *name, const User *head)
{
    User *curr = (User *)head;
    // search for name
    while (curr != NULL)
    {
        if (strcmp(curr->name, name) == 0)
        {
            return curr;
        }
        curr = curr->next;
    }

    // will return NULL if the name wasn't found in the loop
    return NULL;
}

/*
 * Print the usernames of all users in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
void list_users(const User *curr)
{
    printf("User List\n");
    while (curr != NULL)
    {
        printf("\t%s\n", curr->name);
        curr = curr->next;
    }
}

/*
 * Change the filename for the profile pic of the given user.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the file does not exist or cannot be opened.
 *   - 2 if the filename is too long.
 */
int update_pic(User *user, const char *filename)
{
    // filename is too long
    if (strlen(filename) >= MAX_NAME)
    {
        return 2;
    }
    // make sure file exists/can be opened
    FILE *pfp = fopen(filename, "r");
    if (pfp == NULL)
    { // error with fopen
        return 1;
    }
    else
    {
        fclose(pfp); // we only need to file name
    }

    // everything checked, nothing is wrong
    strcpy(user->profile_pic, filename);
    user->profile_pic[strlen(filename)] = '\0';

    return 0;
}

// this is celina's
int list_length(User *friends[]) {
    int i = 0;
    while (friends[i] != NULL && i < MAX_FRIENDS) {
        i ++;
    }
    return i;
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
int make_friends(const char *name1, const char *name2, User *head)
{
    // locate users
    User *user1 = find_user(name1, head);
    User *user2 = find_user(name2, head);

    // at least one user does not exist -- return 4
    if (user1 == NULL || user2 == NULL)
    {
        return 4;
    }
    else if (user1 == user2)
    { // same user passed in twice
        return 3;
    }

    // attempt to make the users friends
    // store indices for user1 and user2's friend list:
    int user1_friend = 0;
    int user2_friend = 0;

    // traverse each friends array until we reach the end or an empty spot has been found in both
    while ((user1_friend < MAX_FRIENDS && user2_friend < MAX_FRIENDS) &&
           (user1->friends[user1_friend]->name != NULL || user2->friends[user2_friend]->name != NULL))
    {

        // the users are already friends
        if ((user1->friends[user1_friend] == user2) || (user2->friends[user2_friend] == user1))
        {
            return 1;
        }

        // increment friend indices if they are not empty
        if (user1->friends[user1_friend]->name != NULL)
        {
            user1_friend++;
        }
        if (user2->friends[user2_friend]->name != NULL)
        {
            user2_friend++;
        }
    }
    // at this point user1_friend and user2_friend should both hold the first empty index of the FRIENDS array,
    // or one/both is equal to MAX_FRIENDS.
    // the users should not already be friends
    if (user1_friend == MAX_FRIENDS || user2_friend == MAX_FRIENDS)
    { // if one has max friends
        return 2;
    }

    // at last, make them friends
    user1->friends[user1_friend] = user2;
    user2->friends[user2_friend] = user1;

    return 0;
}

/**
 * print the ascii photo specified in profile_pic
 * this function assumes that update_pic works correctly so the file exists and can be opened/read
 */
void print_pic(const char *profile_pic)
{
    // open file
    FILE *photo = fopen(profile_pic, "rb");

    // print contents
    char c;
    while (fread(&c, sizeof(char), 1, photo) != 0)
    {
        printf("%c", c);
    }
    printf("\n");

    // close file
    fclose(photo);
}
/*
 * Print a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
int print_user(const User *user)
{
    // user is NULL
    if (user == NULL)
    {
        return 1;
    }
    // print pfp
    if (strcmp(user->profile_pic, "") != 0)
    {
        print_pic(user->profile_pic);
    }
    // print info
    printf("Name: %s\n------------------------------------------\n", user->name); // name
    printf("Friends:\n");
    int i = 0;
    while (i < MAX_FRIENDS && user->friends[i] != NULL)
    { // list friends
        printf("%s\n", user->friends[i]->name);
        i++;
    }
    printf("------------------------------------------\nPosts:\n");
    Post *curr = user->first_post;
    while (curr != NULL)
    { // list posts
        // formatting between posts
        if (curr != user->first_post)
        {
            printf("\n===\n\n");
        }

        // listed in multiple print statements for clarity
        printf("From: %s\n", curr->author);
        printf("Date: %s\n", ctime(curr->date));
        printf("%s\n", curr->contents);

        curr = curr->next;
    }
    printf("------------------------------------------\n");

    return 0;
}

/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * 'contents' is a pointer to heap-allocated memory - you do not need
 * to allocate more memory to store the contents of the post.
 *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents)
{
    // one of the users is NULL
    if (author == NULL || target == NULL)
    {
        return 2;
    }
    // check if the users are not friends
    int i = 0;
    // locate target in author's friends list or traverse entire array
    while (i < MAX_FRIENDS && (author->friends[i] != NULL && author->friends[i] != target))
    {
        i++;
    }
    // target was not found in author's friends list => they are not friends with each other
    if (i == MAX_FRIENDS)
    {
        printf("%d\n", i);
        return 1;
    }
    // users should both be friends at this point: make the new post
    Post *new_post = malloc(sizeof(Post));
    if (new_post == NULL)
    {
        perror("malloc");
        exit(1);
    }

    // add name of author
    strncpy(new_post->author, author->name, strlen(author->name));
    new_post->author[strlen(author->name)] = '\0';
    // add contents
    new_post->contents = contents;
    new_post->contents[strlen(contents)] = '\0'; // add null terminator
    // add dates
    new_post->date = malloc(sizeof(time_t));
    if (new_post->date == NULL)
    {
        perror("malloc");
        exit(1);
    }

    time(new_post->date); // current time

    // add to front of list
    if (target->first_post == NULL)
    { // no posts yet
        new_post->next = NULL;
        target->first_post = new_post;
    }
    else
    {
        new_post->next = target->first_post;
        target->first_post = new_post;
    }

    return 0;
}

/**
 * remove the target from the user's friend list
 * function assumes that the target is in the user's friend list already
 */
void remove_friend(User *user, User *target)
{
    int i = 0;
    // locate index of target
    while (user->friends[i] != target)
    {
        i++;
    }

    // remove target
    user->friends[i] = NULL;

    // a's friends array: b x x x x x x x *j* k

    // shift rest of elements up
    int j = 0; // j will point to the first null index of friends
    while (user->friends[j] != NULL) { 
        j++;
    }
    
    // switch index i and j until all of the null elements are at the end
    i += 1;
    while (i < MAX_FRIENDS && user->friends[i] != NULL) {
        user->friends[j] = user->friends[i];
        user->friends[i] = NULL;
        i++;
        j++;
    }
}

/*
 * From the list pointed to by *user_ptr_del, delete the user
 * with the given name.
 * Remove the deleted user from any lists of friends.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user with this name does not exist.
 */
int delete_user(const char *name, User **user_ptr_del)
{
    // get user pointer
    User *to_delete = find_user(name, *user_ptr_del);

    // user was not found => user does not exist
    if (to_delete == NULL)
    {
        return 1;
    }

    // first, delete user from all of its friends' friends lists
    int i = 0;
    while (i < MAX_FRIENDS && to_delete->friends[i] != NULL)
    {
        remove_friend(to_delete->friends[i], to_delete);
        i++;
    }

    // next, delete all of the user's posts (and freeing all associated heap memory)
    Post *curr = to_delete->first_post;
    Post *next;
    while (curr != NULL)
    {
        next = curr->next; // stores next value to go to since we're freeing the current item
        free(curr->date);
        free(curr->contents);
        free(curr);

        curr = next;
    }

    // finally, delete the user
    if (*user_ptr_del == to_delete)
    { // user being deleted is first in the list
        *user_ptr_del = (*user_ptr_del)->next;
        free(to_delete);
        return 0;
    }

    // otherwise search list for user
    // since the user is in the list (confirmed above), if we made it here it means that the list is at least 2 items long
    User *curr_user = *user_ptr_del;
    User *next_user = curr_user->next;
    // locate target
    while (next_user != NULL)
    {
        if (next_user == to_delete)
        { // located user to be removed
            curr_user->next = next_user->next;
            free(to_delete); // free associated mems

            break;
        }

        curr_user = curr_user->next;
        next_user = curr_user->next;
    }

    return 0;
}