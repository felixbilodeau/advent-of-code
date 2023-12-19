#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 64
#define MAX_ROWS 1024

#define OPERATIONAL '.'
#define DAMAGED '#'
#define UNKNOWN '?'

#define CHECK_BIT(var, pos) (!!((var) & (1 << (pos))))

struct Row {
    char *springs;
    size_t *numbers;
    size_t numbers_length;
};

size_t power(size_t base, size_t exp)
{
    size_t result;

    result = 1;
    while (exp > 0) {
        result = result * base;
        --exp;
    }
    return result;
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
    if (i == max_line - 1)
        puts("WARNING: Could not load entire line into buffer");
    line[i] = '\0';
    return i;
}

int is_spring(char c)
{
    return c == OPERATIONAL || c == DAMAGED || c == UNKNOWN;
}

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

size_t get_numbers_length(char **p_line)
{
    size_t length;
    char *start;

    start = *p_line;
    length = 1;
    while (**p_line) {
        if (**p_line == ',') ++length;
        ++(*p_line);
    }
    *p_line = start;
    return length;
}

size_t parse_number(char **p_line)
{
    if (!**p_line) return 0;
    size_t number, multiplier;
    char *end, *start;
    start = *p_line;
    while (**p_line && is_digit(**p_line))
        ++(*p_line);
    end = *p_line;
    if (!is_digit(**p_line)) --(*p_line);
    number = 0;
    multiplier = 1;
    while (*p_line != start) {
        number = number + multiplier * (size_t)(**p_line - '0');
        multiplier = 10 * multiplier;
        --(*p_line);
    }
    number = number + multiplier * (size_t)(**p_line - '0');
    *p_line = end;
    return number;
}

struct Row parse_line(char line[])
{
    size_t i, *numbers, number, numbers_length;
    char *springs, *line_pointer;

    for (i = 0; line[i] && is_spring(line[i]); ++i);
    springs = malloc(++i * sizeof(*springs));
    memcpy(springs, line, i);
    springs[i - 1] = '\0';

    line_pointer = line + i;
    numbers_length = get_numbers_length(&line_pointer);
    numbers = malloc(numbers_length * sizeof(*numbers));
    i = 0;
    while (*line_pointer) {
        number = parse_number(&line_pointer);
        numbers[i] = number;
        if (*line_pointer == ',') ++line_pointer;
        ++i;
    }
    return (struct Row) {
        .springs = springs,
        .numbers = numbers,
        .numbers_length = numbers_length
    };
}

size_t parse_rows(struct Row rows[], size_t max_rows)
{
    char line[MAX_LINE];
    size_t num_rows;

    num_rows = 0;
    while (get_line(line, MAX_LINE) && num_rows < max_rows)
        rows[num_rows++] = parse_line(line);
    if (num_rows == max_rows)
        puts("WARNING: Could not load all rows into buffer");
    return num_rows;
}

size_t get_num_unknowns(struct Row row)
{
    size_t count;
    char *p_springs;

    count = 0;
    for (p_springs = row.springs; *p_springs; ++p_springs)
        if (*p_springs == UNKNOWN) ++count;
    return count;
}

size_t get_next_length(char **p_test)
{
    size_t count;

    while (**p_test && **p_test != DAMAGED) ++(*p_test);
    for (count = 0; **p_test && **p_test == DAMAGED; ++count) ++(*p_test);
    return count;
}

int is_possible(char *test, struct Row row)
{
    size_t next_length, i;
    char *p_test;

    p_test = test;
    for (
        i = next_length = 0;
        (next_length = get_next_length(&p_test))
        && i < row.numbers_length;
        ++i
    )
        if(next_length != row.numbers[i]) return 0;
    return !next_length && i == row.numbers_length;
}

int get_possibility(
    char *test, struct Row row, size_t num_unknowns, size_t seed
)
{
    if (seed >= power(2, num_unknowns)) return 0;
    size_t i;
    
    i = 0;
    while (num_unknowns > 0) {
        while (row.springs[i] && row.springs[i] != UNKNOWN) ++i;
        test[i] = CHECK_BIT(seed, num_unknowns - 1) ? DAMAGED : OPERATIONAL;
        ++i;
        --num_unknowns;
    }
    return 1;
}

size_t count_possibilities(struct Row row)
{
    size_t seed, num_unknowns, length, count;
    char *test;

    seed = 0;
    num_unknowns = get_num_unknowns(row);
    length = strlen(row.springs);
    test = malloc(length + 1);
    memcpy(test, row.springs, length + 1);
    for (
        seed = count = 0;
        get_possibility(test, row, num_unknowns, seed);
        ++seed
    )
        if (is_possible(test, row)) ++count;
    free(test);
    return count;
}

size_t count_all_possibilities(struct Row rows[], size_t num_rows)
{
    size_t count, i;

    for (i = count = 0; i < num_rows; ++i)
        count = count + count_possibilities(rows[i]);
    return count;
}

void print_rows(struct Row rows[], size_t num_rows)
{
    size_t i, j;
    for (i = 0; i < num_rows; ++i) {
        printf("Row %zu: %s ", i + 1, rows[i].springs);
        if (!rows[i].numbers_length) {
            printf("[]");
        } else {
            putchar('[');
            for (j = 0; j < rows[i].numbers_length - 1; ++j)
                printf("%zu, ", rows[i].numbers[j]);
            printf("%zu]", rows[i].numbers[rows[i].numbers_length - 1]);
        }
        printf(" -> %zu unknowns", get_num_unknowns(rows[i]));
        putchar('\n');
    }
}

void free_rows(struct Row rows[], size_t num_rows)
{
    size_t i;
    for (i = 0; i < num_rows; ++i) {
        free(rows[i].springs);
        free(rows[i].numbers);
        rows[i].springs = NULL;
        rows[i].numbers = NULL;
        rows[i].numbers_length = 0;
    }
}

int main(void)
{
    struct Row rows[MAX_ROWS];
    size_t num_rows;

    num_rows = parse_rows(rows, MAX_ROWS);
    print_rows(rows, num_rows);
    printf("Total = %zu\n", count_all_possibilities(rows, num_rows));
    free_rows(rows, num_rows);
    return 0;
}
