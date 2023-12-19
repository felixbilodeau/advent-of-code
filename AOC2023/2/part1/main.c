#include <stdio.h>

#define MAX_LINE 200

#define RED "red"
#define GREEN "green"
#define BLUE "blue"

#define MAX_RED 12
#define MAX_GREEN 13
#define MAX_BLUE 14

int is_digit(char c)
{
    return '0' <= c && c <= '9';
}

size_t get_number(char **p_line)
{
    size_t number, multiplier;
    char *base, *end;

    for (base = *p_line; is_digit(**p_line); ++(*p_line));
    end = *p_line;
    --(*p_line);

    multiplier = 1;
    number = 0;
    while (*p_line != base) {
        number = number + multiplier * (size_t)(**p_line - '0');
        multiplier = multiplier * 10;
        --(*p_line);
    }
    number = number + multiplier * (size_t)(**p_line - '0');
    *p_line = end;
    return number;
}

int is_substring(char **p_line, const char* substring)
{
    char *base;
    base = *p_line;
    while (**p_line && *substring && **p_line == *substring) {
        ++(*p_line);
        ++substring;
    }
    if (!*substring) return 1;
    *p_line = base;
    return 0;
}

int first_game_pred(char c)
{
    return c == ':';
}

int next_game_pred(char c)
{
    return c == ';';
}

int seek_predicate(char **p_line, int (*pred)(char))
{
    while (**p_line && !pred(**p_line)) ++(*p_line);
    return !!**p_line;
}

int seek_first_game(char **p_line)
{
    if (seek_predicate(p_line, first_game_pred))
        ++(*p_line);
    return !!**p_line;
}

int seek_next_game(char **p_line)
{
    if (seek_predicate(p_line, next_game_pred))
        ++(*p_line);
    return !!**p_line;
}

int parse_game(char **p_line)
{
    size_t number;
    number = 0;
    while (**p_line && **p_line != ';') {
        if (is_digit(**p_line)) {
            number = get_number(p_line);
            ++(*p_line);
        } else if (is_substring(p_line, RED)) {
            if (number > MAX_RED) return 0;
            number = 0;
        } else if (is_substring(p_line, GREEN)) {
            if (number > MAX_GREEN) return 0;
            number = 0;
        } else if (is_substring(p_line, BLUE)) {
            if (number > MAX_BLUE) return 0;
            number = 0;
        } else {
            ++(*p_line);
        }
    }
    return 1;
}

size_t get_line(char line[], size_t max_line)
{
    int c;
    size_t length;
    for (
        length = 0;
        (c = getchar()) != EOF && c != '\n' && length < max_line - 1;
        ++length
    )
        line[length] = c;
    line[length] = '\0';
    return length;
}

int main(void)
{
    char line[MAX_LINE], *lineptr;
    size_t game_id, total;
    int possible;

    game_id = total = 0;
    while (get_line(line, MAX_LINE)) {
        lineptr = line;
        possible = 1;
        ++game_id;
        if (!seek_first_game(&lineptr)) continue;
        while (possible && *lineptr) {
            possible = parse_game(&lineptr);
            if (possible)
                seek_next_game(&lineptr);
        }
        if (possible)
            total = total + game_id;
    }
    printf("total = %zu\n", total);
    return 0;
}
