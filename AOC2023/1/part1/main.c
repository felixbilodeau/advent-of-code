#include <stdio.h>

int main()
{
    size_t result;
    int c;
    char first_digit, last_digit;

    result = 0;
    first_digit = last_digit = '\0';
    while ((c = getchar()) != EOF) {
        if (c >= '0' && c <= '9') {
            if (!first_digit)
                first_digit = (char)c;
            last_digit = (char)c;
        } else if (c == '\n') {
            result = result + (size_t)(first_digit - '0') * 10 + (size_t)(last_digit - '0');
            first_digit = last_digit = '\0';
        }
    }

    printf("result = %zu\n", result);
    return 0;
}
