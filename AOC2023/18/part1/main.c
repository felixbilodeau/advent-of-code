#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_LINE 32

#define UP      'U'
#define DOWN    'D'
#define LEFT    'L'
#define RIGHT   'R'

#define DUG '#'
#define LEVEL '.'

#define PATH_LEFT   '<'
#define PATH_RIGHT  '>'

#define MIN_BUFFER 8

struct BufferDescriptor {
    char **buffer;
    size_t length;
    size_t capacity;
};

struct Step {
    char direction;
    int32_t step;
};

struct Bounds {
    int32_t min_x;
    int32_t max_x;
    int32_t min_y;
    int32_t max_y;
};

struct BufferDescriptor *BufferDescriptor_create(size_t start_capacity)
{
    struct BufferDescriptor *bd;
    char **temp;
    if (start_capacity < MIN_BUFFER) start_capacity = MIN_BUFFER;
    bd = malloc(sizeof(*bd) + sizeof(*(bd->buffer)));
    if (!bd) {
        perror("malloc");
        puts("Failed to allocate BufferDescriptor");
        return NULL;
    }
    temp = malloc(start_capacity * sizeof(*(bd->buffer)));
    if (!temp) {
        perror("malloc");
        puts("Failed to allocate BufferDescriptor");
        free(bd);
        return NULL;
    }
    *bd = (struct BufferDescriptor) {
        .buffer = temp,
        .length = 0,
        .capacity = start_capacity
    };
    return bd;
}

int BufferDescriptor_grow(struct BufferDescriptor *bd)
{
    char **temp;
    size_t new_capacity;
    new_capacity = 2 * bd->capacity;
    temp = realloc(bd->buffer, new_capacity * sizeof(*(bd->buffer)));
    if (!temp) {
        perror("realloc");
        puts("Could not grow BufferDescriptor");
        return 0;
    }
    bd->buffer = temp;
    bd->capacity = new_capacity;
    return 1;
}

int BufferDescriptor_shrink(struct BufferDescriptor *bd)
{
    char **temp;
    size_t new_capacity;
    new_capacity = bd->capacity / 2;
    if (new_capacity < MIN_BUFFER) new_capacity = MIN_BUFFER;
    temp = realloc(bd->buffer, new_capacity * sizeof(*(bd->buffer)));
    if (!temp) {
        perror("realloc");
        puts("Could not shrink BufferDescriptor");
        return 0;
    }
    bd->buffer = temp;
    bd->capacity = new_capacity;
    return 1;
}

int BufferDescriptor_insert_line(
    struct BufferDescriptor *bd, char *line, size_t line_length
)
{
    char *line_cpy;
    if (bd->length + 1 >= bd->capacity)
        if (!BufferDescriptor_grow(bd)) return 0;
    line_cpy = malloc((line_length + 1) * sizeof(*line));
    if (!line_cpy) {
        perror("malloc");
        puts("Failed to copy line into BufferDesscriptor");
        return 0;
    }
    memcpy(line_cpy, line, line_length + 1);
    bd->buffer[bd->length] = line_cpy;
    ++(bd->length);
    return 1;
}

void BufferDescriptor_free(struct BufferDescriptor *bd)
{
    size_t i;
    for (i = 0; i < bd->length; ++i)
        free(bd->buffer[i]);
    free(bd->buffer);
    free(bd);
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
        puts("Warning: Could not load entire line into buffer");
    line[i] = '\0';
    return i;
}

int is_digit(char c)
{
    return '0' <= c && c <= '9';
}

int32_t get_step(char **p_line)
{
    int32_t number, multiplier;
    char *base, *end;
    while (!is_digit(**p_line)) ++(*p_line);
    base = *p_line;
    while (is_digit(**p_line)) ++(*p_line);
    end = --(*p_line);
    number = 0;
    multiplier = 1;
    while (*p_line != base) {
        number = number + multiplier * (int32_t)(**p_line - '0');
        multiplier = multiplier * 10;
        --(*p_line);
    }
    number = number + multiplier * (int32_t)(**p_line - '0');
    *p_line = end;
    return number;
}

void parse_direction(
    char direction,
    size_t step,
    int32_t *current_x,
    int32_t *current_y,
    int32_t *min_x,
    int32_t *max_x,
    int32_t *min_y,
    int32_t *max_y
)
{
    switch (direction) {
    case UP:
        *current_y = *current_y - step;
        if (*current_y < *min_y)
            *min_y = *current_y;
        break;
    case DOWN:
        *current_y = *current_y + step;
        if (*current_y > *max_y)
            *max_y = *current_y;
        break;
    case LEFT:
        *current_x = *current_x - step;
        if (*current_x < *min_x)
            *min_x = *current_x;
        break;
    case RIGHT:
        *current_x = *current_x + step;
        if (*current_x > *max_x)
            *max_x = *current_x;
        break;
    default:
        puts("Failed to parse step");
        break;
    }
}

char **get_ground(size_t num_lines, size_t num_cols)
{
    char **ground;
    size_t line;
    ground = malloc(num_lines * sizeof(*ground));
    if (!ground) {
        perror("malloc");
        puts("Failed to allocate ground array");
        return NULL;
    }
    for (line = 0; line < num_lines; ++line) {
        ground[line] = malloc(num_cols * sizeof(**ground));
        if (!ground[line]) {
            perror("malloc");
            puts("Failed to allocate ground array");
            while (line) {
                free(ground[line - 1]);
                --line;
            }
            free(ground);
            return NULL;
        }
        memset(ground[line], LEVEL, num_cols * sizeof(**ground));
    }
    return ground;
}

void free_ground(char **ground, size_t num_lines)
{
    size_t line;
    for (line = 0; line < num_lines; ++line) free(ground[line]);
    free(ground);
}

void apply_step(
    struct Step step,
    char **ground,
    size_t *current_x,
    size_t *current_y,
    size_t num_lines,
    size_t num_cols
)
{
    size_t final_x, final_y;
    switch(step.direction) {
    case UP:
        final_x = *current_x;
        final_y = *current_y - step.step;
        while (*current_y != final_y) {
            if (*current_x && ground[*current_y][*current_x - 1] == LEVEL)
                ground[*current_y][*current_x - 1] = PATH_LEFT;
            if (
                *current_x < num_cols - 1
                && ground[*current_y][*current_x + 1] == LEVEL
            )
                ground[*current_y][*current_x + 1] = PATH_RIGHT;
            ground[*current_y][*current_x] = DUG;
            --(*current_y);
        }
        if (*current_x && ground[*current_y][*current_x - 1] == LEVEL)
            ground[*current_y][*current_x - 1] = PATH_LEFT;
        if (
            *current_x < num_cols - 1
            && ground[*current_y][*current_x + 1] == LEVEL
        )
            ground[*current_y][*current_x + 1] = PATH_RIGHT;
        ground[*current_y][*current_x] = DUG;
        break;
    case DOWN:
        final_x = *current_x;
        final_y = *current_y + step.step;
        while (*current_y != final_y) {
            if (*current_x && ground[*current_y][*current_x - 1] == LEVEL)
                ground[*current_y][*current_x - 1] = PATH_RIGHT;
            if (
                *current_x < num_cols - 1
                && ground[*current_y][*current_x + 1] == LEVEL
            )
                ground[*current_y][*current_x + 1] = PATH_LEFT;
            ground[*current_y][*current_x] = DUG;
            ++(*current_y);
        }
        if (*current_x && ground[*current_y][*current_x - 1] == LEVEL)
            ground[*current_y][*current_x - 1] = PATH_RIGHT;
        if (
            *current_x < num_cols - 1
            && ground[*current_y][*current_x + 1] == LEVEL
        )
            ground[*current_y][*current_x + 1] = PATH_LEFT;
        ground[*current_y][*current_x] = DUG;
        break;
    case LEFT:
        final_x = *current_x - step.step;
        final_y = *current_y;
        while (*current_x != final_x) {
            if (*current_y && ground[*current_y - 1][*current_x] == LEVEL)
                ground[*current_y - 1][*current_x] = PATH_RIGHT;
            if (
                *current_y < num_lines - 1
                && ground[*current_y + 1][*current_x] == LEVEL
            )
                ground[*current_y + 1][*current_x] = PATH_LEFT;
            ground[*current_y][*current_x] = DUG;
            --(*current_x);
        }
        if (*current_y && ground[*current_y - 1][*current_x] == LEVEL)
            ground[*current_y - 1][*current_x] = PATH_RIGHT;
        if (
            *current_y < num_lines - 1
            && ground[*current_y + 1][*current_x] == LEVEL
        )
            ground[*current_y + 1][*current_x] = PATH_LEFT;
        ground[*current_y][*current_x] = DUG;
        break;
    case RIGHT:
        final_x = *current_x + step.step;
        final_y = *current_y;
        while (*current_x != final_x) {
            if (*current_y && ground[*current_y - 1][*current_x] == LEVEL)
                ground[*current_y - 1][*current_x] = PATH_LEFT;
            if (
                *current_y < num_lines - 1
                && ground[*current_y + 1][*current_x] == LEVEL
            )
                ground[*current_y + 1][*current_x] = PATH_RIGHT;
            ground[*current_y][*current_x] = DUG;
            ++(*current_x);
        }
        if (*current_y && ground[*current_y - 1][*current_x] == LEVEL)
            ground[*current_y - 1][*current_x] = PATH_LEFT;
        if (
            *current_y < num_lines - 1
            && ground[*current_y + 1][*current_x] == LEVEL
        )
            ground[*current_y + 1][*current_x] = PATH_RIGHT;
        ground[*current_y][*current_x] = DUG;
        break;
    }
}

void define_in_out(char **ground, size_t num_lines, size_t num_cols)
{
    size_t line, col, ray_line, ray_col;
    for (line = 0; line < num_lines; ++line) {
        for (col = 0; col < num_cols; ++col) {
            if (ground[line][col] == LEVEL) {
                for (
                    ray_col = col;
                    ray_col && ground[line][ray_col - 1] == LEVEL;
                    --ray_col
                );
                if (ray_col) {
                    ground[line][col] = ground[line][ray_col - 1];
                    continue;
                }
                for (
                    ray_line = line;
                    ray_line && ground[ray_line - 1][col] == LEVEL;
                    --ray_line
                );
                if (ray_line) {
                    ground[line][col] = ground[ray_line - 1][col];
                    continue;
                }
                for (
                    ray_col = col;
                    ray_col < num_cols - 1
                    && ground[line][ray_col + 1] == LEVEL;
                    ++ray_col
                );
                if (ray_col < num_cols - 1) {
                    ground[line][col] = ground[line][ray_col + 1];
                    continue;
                }
                for (
                    ray_line = line;
                    ray_line < num_lines - 1
                    && ground[ray_line + 1][col] == LEVEL;
                    ++ray_line
                );
                if (ray_line) {
                    ground[line][col] = ground[ray_line + 1][col];
                    continue;
                }
            }
        }
    }
}

char get_out_char(char **ground, size_t num_lines, size_t num_cols)
{
    size_t line, col;
    for (line = 0; line < num_lines; ++line) {
        for (col = 0; col < num_cols && ground[line][col] != DUG; ++col) {
            if (
                ground[line][col] == PATH_LEFT
                || ground[line][col] == PATH_RIGHT
            ) return ground[line][col];
        }
    }
    for (line = 0; line < num_lines; ++line) {
        for (col = num_cols; col && ground[line][col - 1] != DUG; --col) {
            if (
                ground[line][col - 1] == PATH_LEFT
                || ground[line][col - 1] == PATH_RIGHT
            ) return ground[line][col - 1];
        }
    }
    for (col = 0; col < num_cols; ++col) {
        for (line = 0; line < num_lines && ground[line][col] != DUG; ++line) {
            if (
                ground[line][col] == PATH_LEFT
                || ground[line][col] == PATH_RIGHT
            ) return ground[line][col];
        }
    }
    for (col = 0; col < num_cols; ++col) {
        for (line = num_lines; line && ground[line - 1][col] != DUG; --line) {
            if (
                ground[line - 1][col] == PATH_LEFT
                || ground[line - 1][col] == PATH_RIGHT
            ) return ground[line - 1][col];
        }
    }
    return LEVEL;
}

size_t total_dug_volume(
    struct Step *steps, size_t num_steps, struct Bounds bounds
)
{
    size_t num_lines, num_cols, start_x, start_y, x, y;
    size_t current_x, current_y, step, total;
    char **ground, in_char, out_char;
    num_cols = (size_t)(bounds.max_x - bounds.min_x) + 1;
    num_lines = (size_t)(bounds.max_y - bounds.min_y) + 1;
    ground = get_ground(num_lines, num_cols);
    start_x = (size_t)(-bounds.min_x);
    start_y = (size_t)(-bounds.min_y);
    current_x = start_x;
    current_y = start_y;
    ground[start_y][start_x] = DUG;
    for (step = 0; step < num_steps; ++step)
        apply_step(
            steps[step], ground, &current_x, &current_y, num_lines, num_cols
        );
    define_in_out(ground, num_lines, num_cols);
    out_char = get_out_char(ground, num_lines, num_cols);
    switch (out_char) {
    case PATH_LEFT:
        in_char = PATH_RIGHT;
        break;
    case PATH_RIGHT:
        in_char = PATH_LEFT;
        break;
    default:
        in_char = LEVEL;
        return 0;
    }
    total = 0;
    for (y = 0; y < num_lines; ++y)
        for (x = 0; x < num_cols; ++x)
            if (ground[y][x] == in_char || ground[y][x] == DUG)
                ++total;
    steps[num_steps - 1].step = steps[num_steps - 1].step;
    free_ground(ground, num_lines);
    return total;
}

size_t parse_steps(struct Step *steps, struct BufferDescriptor *bd)
{
    struct Bounds bounds;
    int32_t min_x, min_y, max_x, max_y;
    int32_t step, current_x, current_y;
    size_t line;
    char *p_line, direction;
    min_x = min_y = max_x = max_y = current_x = current_y = 0;
    for (line = 0; line < bd->length; ++line) {
        p_line = bd->buffer[line];
        direction = *p_line;
        p_line = p_line + 2;
        step = get_step(&p_line);
        steps[line] = (struct Step) {
            .direction = direction,
            .step = step
        };
        parse_direction(
            direction,
            step,
            &current_x,
            &current_y,
            &min_x,
            &max_x,
            &min_y,
            &max_y
        );
    }
    bounds = (struct Bounds) {
        .min_x = min_x,
        .max_x = max_x,
        .min_y = min_y,
        .max_y = max_y
    };
    return total_dug_volume(steps, bd->length, bounds);
}

void parse_plan(struct BufferDescriptor *bd)
{
    struct Step *steps;
    size_t total;
    steps = malloc(bd->length * sizeof(*steps));
    if (!steps) {
        perror("malloc");
        puts("Failed to create Steps array");
        return;
    }
    total = parse_steps(steps, bd);
    printf("Total = %zu\n", total);
    free(steps);
}

int main(void)
{
    struct BufferDescriptor *bd;
    char line[MAX_LINE];
    size_t line_length;
    bd = BufferDescriptor_create(0);
    while ((line_length = get_line(line, MAX_LINE))) {
        if (!BufferDescriptor_insert_line(bd, line, line_length)) {
            BufferDescriptor_free(bd);
            return 1;
        }
    }
    parse_plan(bd);
    BufferDescriptor_free(bd);
    return 0;
}
