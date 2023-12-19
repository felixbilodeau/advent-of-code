#include <stdio.h>

#define MAX_LINES 20
#define MAX_COLS 20

#define LINE_MULTIPLIER 100
#define COL_MULTIPLIER 1

struct PatternSize {
    size_t num_lines;
    size_t num_cols;
};

size_t get_line(char line[], size_t max_line)
{
    size_t i;
    int c;

    for (
        i = 0;
        (c = getchar()) != EOF
        && c != '\n'
        && i < max_line;
        ++i
    )
        line[i] = c;
    return i;
}

struct PatternSize get_pattern(char pattern[MAX_LINES][MAX_COLS])
{
    size_t num_lines, num_cols, line_length;

    for (
        num_lines = num_cols = 0;
        (line_length = get_line(pattern[num_lines], MAX_COLS));
        ++num_lines
    ) if (!num_cols) num_cols = line_length;
    return (struct PatternSize) {
        .num_lines = num_lines, .num_cols = num_cols
    };
}

int check_two_lines(
    char pattern[MAX_LINES][MAX_COLS],
    struct PatternSize pattern_size,
    size_t line_1_index,
    size_t line_2_index
)
{
    size_t j;
    for (j = 0; j < pattern_size.num_cols; ++j)
        if (pattern[line_1_index][j] != pattern[line_2_index][j])
            return 0;
    return 1;
}

size_t check_line_reflections(
    char pattern[MAX_LINES][MAX_COLS], struct PatternSize pattern_size
)
{
    size_t reflections, current_line, offset;
    int found;
    reflections = 0;
    for (
        current_line = reflections = 0;
        current_line < pattern_size.num_lines - 1;
        ++current_line
    ) {
        found = 1;
        for (
            offset = 0;
            current_line + 2 - offset
            && current_line + offset < pattern_size.num_lines;
            ++offset
        ) {
            if (
                !check_two_lines(
                    pattern,
                    pattern_size,
                    current_line + 1 - offset,
                    current_line + offset
                )
            ) {
                found = 0;
                break;
            }
        }
        if (found) reflections = LINE_MULTIPLIER * (current_line + 1);
    }
    return reflections;
}

int check_two_columns(
    char pattern[MAX_LINES][MAX_COLS],
    struct PatternSize pattern_size,
    size_t col_1_index,
    size_t col_2_index
)
{
    size_t i;
    for (i = 0; i < pattern_size.num_lines; ++i)
        if (pattern[i][col_1_index] != pattern[i][col_2_index])
            return 0;
    return 1;
}

size_t check_col_reflections(
    char pattern[MAX_LINES][MAX_COLS], struct PatternSize pattern_size
)
{
    size_t reflections, current_col, offset;
    int found;
    reflections = 0;
    for (
        current_col = reflections = 0;
        current_col < pattern_size.num_cols - 1;
        ++current_col
    ) {
        found = 1;
        for (
            offset = 0;
            current_col + 2 - offset
            && current_col + offset < pattern_size.num_cols;
            ++offset
        ) {
            if (
                !check_two_columns(
                    pattern,
                    pattern_size,
                    current_col + 1 - offset,
                    current_col + offset
                )
            ) {
                found = 0;
                break;
            }
        }
        if (found) reflections = COL_MULTIPLIER * (current_col + 1);
    }
    return reflections;
}

int main(void)
{
    char pattern[MAX_LINES][MAX_COLS];
    struct PatternSize pattern_size;
    size_t total;
    
    total = 0;
    while((pattern_size = get_pattern(pattern)).num_lines)
        total = (
            total
            + check_line_reflections(pattern, pattern_size)
            + check_col_reflections(pattern, pattern_size)
        );
    printf("Total score  = %zu\n", total);
    return 0;
}
