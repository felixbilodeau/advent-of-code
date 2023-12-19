#include <stdio.h>

#define MAX_ROW 140
#define MAX_LINE MAX_ROW

void load_file(char file[MAX_ROW][MAX_LINE])
{
    size_t i, j;

    for (i = 0; i < MAX_ROW; ++i) {
        for (j = 0; j < MAX_LINE; ++j)
            file[i][j] = getchar();
        getchar();
    }
}

int check_row(
    char file[MAX_ROW][MAX_LINE],
    size_t row,
    size_t line_start,
    size_t number_length
)
{
    size_t i;

    if (!line_start)
        i = 0;
    else
        i = line_start - 1;
    
    while (i < MAX_LINE && i <= line_start + number_length) {
        if (file[row][i] != '.')
            return 1;
        ++i;
    }
    return 0;
}

int check_for_symbols(
    char file[MAX_ROW][MAX_LINE],
    size_t row,
    size_t line_start,
    size_t number_length
)
{
    int result;

    result = 0;
    if (line_start) {
        result = result || file[row][line_start - 1] != '.';
        if (result)
            return result;
    }
    if (line_start + number_length < MAX_LINE) {
        result = result || file[row][line_start + number_length] != '.';
        if (result)
            return result;
    }
    if (row) {
        result = result || check_row(
            file, row - 1, line_start, number_length
        );
        if (result)
            return result;
    }
    if (row < MAX_ROW - 1) {
        result = result || check_row(
            file, row + 1, line_start, number_length
        );
        if (result)
            return result;
    }
    return result;
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
    size_t line_start,
    size_t *number
)
{
    size_t number_length;

    number_length = 0;
    while (
        line_start + number_length < MAX_LINE
        && file[row][line_start + number_length] >= '0'
        && file[row][line_start + number_length] <= '9'
    )
        ++number_length;

    if (check_for_symbols(file, row, line_start, number_length)) {
        *number = convert_number(file, row, line_start, number_length);
    } else {
        *number = 0;
    }
    return number_length;
}

int main(void)
{
    char file[MAX_ROW][MAX_LINE];
    size_t total, i, j, number;
    
    load_file(file);

    i = j = total = number = 0;

    while (i < MAX_ROW) {
        j = 0;
        while (j < MAX_LINE) {
            if (file[i][j] >= '0' && file[i][j] <= '9') {
                j = j + get_number(file, i, j, &number);
                total = total + number;
            } else {
                ++j;
            }
        }
        ++i;
    }

    printf("Total = %zu\n", total);
    return 0;
}
