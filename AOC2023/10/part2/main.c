#include <stdio.h>
#include <string.h>

#define NUM_LINES 140UL
#define NUM_COLS 140UL

#define NORTH 0
#define SOUTH 1
#define WEST 2
#define EAST 3

#define NONE 'X'
#define LEFT '<'
#define RIGHT '>'

size_t get_index_of(size_t line, size_t col)
{
    return line * NUM_COLS + col;
}

int check_north(char file[], size_t line, size_t col)
{
    if (!line) return 0;
    size_t index;
    index = get_index_of(--line, col);
    return file[index] == '|' || file[index] == '7' || file[index] == 'F';
}

int check_south(char file[], size_t line, size_t col)
{
    if (line == NUM_LINES - 1) return 0;
    size_t index;
    index = get_index_of(++line, col);
    return file[index] == '|' || file[index] == 'L' || file[index] == 'J';
}

int check_west(char file[], size_t line, size_t col)
{
    if (!col) return 0;
    size_t index;
    index = get_index_of(line, --col);
    return file[index] == '-' || file[index] == 'L' || file[index] == 'F';
}

int check_east(char file[], size_t line, size_t col)
{
    if (col == NUM_COLS - 1) return 0;
    size_t index;
    index = get_index_of(line, ++col);
    return file[index] == '-' || file[index] == 'J' || file[index] == '7';
}

int main(void)
{
    char file[NUM_LINES * NUM_COLS];
    char loop[NUM_LINES * NUM_COLS];
    char outside, inside;
    size_t i, j, start_col, start_line, index, total;
    int last_move, path_start, c, changes;

    memset(loop, NONE, sizeof(loop));

    for (
        i = 0;
        (c = getchar()) != EOF
        && i < NUM_LINES * NUM_COLS;
        ++i
    ) {
        if (c == '\n') {
            --i;
            continue;   
        }
        file[i] = c;
    }
    c = 0;
    start_line = start_col = 0;
    for (start_line = 0; start_line < NUM_LINES && !c; ++start_line)
        for (start_col = 0; start_col < NUM_COLS && !c; ++start_col)
            c = file[get_index_of(start_line, start_col)] == 'S';
    --start_col; --start_line;
    path_start = last_move = -1;
    index = get_index_of(start_line, start_col);
    if (check_north(file, start_line, start_col)) {
        path_start = NORTH;
        if (check_south(file, start_line, start_col))
            file[index] = '|';
        else if (check_west(file, start_line, start_col))
            file[index] = 'J';
        else if (check_east(file, start_line, start_col))
            file[index] = 'L';
    } else if (check_south(file, start_line, start_col)) {
        path_start = SOUTH;
        if (check_north(file, start_line, start_col))
            file[index] = '|';
        else if (check_west(file, start_line, start_col))
            file[index] = '7';
        else if (check_east(file, start_line, start_col))
            file[index] = 'F';
    } else if (check_west(file, start_line, start_col)) {
        path_start = WEST;
        if (check_north(file, start_line, start_col))
            file[index] = 'J';
        else if (check_south(file, start_line, start_col))
            file[index] = '7';
        else if (check_east(file, start_line, start_col))
            file[index] = '-';
    } else if (check_east(file, start_line, start_col)) {
        path_start = EAST;
        if (check_north(file, start_line, start_col))
            file[index] = 'L';
        else if (check_south(file, start_line, start_col))
            file[index] = 'F';
        else if (check_west(file, start_line, start_col))
            file[index] = '-';
    } else {
        puts("ERROR: Did not find any path starts!");
    }

    i = start_line;
    j = start_col;
    switch (path_start) {
    case NORTH:
        --i;
        last_move = NORTH;
        break;
    case SOUTH:
        ++i;
        last_move = SOUTH;
        break;
    case WEST:
        --j;
        last_move = WEST;
        break;
    case EAST:
        ++j;
        last_move = EAST;
        break;
    default:
        puts("ERROR: Could not determine start position");
        break;
    }

    loop[index] = file[index];
    while (!(i == start_line && j == start_col)) {
        index = get_index_of(i, j);
        loop[index] = file[index];
        switch (file[index]) {
        case '|':
            switch (last_move) {
            case NORTH:
                --i;
                last_move = NORTH;
                break;
            case SOUTH:
                ++i;
                last_move = SOUTH;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case '-':
            switch (last_move) {
            case WEST:
                --j;
                last_move = WEST;
                break;
            case EAST:
                ++j;
                last_move = EAST;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case 'L':
            switch (last_move) {
            case SOUTH:
                ++j;
                last_move = EAST;
                break;
            case WEST:
                --i;
                last_move = NORTH;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case 'J':
            switch (last_move) {
            case EAST:
                --i;
                last_move = NORTH;
                break;
            case SOUTH:
                --j;
                last_move = WEST;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case '7':
            switch (last_move) {
            case EAST:
                ++i;
                last_move = SOUTH;
                break;
            case NORTH:
                --j;
                last_move = WEST;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case 'F':
            switch (last_move) {
            case WEST:
                ++i;
                last_move = SOUTH;
                break;
            case NORTH:
                ++j;
                last_move = EAST;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        default:
            puts("ERROR: Invalid move");
            break;
        }
    }

    i = start_line;
    j = start_col;
    switch (path_start) {
    case NORTH:
        --i;
        last_move = NORTH;
        break;
    case SOUTH:
        ++i;
        last_move = SOUTH;
        break;
    case WEST:
        --j;
        last_move = WEST;
        break;
    case EAST:
        ++j;
        last_move = EAST;
        break;
    default:
        puts("ERROR: Could not determine start position");
        break;
    }

    while (!(i == start_line && j == start_col)) {
        index = get_index_of(i, j);
        switch (loop[index]) {
        case '|':
            switch (last_move) {
            case NORTH:
                if (j && loop[get_index_of(i, j - 1)] == NONE)
                    loop[get_index_of(i, j - 1)] = LEFT;
                if (
                    j < NUM_COLS - 1
                    && loop[get_index_of(i, j + 1)] == NONE
                )
                    loop[get_index_of(i, j + 1)] = RIGHT;
                --i;
                last_move = NORTH;
                break;
            case SOUTH:
                if (j && loop[get_index_of(i, j - 1)] == NONE)
                    loop[get_index_of(i, j - 1)] = RIGHT;
                if (
                    j < NUM_COLS - 1
                    && loop[get_index_of(i, j + 1)] == NONE
                )
                    loop[get_index_of(i, j + 1)] = LEFT;
                ++i;
                last_move = SOUTH;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case '-':
            switch (last_move) {
            case WEST:
                if (i && loop[get_index_of(i - 1, j)] == NONE)
                    loop[get_index_of(i - 1, j)] = RIGHT;
                if (
                    i < NUM_LINES - 1
                    && loop[get_index_of(i + 1, j)] == NONE
                )
                    loop[get_index_of(i + 1, j)] = LEFT;
                --j;
                last_move = WEST;
                break;
            case EAST:
                if (i && loop[get_index_of(i - 1, j)] == NONE)
                    loop[get_index_of(i - 1, j)] = LEFT;
                if (
                    i < NUM_LINES - 1
                    && loop[get_index_of(i + 1, j)] == NONE
                )
                    loop[get_index_of(i + 1, j)] = RIGHT;
                ++j;
                last_move = EAST;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case 'L':
            switch (last_move) {
            case SOUTH:
                if (j) {
                    if (loop[get_index_of(i, j - 1)] == NONE)
                        loop[get_index_of(i, j - 1)] = RIGHT;
                    if (
                        i < NUM_LINES - 1
                        && loop[get_index_of(i + 1, j - 1)] == NONE
                    )
                        loop[get_index_of(i + 1, j - 1)] = RIGHT;
                }
                if (i < NUM_LINES - 1 && loop[get_index_of(i + 1, j)] == NONE)
                    loop[get_index_of(i + 1, j)] = RIGHT;
                if (
                    i
                    && j < NUM_COLS - 1
                    && loop[get_index_of(i - 1, j + 1)] == NONE
                )
                    loop[get_index_of(i - 1, j + 1)] = LEFT;
                ++j;
                last_move = EAST;
                break;
            case WEST:
                if (j) {
                    if (loop[get_index_of(i, j - 1)] == NONE)
                        loop[get_index_of(i, j - 1)] = LEFT;
                    if (
                        i < NUM_LINES - 1
                        && loop[get_index_of(i + 1, j - 1)] == NONE
                    )
                        loop[get_index_of(i + 1, j - 1)] = LEFT;
                }
                if (i < NUM_LINES - 1 && loop[get_index_of(i + 1, j)] == NONE)
                    loop[get_index_of(i + 1, j)] = LEFT;
                if (
                    i
                    && j < NUM_COLS - 1
                    && loop[get_index_of(i - 1, j + 1)] == NONE
                )
                    loop[get_index_of(i - 1, j + 1)] = RIGHT;
                --i;
                last_move = NORTH;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case 'J':
            switch (last_move) {
            case EAST:
                if (i < NUM_LINES - 1) {
                    if (loop[get_index_of(i + 1, j)] == NONE)
                        loop[get_index_of(i + 1, j)] = RIGHT;
                    if (
                        j < NUM_COLS - 1
                        && loop[get_index_of(i + 1, j + 1)] == NONE
                    )
                        loop[get_index_of(i + 1, j + 1)] = RIGHT;
                }
                if (j < NUM_COLS - 1 && loop[get_index_of(i, j + 1)] == NONE)
                    loop[get_index_of(i, j + 1)] = RIGHT;
                if (i && j && loop[get_index_of(i - 1, j - 1)] == NONE)
                    loop[get_index_of(i - 1, j - 1)] = LEFT;
                --i;
                last_move = NORTH;
                break;
            case SOUTH:
                if (i < NUM_LINES - 1) {
                    if (loop[get_index_of(i + 1, j)] == NONE)
                        loop[get_index_of(i + 1, j)] = LEFT;
                    if (
                        j < NUM_COLS - 1
                        && loop[get_index_of(i + 1, j + 1)] == NONE
                    )
                        loop[get_index_of(i + 1, j + 1)] = LEFT;
                }
                if (j < NUM_COLS - 1 && loop[get_index_of(i, j + 1)] == NONE)
                    loop[get_index_of(i, j + 1)] = LEFT;
                if (i && j && loop[get_index_of(i - 1, j - 1)] == NONE)
                    loop[get_index_of(i - 1, j - 1)] = RIGHT;
                --j;
                last_move = WEST;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case '7':
            switch (last_move) {
            case EAST:
                if (i) {
                    if (loop[get_index_of(i - 1, j)] == NONE)
                        loop[get_index_of(i - 1, j)] = LEFT;
                    if (
                        j < NUM_COLS - 1
                        && loop[get_index_of(i - 1, j + 1)] == NONE
                    )
                        loop[get_index_of(i - 1, j + 1)] = LEFT;
                }
                if (j < NUM_COLS - 1 && loop[get_index_of(i, j + 1)] == NONE)
                    loop[get_index_of(i, j + 1)] = LEFT;
                if (
                    i < NUM_LINES - 1
                    && j
                    && loop[get_index_of(i + 1, j - 1)] == NONE
                )
                    loop[get_index_of(i + 1, j - 1)] = RIGHT;
                ++i;
                last_move = SOUTH;
                break;
            case NORTH:
                if (i) {
                    if (loop[get_index_of(i - 1, j)] == NONE)
                        loop[get_index_of(i - 1, j)] = RIGHT;
                    if (
                        j < NUM_COLS - 1
                        && loop[get_index_of(i - 1, j + 1)] == NONE
                    )
                        loop[get_index_of(i - 1, j + 1)] = RIGHT;
                }
                if (j < NUM_COLS - 1 && loop[get_index_of(i, j + 1)] == NONE)
                    loop[get_index_of(i, j + 1)] = RIGHT;
                if (
                    i < NUM_LINES - 1
                    && j
                    && loop[get_index_of(i + 1, j - 1)] == NONE
                )
                    loop[get_index_of(i - 1, j - 1)] = LEFT;
                --j;
                last_move = WEST;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        case 'F':
            switch (last_move) {
            case WEST:
                if (i) {
                    if (loop[get_index_of(i - 1, j)] == NONE)
                        loop[get_index_of(i - 1, j)] = RIGHT;
                    if (j && loop[get_index_of(i - 1, j - 1)] == NONE)
                        loop[get_index_of(i - 1, j - 1)] = RIGHT;
                }
                if (j && loop[get_index_of(i, j - 1)] == NONE)
                    loop[get_index_of(i, j - 1)] = RIGHT;
                if (
                    i < NUM_LINES - 1
                    && j < NUM_COLS - 1
                    && loop[get_index_of(i + 1, j + 1)] == NONE
                )
                    loop[get_index_of(i + 1, j + 1)] = LEFT;
                ++i;
                last_move = SOUTH;
                break;
            case NORTH:
                if (i) {
                    if (loop[get_index_of(i - 1, j)] == NONE)
                        loop[get_index_of(i - 1, j)] = LEFT;
                    if (j && loop[get_index_of(i - 1, j - 1)] == NONE)
                        loop[get_index_of(i - 1, j - 1)] = LEFT;
                }
                if (j && loop[get_index_of(i, j - 1)] == NONE)
                    loop[get_index_of(i, j - 1)] = LEFT;
                if (
                    i < NUM_LINES - 1
                    && j < NUM_COLS - 1
                    && loop[get_index_of(i + 1, j + 1)] == NONE
                )
                    loop[get_index_of(i + 1, j + 1)] = RIGHT;
                ++j;
                last_move = EAST;
                break;
            default:
                puts("Invalid move!");
                i = start_line;
                j = start_col;
                break;
            }
            break;
        default:
            puts("ERROR: Invalid move");
            break;
        }
    }

    outside = inside = NONE;
    i = 0;
    while (loop[get_index_of(i, 0)] != NONE)
        ++i;
    if (loop[get_index_of(i, 0)] == NONE) {
        j = 0;
        while (loop[get_index_of(i, j)] == NONE)
            ++j;
        outside = loop[get_index_of(i, j)];
        if (outside == RIGHT)
            inside = LEFT;
        else
            inside = RIGHT;
    } else {
        puts("ERROR: Could not deterine in and out chars");
    }

    for (i = 0; i < NUM_LINES; ++i) {
        j = 0;
        while (j < NUM_COLS && loop[get_index_of(i, j)] == NONE) {
            loop[get_index_of(i, j)] = outside;
            ++j;
        }
        j = NUM_COLS - 1;
        while (loop[get_index_of(i, j)] == NONE) {
            loop[get_index_of(i, j)] = outside;
            --j;
        }
    }

    for (j = 0; j < NUM_COLS; ++j) {
        i = 0;
        while (
            i < NUM_LINES
            && (
                loop[get_index_of(i, j)] == NONE
                || loop[get_index_of(i, j)] == outside
            )
        ) {
            loop[get_index_of(i, j)] = outside;
            ++i;
        }
        i = NUM_LINES - 1;
        while (
            loop[get_index_of(i, j)] == NONE
            || loop[get_index_of(i, j)] == outside
        ) {
            loop[get_index_of(i, j)] = outside;
            --i;
        }
    }

    changes = 1;
    while (changes) {
        changes = 0;
        for (i = 0; i < NUM_LINES; ++i) {
            for (j = 0; j < NUM_COLS; ++j) {
                index = get_index_of(i, j);
                if (loop[index] == NONE) {
                    if (
                        (i && loop[get_index_of(i - 1, j)] == outside)
                        || (
                            i < NUM_LINES - 1
                            && loop[get_index_of(i + 1, j)] == outside
                        ) || (j && loop[get_index_of(i, j - 1)] == outside)
                        || (
                            j < NUM_COLS - 1
                            && loop[get_index_of(i, j + 1)] == outside
                        ) || (
                            i && j && loop[get_index_of(i - 1, j - 1)] == outside
                        ) || (
                            i
                            && j < NUM_COLS - 1
                            && loop[get_index_of(i - 1, j + 1)] == outside
                        ) || (
                            i < NUM_LINES - 1
                            && j
                            && loop[get_index_of(i + 1, j - 1)] == outside
                        ) || (
                            i < NUM_LINES - 1
                            && j < NUM_COLS - 1
                            && loop[get_index_of(i + 1, j + 1)] == outside
                        )
                    ) {
                        loop[index] = outside;
                        changes = 1;
                    }
                }
            }
        }
    }

    changes = 1;
    while (changes) {
        changes = 0;
        for (i = 0; i < NUM_LINES; ++i) {
            for (j = 0; j < NUM_COLS; ++j) {
                index = get_index_of(i, j);
                if (loop[index] == NONE) {
                    if (
                        (i && loop[get_index_of(i - 1, j)] == inside)
                        || (
                            i < NUM_LINES - 1
                            && loop[get_index_of(i + 1, j)] == inside
                        ) || (j && loop[get_index_of(i, j - 1)] == inside)
                        || (
                            j < NUM_COLS - 1
                            && loop[get_index_of(i, j + 1)] == inside
                        ) || (
                            i && j && loop[get_index_of(i - 1, j - 1)] == inside
                        ) || (
                            i
                            && j < NUM_COLS - 1
                            && loop[get_index_of(i - 1, j + 1)] == inside
                        ) || (
                            i < NUM_LINES - 1
                            && j
                            && loop[get_index_of(i + 1, j - 1)] == inside
                        ) || (
                            i < NUM_LINES - 1
                            && j < NUM_COLS - 1
                            && loop[get_index_of(i + 1, j + 1)] == inside
                        )
                    ) {
                        loop[index] = inside;
                        changes = 1;
                    }
                }
            }
        }
    }

    total = 0;
    for (i = 0; i < NUM_LINES; ++i)
        for (j = 0; j < NUM_COLS; ++j)
            if (loop[get_index_of(i, j)] == inside)
                ++total;

    for (i = 0; i < NUM_LINES; ++i) {
        for (j = 0; j < NUM_COLS; ++j) {
            index = get_index_of(i, j);
            if (loop[index] == NONE) {
                printf("\033[0;31m%c\033[0m", loop[index]);
            } else if (loop[index] == LEFT) {
                printf("\033[0;36m%c\033[0m", loop[index]);
            } else if (loop[index] == RIGHT) {
                printf("\033[0;32m%c\033[0m", loop[index]);
            } else {
                putchar(loop[index]);
            }
        }
        putchar('\n');
    }
    printf("Total = %zu\n", total);
    return 0;
}
