#include <stdio.h>

#define MAX_LINE 128

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int is_number(char c)
{
    return is_digit(c) || c == '-';
}

int seek_next_number(char **p_line)
{
    while (**p_line && !is_number(**p_line))
        ++(*p_line);
    return **p_line != 0;
}

int parse_number(char **p_line)
{
    int number, multiplier, is_negative;
    char *base, *end;

    is_negative = 0;
    if (**p_line == '-') {
        is_negative = 1;
        ++(*p_line);
    }

    base = *p_line;
    while (**p_line && is_digit(**p_line))
        ++(*p_line);

    end = *p_line;
    if (!is_digit(**p_line))
        --(*p_line);

    number = 0;
    multiplier = 1;
    while (*p_line != base) {
        number = number + multiplier * (int)(**p_line - '0');
        multiplier = multiplier * 10;
        --(*p_line);
    }
    number = number + multiplier * (int)(**p_line - '0');
    if (is_negative)
        number = -1 * number;
    *p_line = end;
    return number;
}

int is_all_zeros(int nums[], size_t nums_size)
{
    size_t i;
    for (i = 0; i < nums_size; ++i)
        if (nums[i])
            return 0;
    return 1;
}

int get_prediction(int nums[], size_t nums_size)
{
    if (!nums_size || is_all_zeros(nums, nums_size)) return 0;
    int new_nums[nums_size - 1];
    size_t i;

    for (i = 1; i < nums_size; ++i)
        new_nums[i - 1] = nums[i] - nums[i - 1];

    return nums[nums_size - 1] + get_prediction(new_nums, nums_size - 1);
}

size_t get_line(char line[], size_t max_line)
{
    size_t i;
    int c;

    for (
        i = 0;
        (c = getchar()) != EOF
        && c != '\n'
        && i < max_line - 1;
        ++i
    )
        line[i] = c;
    line[i] = '\0';
    if (i == max_line - 1)
        puts("WARNING: Could not load entire line into buffer");
    return i;
}

int main(void)
{
    char line[MAX_LINE], *line_pointer;
    int nums[MAX_LINE], total;
    size_t nums_size;

    total = 0;
    while (get_line(line, MAX_LINE) != 0) {
        line_pointer = line;
        nums_size = 0;
        while (seek_next_number(&line_pointer)) {
            nums[nums_size] = parse_number(&line_pointer);
            ++nums_size;
        }
        total = total + get_prediction(nums, nums_size);
    }
    printf("Total = %d\n", total);

    return 0;
}
