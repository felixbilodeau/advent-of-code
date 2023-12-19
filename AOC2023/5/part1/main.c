#include <stdio.h>

#define MAX_SIZE 65536
#define NUM_SEEDS 20

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int load_file(char buf[], size_t max_size)
{
    size_t i;
    int c;
    for (i = 0; (c = getchar()) != EOF && i < max_size - 1; ++i)
        buf[i] = c;
    buf[i] = '\0';
    if (c != EOF) {
        puts("Could not load entire file into memory!");
        return 0;
    }
    return 1;
}

int seek_next_colon(char **p_file)
{
    while (**p_file && **p_file != ':')
        ++(*p_file);
    return **p_file != '\0';
}

long get_number(char **p_file)
{
    long number, multiplier;
    char *start, *end;

    start = *p_file;
    while (**p_file && is_digit(**p_file))
        ++(*p_file);

    end = *p_file;
    if (!is_digit(**p_file))
        --(*p_file);

    number = 0;
    multiplier = 1;
    while (*p_file != start) {
        number = number + multiplier * (long)(**p_file - '0');
        multiplier = multiplier * 10;
        --(*p_file);
    }
    number = number + multiplier * (long)(**p_file - '0');

    *p_file = end;
    return number;
}

int load_seeds(char **p_file, long seeds[], size_t num_seeds)
{
    size_t i;
    seek_next_colon(p_file);
    if (*(*p_file + 1))
        ++(*p_file);
    else {
        puts("ERROR: Reached EOF while parsing");
        return 1;
    }
    
    i = 0;
    while (is_digit(*(++(*p_file))) && i < num_seeds) {
        seeds[i] = get_number(p_file);
        ++i;
    }
    return !is_digit(**p_file);
}

int convert_numbers(char **p_file, long numbers[], size_t max_num)
{
    long destination, source, range;
    int converted[max_num];
    size_t i;

    for (i = 0; i < max_num; ++i)
        converted[i] = 0;

    while (*(++(*p_file)) && is_digit(**p_file)) {
        destination = get_number(p_file);
        if (!**p_file || !*(++(*p_file))) return 0;
        source = get_number(p_file);
        if (!**p_file || !*(++(*p_file))) return 0;
        range = get_number(p_file);
        for (i = 0; i < max_num; ++i) {
            if (
                !converted[i]
                && numbers[i] - source < range
                && numbers[i] - source >= 0
            ) {
                numbers[i] = destination + numbers[i] - source;
                converted[i] = 1;
            }
        }
    }
    return 1;
}

int main(void)
{
    char buf[MAX_SIZE];
    char *file_pointer;
    size_t i;

    long numbers[NUM_SEEDS], min_location;
    
    if (!load_file(buf, MAX_SIZE)) return 1;
    file_pointer = buf;
    load_seeds(&file_pointer, numbers, NUM_SEEDS);
    while (seek_next_colon(&file_pointer)) {
        puts("converted!");
        if (!*(++file_pointer)) return 1;
        convert_numbers(&file_pointer, numbers, NUM_SEEDS);
    }
    for (i = 0; i < NUM_SEEDS; ++i)
        printf("%ld ", numbers[i]);
    putchar('\n');
    min_location = numbers[0];
    for (i = 1; i < NUM_SEEDS; ++i)
        if (numbers[i] < min_location)
            min_location = numbers[i];
    printf("Minimum location: %ld\n", min_location);
    return 0;
}
