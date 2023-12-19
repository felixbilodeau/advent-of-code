#include <stdio.h>

#define LINE_LENGTH 140
#define NORTH 0
#define SOUTH 1
#define WEST 2
#define EAST 3

size_t get_index_of(size_t line, size_t col)
{
    return line * LINE_LENGTH + col;
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
    if (line == LINE_LENGTH - 1) return 0;
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
    if (col == LINE_LENGTH - 1) return 0;
    size_t index;
    index = get_index_of(line, ++col);
    return file[index] == '-' || file[index] == 'J' || file[index] == '7';
}

int main(void)
{
    char file[LINE_LENGTH * LINE_LENGTH];
    size_t distances[LINE_LENGTH * LINE_LENGTH], distance;
    size_t i, j, start_col, start_line, index;
    int last_move, path1_start, path2_start;
    int c;

    for (i = 0; i < LINE_LENGTH * LINE_LENGTH; ++i)
        distances[i] = 0;

    for (
        i = 0;
        (c = getchar()) != EOF
        && i < LINE_LENGTH * LINE_LENGTH;
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
    for (start_line = 0; start_line < LINE_LENGTH && !c; ++start_line)
        for (start_col = 0; start_col < LINE_LENGTH && !c; ++start_col)
            c = file[get_index_of(start_line, start_col)] == 'S';
    --start_col; --start_line;
    path1_start = path2_start = last_move = -1;
    if (check_north(file, start_line, start_col)) {
        if (path1_start == -1)
            path1_start = NORTH;
        else if (path2_start == -1)
            path2_start = NORTH;
        else
            puts("ERROR: Found more than 2 starting paths!");
    }
    if (check_south(file, start_line, start_col)) {
        if (path1_start == -1)
            path1_start = SOUTH;
        else if (path2_start == -1)
            path2_start = SOUTH;
        else
            puts("ERROR: Found more than 2 starting paths!");
    }
    if (check_west(file, start_line, start_col)) {
        if (path1_start == -1)
            path1_start = WEST;
        else if (path2_start == -1)
            path2_start = WEST;
        else
            puts("ERROR: Found more than 2 starting paths!");
    }
    if (check_east(file, start_line, start_col)) {
        if (path1_start == -1)
            path1_start = EAST;
        else if (path2_start == -1)
            path2_start = EAST;
        else
            puts("ERROR: Found more than 2 starting paths!");
    }

    i = start_line;
    j = start_col;
    switch (path1_start) {
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
    distance = 0;
    while (!(i == start_line && j == start_col)) {
        index = get_index_of(i, j);
        distances[index] = ++distance;
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
    switch (path2_start) {
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
    distance = 0;
    while (!(i == start_line && j == start_col)) {
        index = get_index_of(i, j);
        if (++distance < distances[index])
            distances[index] = distance;
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

    distance = 0;
    for (i = 0; i < LINE_LENGTH; ++i) {
        for (j = 0; j < LINE_LENGTH; ++j) {
            index = get_index_of(i, j);
            if (distances[index] > distance)
                distance = distances[index];
        }
    }
    printf("Max distance = %zu\n", distance);
    return 0;
}
