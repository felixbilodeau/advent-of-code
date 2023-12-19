#include <stdio.h>

#define BUFF_SIZE 64

size_t get_next_step(char buff[], size_t buff_size)
{
    size_t i;
    int c;
    for (
        i = 0;
        (c = getchar()) != EOF
        && c != '\n'
        && c != ','
        && i < buff_size;
        ++i
    )
        buff[i] = (char)c;
    if (i == buff_size)
        puts("WARNING: Could not read entire step into buffer");
    buff[i] = '\0';
    return i;
}

size_t hash_single_char(char c, size_t last_hash)
{
    last_hash = last_hash + (size_t)c;
    last_hash = 17 * last_hash;
    return last_hash % 256;
}

size_t hash_string(char *str)
{
    size_t hash;

    hash = 0;
    while (*str) {
        hash = hash_single_char(*str, hash);
        ++str;
    }
    return hash;
}

int main(void)
{
    char buff[BUFF_SIZE];
    size_t total;

    total = 0;
    while (get_next_step(buff, BUFF_SIZE))
        total = total + hash_string(buff);
    printf("Total = %zu\n", total);
    
    return 0;
}
