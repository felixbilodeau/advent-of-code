#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 64
#define MAX_ROWS 1024

#define DUPLICATIONS 5
#define MAX_BUFFER_SIZE 1024
#define MAX_CACHE 1024 * 1024 * 16

#define OPERATIONAL '.'
#define DAMAGED '#'
#define UNKNOWN '?'

size_t collisions = 0;

struct Row {
    char *springs;
    size_t *numbers;
    size_t numbers_length;
};

struct CountCache {
    size_t result;
    char *arg_string;
};

size_t hash(char *word, size_t array_size)
{
    size_t hash = 5381;
    char c;
    while ((c = *word++))
        hash = ((hash << 5) + hash) + (size_t)c;
    return hash % array_size;
}

size_t min(size_t number, size_t other)
{
    return number < other ? number : other;
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
    size_t i, j, *numbers, number, numbers_length, springs_len, mem_size;
    char *springs, *line_pointer;

    for (i = 0; line[i] && is_spring(line[i]); ++i);
    springs_len = i;
    mem_size = (springs_len + 1) * DUPLICATIONS * sizeof(*springs);
    springs = malloc(mem_size);
    memset(springs, UNKNOWN, mem_size);
    for (i = 0; i < DUPLICATIONS; ++i)
        memcpy(springs + i * (springs_len + 1), line, springs_len);
    springs[mem_size - 1] = '\0';

    line_pointer = line + springs_len + 1;
    numbers_length = get_numbers_length(&line_pointer);
    numbers = malloc(numbers_length * DUPLICATIONS * sizeof(*numbers));
    for (j = 0; j < DUPLICATIONS; ++j) {
        i = 0;
        line_pointer = line + springs_len + 1;
        while (*line_pointer) {
            number = parse_number(&line_pointer);
            numbers[j * numbers_length + i] = number;
            if (*line_pointer == ',') ++line_pointer;
            ++i;
        }
    }
    return (struct Row) {
        .springs = springs,
        .numbers = numbers,
        .numbers_length = numbers_length * DUPLICATIONS
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

int check_next_n_spots(char *line, size_t n)
{
    while (*line && n) {
        if (*line == OPERATIONAL) return 0;
        ++line;
        --n;
    }
    return !n;
}

int prepare_arg_string(
    char **p_line,
    size_t line_length,
    size_t *numbers,
    size_t numbers_length,
    char *arg_string_buffer,
    size_t max_buffer_size
)
{
    static const size_t max_len = 64;
    char buf[max_len];
    strncpy(arg_string_buffer, *p_line, max_buffer_size);
    while (numbers_length) {
        sprintf(buf, "%zu,", numbers[numbers_length - 1]);
        strncat(
            arg_string_buffer,
            buf,
            min(max_buffer_size - line_length, max_len)
        );
        --numbers_length;
    }
    return 1;
}

int cmp_str(char *str, char *other)
{
    while (*str && *other) if (*(str++) != *(other++)) return 0;
    return !*str && !*other;
}

int search_cache(
    struct CountCache *cache,
    size_t max_cache,
    char *arg_string,
    size_t *result
)
{
    size_t start_index, i;

    start_index = hash(arg_string, max_cache);
    if (!(cache[start_index].arg_string)) return 0;
    for (i = start_index; i < max_cache; ++i) {
        if (!(cache[i].arg_string)) return 0;
        if (cmp_str(cache[i].arg_string, arg_string)) {
            *result = cache[i].result;
            return 1;
        }
    }
    for (i = 0; i < start_index; ++i) {
        if (!(cache[i].arg_string)) return 0;
        if (cmp_str(cache[i].arg_string, arg_string)) {
            *result = cache[i].result;
            return 1;
        }
    }
    *result = 0;
    return 0;
}

int store_cache(
    struct CountCache *cache,
    size_t max_cache,
    char *arg_string,
    size_t result
)
{
    size_t start_index, i, arg_string_len;
    char *arg_string_cpy;

    start_index = hash(arg_string, max_cache);
    arg_string_len = strlen(arg_string);
    arg_string_cpy = malloc(arg_string_len + 1);
    memcpy(arg_string_cpy, arg_string, arg_string_len + 1);
    if (!cache[start_index].arg_string) {
        cache[start_index] = (struct CountCache) {
            .result = result,
            .arg_string = arg_string_cpy
        };
        return 1;
    }
    ++collisions;
    for (i = start_index; i < max_cache; ++i) {
        if (!(cache[i].arg_string)) {
            cache[i] = (struct CountCache) {
                .result = result,
                .arg_string = arg_string_cpy
            };
            return 1;
        }
    }
    for (i = 0; i < start_index; ++i) {
        if (!(cache[i].arg_string)) {
            cache[i] = (struct CountCache) {
                .result = result,
                .arg_string = arg_string_cpy
            };
            return 1;
        }
    }
    puts("WARNING: Ran out of space in the cache");
    return 0;
}

size_t count(
    char **p_line,
    size_t line_length,
    size_t *numbers,
    size_t numbers_length,
    char *arg_string_buffer,
    size_t max_buffer_size,
    struct CountCache *cache,
    size_t max_cache
)
{
    size_t result;
    char *base;
    if (!**p_line) return !numbers_length;
    if (!numbers_length) {
        while (**p_line) {
            if (**p_line == DAMAGED) return 0;
            ++(*p_line);
        }
        return 1;
    }

    prepare_arg_string(
        p_line,
        line_length,
        numbers,
        numbers_length,
        arg_string_buffer,
        max_buffer_size
    );
    if (search_cache(cache, max_cache, arg_string_buffer, &result)) {
        return result;
    }

    result = 0;
    base = *p_line;
    if (**p_line == OPERATIONAL || **p_line == UNKNOWN) {
        ++(*p_line);
        result = result + count(
            p_line,
            line_length - 1,
            numbers,
            numbers_length,
            arg_string_buffer,
            max_buffer_size,
            cache,
            max_cache
        );
        *p_line = base;
    }
    if (**p_line == DAMAGED || **p_line == UNKNOWN) {
        if (
            *numbers <= line_length
            && check_next_n_spots(*p_line, *numbers)
            && (
                *numbers == line_length
                || *(*p_line + *numbers) != DAMAGED
            )
        ) {
            *p_line = (
                *(*p_line + *numbers)
                ? *p_line + *numbers + 1
                : *p_line + *numbers
            );
            result = result + count(
                p_line,
                line_length - *numbers > 0
                ? line_length - *numbers - 1
                : line_length - *numbers,
                numbers + 1,
                numbers_length - 1,
                arg_string_buffer,
                max_buffer_size,
                cache,
                max_cache
            );
            *p_line = base;
        }
    }
    prepare_arg_string(
        p_line,
        line_length,
        numbers,
        numbers_length,
        arg_string_buffer,
        max_buffer_size
    );
    if (*arg_string_buffer) {
        store_cache(cache, max_cache, arg_string_buffer, result);
    }
    return result;
}

size_t count_row(
    struct Row row,
    char *arg_string_buffer,
    size_t max_buffer_size,
    struct CountCache *cache,
    size_t max_cache
)
{
    char *p_springs;

    p_springs = row.springs;
    return count(
        &p_springs,
        strlen(p_springs),
        row.numbers,
        row.numbers_length,
        arg_string_buffer,
        max_buffer_size,
        cache,
        max_cache
    );
}

size_t count_all_rows(
    struct Row rows[],
    size_t num_rows,
    char *arg_string_buffer,
    size_t max_buffer_size,
    struct CountCache *cache,
    size_t max_cache
)
{
    size_t total;

    total = 0;
    while (num_rows) {
        total = total + count_row(
            rows[num_rows - 1],
            arg_string_buffer,
            max_buffer_size,
            cache,
            max_cache
        );
        --num_rows;
    }
    return total;
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
    char arg_string_buffer[MAX_BUFFER_SIZE];
    struct CountCache *cache;
    struct Row rows[MAX_ROWS];
    size_t num_rows;

    cache = malloc(MAX_CACHE * sizeof(struct CountCache));
    if (!cache) {
        puts("Failed to allocate cache");
        return 1;
    }
    for (num_rows = 0; num_rows < MAX_CACHE; ++num_rows)
        cache[num_rows] = (struct CountCache) {
            .result = 0,
            .arg_string = NULL
        };
    num_rows = parse_rows(rows, MAX_ROWS);
    printf(
        "Total = %zu\n",
        count_all_rows(
            rows,
            num_rows,
            arg_string_buffer,
            MAX_BUFFER_SIZE,
            cache,
            MAX_CACHE
        )
    );
    free_rows(rows, num_rows);
    for (num_rows = 0; num_rows < MAX_CACHE; ++num_rows)
        if (cache[num_rows].arg_string)
            free(cache[num_rows].arg_string);
    free(cache);
    printf("Cache collisions: %zu\n", collisions);
    return 0;
}
