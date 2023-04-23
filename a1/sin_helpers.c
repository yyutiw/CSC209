/*
 * Convert a 9 digit int to a 9 element int array.
 */
int populate_array(int sin, int *sin_array) {
    int remainder;
    for (int i = 0; i < 9; i++) {
        remainder = sin % 10;
        sin_array[8-i] = remainder;
        sin = sin / 10;
    }
    return 0;
}
/*
 * Return 0 if the given sin_array is a valid SIN, and 1 otherwise.
 */
int check_sin(int *sin_array) {
    int new_sin[9];
    int sum = 0;
    int digit;
    for (int i = 0; i < 9; i++) {
        digit = sin_array[i];
        if (i % 2 == 1) {
            digit = digit * 2;
            if (digit >= 10) {
            digit = (digit % 10) + (digit / 10);
        }
        } 
        new_sin[i] = digit;
    }


    for (int j = 0; j < 9; j++) {
        sum += new_sin[j];
    }

    if (sum % 10 == 0) return 0;
    return 1;
}
