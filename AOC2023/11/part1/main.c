#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILE_SIZE_START 1024

#define EMPTY '.'
#define GALAXY '#'
#define EOL '\n'

struct Galaxy {
    int i;
    int j;
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

struct BufferDescriptor duplicate(struct BufferDescriptor bd)
{
    size_t i, j, ii, end;
    int is_empty;
    for (i = 0; i < bd.num_lines; ++i) {
        is_empty = 1;
        for (j = 0; j < bd.num_cols - 1 && is_empty; ++j)
            if (bd.buffer[get_index_of(i, j, bd.num_cols)] != EMPTY)
                is_empty = 0;
        if (is_empty) {
            bd.length = bd.length + bd.num_cols;
            if (bd.length >= bd.capacity) {
                bd.capacity = 2 * bd.capacity;
                bd = reallocate_buffer(bd);
                if (!bd.buffer) return bd;
            }
            ++(bd.num_lines);
            for (ii = bd.num_lines - 1; ii > i; --ii) {
                memcpy(
                    bd.buffer + get_index_of(ii, 0, bd.num_cols),
                    bd.buffer + get_index_of(ii - 1, 0, bd.num_cols),
                    bd.num_cols
                );
            }
            ++i;
        }
    }
    
    for (j = 0; j < bd.num_cols - 1; ++j) {
        is_empty = 1;
        for (i = 0; i < bd.num_lines && is_empty; ++i)
            if (bd.buffer[get_index_of(i, j, bd.num_cols)] != EMPTY)
                is_empty = 0;
        if (is_empty) {
            end = bd.length;
            bd.length = bd.length + bd.num_lines;
            if (bd.length >= bd.capacity) {
                bd.capacity = 2 * bd.capacity;
                bd = reallocate_buffer(bd);
                if (!bd.buffer) return bd;
            }

            for (ii = bd.length - 1; end > 0; --ii) {
                if (((end - 1) % bd.num_cols) == j) {
                    bd.buffer[ii] = bd.buffer[end - 1];
                    --ii;
                }
                bd.buffer[ii] = bd.buffer[end - 1];
                --end;
            }
            ++(bd.num_cols);
            ++j;
        }
    }
    
    return bd;
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
                    .i = (int)i, .j = (int)j
                };
    return count;
}

int main(void)
{
    struct BufferDescriptor bd;
    size_t i, j, num_galaxies, total;

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

    num_galaxies = count_galaxies(bd);
    bd = get_coords(bd);
    bd = duplicate(bd);

    struct Galaxy galaxies[num_galaxies];

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
                + (size_t)abs(galaxies[i].i - galaxies[j].i)
                + (size_t)abs(galaxies[i].j - galaxies[j].j)
            );
        }
    }
    printf("Total distance = %zu\n", total);
    return 0;
}
