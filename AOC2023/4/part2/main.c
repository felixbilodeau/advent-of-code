#include <stdio.h>

#define MAX_LINE 200
#define WINNING_NUMBERS 10
#define NUM_CARDS 204

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int is_winning(int number, int winning[WINNING_NUMBERS])
{
    size_t i;

    for (i = 0; i < WINNING_NUMBERS; ++i)
        if (number == winning[i])
            return 1;
    return 0;
}

size_t get_line(char line[], size_t max_line)
{
    int c;
    size_t i;

    for (
        i = 0;
        (c = getchar()) != EOF && c != '\n' && i < max_line - 1;
        ++i
    ) {
        line[i] = c;
    }

    line[i] = '\0';
    return i;
}

int seek_first_number(char **p_line)
{
    while (**p_line && **p_line != ':')
        ++(*p_line);
    
    while (**p_line && !is_digit(**p_line))
        ++(*p_line);

    if (!**p_line) {
        puts("ERROR: Could not seek first number!");
        return 0;
    }
    return 1;
}

int get_number(char **p_line)
{
    char *base, *end;
    int number, multiplier;

    base = *p_line;
    while (**p_line && is_digit(**p_line))
        ++(*p_line);

    end = *p_line;
    --(*p_line);

    number = 0;
    multiplier = 1;

    while (*p_line != base) {
        number = number + multiplier * (int)(**p_line - '0');
        multiplier = multiplier * 10;
        --(*p_line);
    }
    number = number + multiplier * (int)(**p_line - '0');
    *p_line = end;
    return number;
}

int seek_next_number(char **p_line)
{
    if (!**p_line) return 0;
    while (**p_line && !is_digit(**p_line) && **p_line != '|')
        ++(*p_line);
    
    if (**p_line == '|') {
        ++(*p_line);
        return -1;
    } else {
        return 1;
    }
}

int main(void)
{
    char line[MAX_LINE];
    int winning[WINNING_NUMBERS], cards[NUM_CARDS], matches, number, total;
    char *line_pointer;
    size_t i, card, start;

    for (i = 0; i < NUM_CARDS; ++i)
        cards[i] = 1;

    total = card = 0;
    while (get_line(line, MAX_LINE) != 0) {
        line_pointer = line;
        total = total + cards[card];
        if (!seek_first_number(&line_pointer)) return 1;
        number = get_number(&line_pointer);
        winning[0] = number;
        i = 1;
        while (seek_next_number(&line_pointer) != -1) {
            winning[i] = get_number(&line_pointer);
            ++i;
        }
        if (i != WINNING_NUMBERS) {
            puts("Failed to parse winning numbers!");
            return 1;
        }
        matches = 0;
        while (seek_next_number(&line_pointer)) {
            number = get_number(&line_pointer);
            if (is_winning(number, winning)) {
                ++matches;
            }
        }
        start = card + matches;
        if (start >= NUM_CARDS)
            start = NUM_CARDS - 1;
        for (i = start; i > card; --i)
            cards[i] = cards[i] + cards[card];
        ++card;
    }
    printf("total = %d\n", total);
    return 0;
}
