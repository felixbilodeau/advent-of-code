#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_SIZE_START 1024

#define EMPTY '.'
#define GALAXY '#'
#define EOL '\n'

#define EXPANSION 1000000

struct Galaxy {
    size_t i;
    size_t j;
};

struct BufferDescriptor {
    char *buffer;
    size_t length;
    size_t capacity;
    size_t num_lines;
    size_t num_cols;
};

struct BufferDescriptor reallocate_buffer(struct BufferDescriptor bd)
{
    char *new_buffer;

    new_buffer = realloc(bd.buffer, bd.capacity);
    if (!new_buffer) {
        puts("ERROR: Could not reallocate buffer!");
        return (struct BufferDescriptor) {.buffer = NULL};
    }
    bd.buffer = new_buffer;
    return bd;
}

struct BufferDescriptor load_file(struct BufferDescriptor bd)
{
    size_t i;
    int c;
    
    for (i = bd.length; (c = getchar()) != EOF; ++i) {
        if (i >= bd.capacity - 1) {
            bd.capacity = 2 * bd.capacity;
            bd = reallocate_buffer(bd);
            if (!bd.buffer) return bd;
        }
        bd.buffer[i] = c;
    }
    if (i >= bd.capacity) {
        bd.capacity = bd.capacity * 2;
        bd = reallocate_buffer(bd);
        if (!bd.buffer) return bd;
    }
    bd.buffer[i] = '\0';
    bd.length = i;
    return bd;
}

struct BufferDescriptor get_coords(struct BufferDescriptor bd)
{
    size_t i;

    for (i = 0; i < bd.length && bd.buffer[i] != EOL; ++i);
    if (i == bd.length) {
        puts("ERROR: Could not determine number of columns");
        return bd;
    }
    bd.num_cols = ++i;
    bd.num_lines = bd.length / bd.num_cols;
    return bd;
}

size_t get_index_of(size_t i, size_t j, size_t num_cols)
{
    return i * num_cols + j;
}

size_t count_galaxies(struct BufferDescriptor bd)
{
    size_t i, num_galaxies;
    for (i = num_galaxies = 0; i < bd.length; ++i)
        if (bd.buffer[i] == GALAXY) ++ num_galaxies;
    return num_galaxies;
}

size_t load_galaxies(struct BufferDescriptor bd, struct Galaxy galaxies[])
{
    size_t i, j, count;
    for (i = count = 0; i < bd.num_lines; ++i)
        for (j = 0; j < bd.num_cols - 1; ++j)
            if (bd.buffer[get_index_of(i, j, bd.num_cols)] == GALAXY)
                galaxies[count++] = (struct Galaxy) {
                    .i = i, .j = j
                };
    return count;
}

size_t count_empty_lines(struct BufferDescriptor bd)
{
    size_t i, j, count;
    int is_empty;
    for (i = count = 0; i < bd.num_lines; ++i) {
        is_empty = 1;
        for (j = 0; j < bd.num_cols - 1 && is_empty; ++j)
            if (bd.buffer[get_index_of(i, j, bd.num_cols)] != EMPTY)
                is_empty = 0;
        if (is_empty)
            ++count;
    }
    return count;
}

size_t count_empty_cols(struct BufferDescriptor bd)
{
    size_t i, j, count;
    int is_empty;
    for (j = count = 0; j < bd.num_cols - 1; ++j) {
        is_empty = 1;
        for (i = 0; i < bd.num_lines && is_empty; ++i)
            if (bd.buffer[get_index_of(i, j, bd.num_cols)] != EMPTY)
                is_empty = 0;
        if (is_empty)
            ++count;
    }
    return count;
}

size_t load_empty_lines(struct BufferDescriptor bd, size_t empty_lines[])
{
    size_t i, j, count;
    int is_empty;
    for (i = count = 0; i < bd.num_lines; ++i) {
        is_empty = 1;
        for (j = 0; j < bd.num_cols - 1 && is_empty; ++j)
            if (bd.buffer[get_index_of(i, j, bd.num_cols)] != EMPTY)
                is_empty = 0;
        if (is_empty)
            empty_lines[count++] = i;
    }
    return count;
}

size_t load_empty_cols(struct BufferDescriptor bd, size_t empty_cols[])
{
    size_t i, j, count;
    int is_empty;
    for (j = count = 0; j < bd.num_cols - 1; ++j) {
        is_empty = 1;
        for (i = 0; i < bd.num_lines && is_empty; ++i)
            if (bd.buffer[get_index_of(i, j, bd.num_cols)] != EMPTY)
                is_empty = 0;
        if (is_empty)
            empty_cols[count++] = j;
    }
    return count;
}

size_t get_distance_between(
    struct Galaxy galaxy1,
    struct Galaxy galaxy2,
    size_t empty_lines[],
    size_t empty_cols[],
    size_t num_empty_lines,
    size_t num_empty_cols
)
{
    size_t crossed_empty_lines, crossed_empty_cols, i;
    size_t min_line, max_line, min_col, max_col;

    if (galaxy1.i < galaxy2.i) {
        max_line = galaxy2.i;
        min_line = galaxy1.i;
    } else {
        max_line = galaxy1.i;
        min_line = galaxy2.i;
    }

    if (galaxy1.j < galaxy2.j) {
        max_col = galaxy2.j;
        min_col = galaxy1.j;
    } else {
        max_col = galaxy1.j;
        min_col = galaxy2.j;
    }

    i = 0;
    while (i < num_empty_lines && empty_lines[i] < min_line)
        ++i;
    crossed_empty_lines = 0;
    while (i < num_empty_lines && empty_lines[i] < max_line) {
        ++i;
        ++crossed_empty_lines;
    }

    i = 0;
    while (i < num_empty_cols && empty_cols[i] < min_col)
        ++i;
    crossed_empty_cols = 0;
    while (i < num_empty_cols && empty_cols[i] < max_col) {
        ++i;
        ++crossed_empty_cols;
    }

    return (
        (max_line - min_line - crossed_empty_lines)
        + EXPANSION * crossed_empty_lines
        + (max_col - min_col - crossed_empty_cols)
        + EXPANSION * crossed_empty_cols
    );
}

int main(void)
{
    struct BufferDescriptor bd;
    size_t i, j, num_galaxies, total;
    size_t num_empty_cols, num_empty_lines;

    bd = (struct BufferDescriptor) {
        .buffer = malloc(FILE_SIZE_START),
        .length = 0,
        .capacity = FILE_SIZE_START,
        .num_lines = 0,
        .num_cols = 0
    };
    if (!bd.buffer) {
        puts("ERROR: Failed to allocate memory");
        return 1;
    }

    bd = load_file(bd);
    if (!bd.buffer) {
        puts("ERROR: error reading file into memory");
        return 1;
    }

    bd = get_coords(bd);

    num_galaxies = count_galaxies(bd);
    num_empty_lines = count_empty_lines(bd);
    num_empty_cols = count_empty_cols(bd);

    size_t empty_cols[num_empty_cols], empty_lines[num_empty_lines];
    struct Galaxy galaxies[num_galaxies];

    if (load_empty_lines(bd, empty_lines) != num_empty_lines) {
        puts("ERROR: Failed to load empty lines!");
        free(bd.buffer);
        return 1;
    }

    if (load_empty_cols(bd, empty_cols) != num_empty_cols) {
        puts("ERROR: Failed to load empty columns!");
        free(bd.buffer);
        return 1;
    }

    if (load_galaxies(bd, galaxies) != num_galaxies) {
        puts("ERROR: Failed to load galaxies!");
        free(bd.buffer);
        return 1;
    }

    free(bd.buffer);
    bd = (struct BufferDescriptor) {0};
    bd.buffer = NULL;

    total = 0;
    for (i = 0; i < num_galaxies; ++i) {
        for (j = i + 1; j < num_galaxies; ++j) {
            total = (
                total
                + get_distance_between(
                    galaxies[i],
                    galaxies[j],
                    empty_lines,
                    empty_cols,
                    num_empty_lines,
                    num_empty_cols
                )
            );
        }
    }
    printf("Total distance = %zu\n", total);
    return 0;
}
