#include <stdio.h>

int main() {
    char phone[11];
    int num;

    scanf("%s", phone);
    while (scanf("%d", &num) != 0) {
        if (num == -1) {
            printf("%s\n", phone);
        } else if ((num < -1) | (num > 9)) {
            printf("ERROR\n");

            return 1;
        } else {
            printf("%c\n", phone[num]);
        }
    }
    return 0;
}