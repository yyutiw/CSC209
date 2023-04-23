#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Invalid\n");
    } else if (strcmp(argv[1], argv[2]) == 0) {
        printf("Same\n");
    } else {
        printf("Different\n");
    }
    return 0;
}
