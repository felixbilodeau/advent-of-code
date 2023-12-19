#include <stdio.h>

#define MAX_ROW 140
#define MAX_LINE MAX_ROW

int is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

void load_file(char file[MAX_ROW][MAX_LINE])
{
    size_t i, j;

    for (i = 0; i < MAX_ROW; ++i) {
        for (j = 0; j < MAX_LINE; ++j)
            file[i][j] = getchar();
        getchar();
    }
}

size_t convert_number(
    char file[MAX_ROW][MAX_LINE],
    size_t row,
    size_t line_start,
    size_t number_length
)
{
    size_t number, multiplier;
    int i;

    number = 0;
    multiplier = 1;
    for (i = line_start + number_length - 1; i >= (int)line_start; --i) {
        number = number + multiplier * (size_t)(
            file[row][i] - '0'
        );
        multiplier = multiplier * 10;
    }
    return number;
}

size_t get_number(
    char file[MAX_ROW][MAX_LINE],
    size_t row, 
    size_t line_start
)
{
    size_t number_length;

    number_length = 0;
    while (
        line_start + number_length < MAX_LINE
        && is_digit(file[row][line_start + number_length])
    )
        ++number_length;

    return convert_number(file, row, line_start, number_length);
}

size_t check_gear(char file[MAX_ROW][MAX_LINE], size_t row, size_t line_start)
{
    size_t num1, num2, number, start;

    num1 = num2 = number = 0;
    if (row) {
        if (line_start && is_digit(file[row - 1][line_start - 1])) {
            start = line_start - 1;
            while (start && is_digit(file[row - 1][start])) {
                --start;
            }
            if (!is_digit(file[row - 1][start]))
                ++start;

            number = get_number(file, row - 1, start);
            if (!num1)
                num1 = number;
            else if (!num2)
                num2 = number;
            else
                return 0;
        }
        if (is_digit(file[row - 1][line_start])) {
            if (!(line_start && is_digit(file[row - 1][line_start - 1]))) {
                number = get_number(file, row - 1, line_start);
                if (!num1)
                    num1 = number;
                else if (!num2)
                    num2 = number;
                else
                    return 0;
            }
        }
        if (line_start < MAX_LINE - 1 && is_digit(file[row - 1][line_start + 1])) {
            if (!is_digit(file[row - 1][line_start])) {
                number = get_number(file, row - 1, line_start + 1);
                if (!num1)
                    num1 = number;
                else if (!num2)
                    num2 = number;
                else
                    return 0;
            }
        }
    }
    if (row < MAX_ROW - 1) {
        if (line_start && is_digit(file[row + 1][line_start - 1])) {
            start = line_start - 1;
            while (start && is_digit(file[row + 1][start])) {
                --start;
            }
            if (!is_digit(file[row + 1][start]))
                ++start;

            number = get_number(file, row + 1, start);
            if (!num1)
                num1 = number;
            else if (!num2)
                num2 = number;
            else
                return 0;
        }
        if (is_digit(file[row + 1][line_start])) {
            if (!(line_start && is_digit(file[row + 1][line_start - 1]))) {
                number = get_number(file, row + 1, line_start);
                if (!num1)
                    num1 = number;
                else if (!num2)
                    num2 = number;
                else
                    return 0;
            }
        }
        if (line_start < MAX_LINE - 1 && is_digit(file[row + 1][line_start + 1])) {
            if (!is_digit(file[row + 1][line_start])) {
                number = get_number(file, row + 1, line_start + 1);
                if (!num1)
                    num1 = number;
                else if (!num2)
                    num2 = number;
                else
                    return 0;
            }
        }
    }
    if (line_start) {
        if (is_digit(file[row][line_start - 1])) {
            start = line_start - 1;
            while (start && is_digit(file[row][start])) {
                --start;
            }
            if (!is_digit(file[row][start]))
                ++start;

            number = get_number(file, row, start);
            if (!num1)
                num1 = number;
            else if (!num2)
                num2 = number;
            else
                return 0;
        }
    }
    if (line_start < MAX_LINE - 1) {
        if (is_digit(file[row][line_start + 1])) {
            number = get_number(file, row, line_start + 1);
            if (!num1)
                num1 = number;
            else if (!num2)
                num2 = number;
            else
                return 0;
        }
    }
    return num1 * num2;
}

int main(void)
{
    char file[MAX_ROW][MAX_LINE];
    size_t total, i, j;
    
    load_file(file);

    total = 0;

    for (i = 0; i < MAX_ROW; ++i) {
        for (j = 0; j < MAX_LINE; ++j) {
            if (file[i][j] == '*')
                total = total + check_gear(file, i, j);
        }
    }

    printf("Total = %zu\n", total);
    return 0;
}
