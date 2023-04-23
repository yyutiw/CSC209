#include <stdio.h>
#include <stdlib.h>

// prototype for check_permissions
int check_permissions(char *, char *);

int check_permissions(char *permission, char *required)
{
    for (int i = 0; i < 9; i++)
    {
        if (required[i] == '-')
            continue;
        else if (permission[i + 1] != required[i])
            return 0; // permissions are not equal, return 0
    }
    // permissions should be the same
    return 1;
}

int main(int argc, char **argv)
{
    if (!(argc == 2 || argc == 3))
    {
        fprintf(stderr, "USAGE: count_large size [permissions]\n");
        return 1;
    }
    // process command line arguments
    int target_size = strtol(argv[1], NULL, 10);
    char *target_perm;
    if (argc == 3)
        target_perm = argv[2];

    int inputs;
    char permission[10]; // included first character lol
    int size, count = 0;

    scanf("%*s %*d"); // ignore first line
    do
    {
        inputs = scanf("%s %*d %*s %*s %d %*s %*d %*s %*s", permission, &size);

        // check size
        if (size < target_size)
            continue;

        // check permissions if needed and add to count
        if (argc == 3 && check_permissions(permission, target_perm))
            count += 1;
        else if (argc == 2)
            count += 1;
    } while (inputs != EOF);

    printf("%d\n", count);

    return 0;
}
