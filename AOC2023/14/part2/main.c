#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ROUND_ROCK 'O'
#define CUBE_ROCK '#'
#define EMTPY '.'

struct Buffer {
    char *buffer;
    size_t length;
    size_t capacity;
};

struct Buffer2D {
    char *buffer;
    size_t num_lines;
    size_t num_cols;
    size_t length;
    size_t capacity;
};

struct HashMap_Buffer2D_Cycle {
    char *buffer;
    size_t cycle;
};

int Buffer_create(struct Buffer *buff, size_t start_capacity)
{
    if (!start_capacity)
        start_capacity = 128;
    buff->buffer = malloc(start_capacity);
    if (!buff->buffer) {
        puts("ERROR: Failed to allocate memory for the Buffer");
        return 0;
    }
    buff->capacity = start_capacity;
    buff->length = 0;
    return 1;
}

int Buffer_grow(struct Buffer *buff)
{
    char *temp;
    temp = realloc(buff->buffer, 2 * buff->capacity);
    if (!temp) {
        puts("WARNING: Could not allocate more memory");
        return 0;
    }
    buff->capacity = 2 * buff->capacity;
    buff->buffer = temp;
    return 1;
}

int Buffer_get_line(struct Buffer *buff)
{
    size_t i;
    int c;
    for (
        i = 0;
        (c = getchar()) != EOF
        && c != '\n';
        ++i
    ) {
        buff->buffer[i] = c;
        if (i >= buff->capacity - 1) {
            if (!Buffer_grow(buff)) {
                puts("WARNING: Could not load entire line into buffer");
                buff->length = buff->capacity;
                return 0;
            }
        }
    }
    buff->length = i;
    return 1;
}

void Buffer_free_internals(struct Buffer *buff)
{
    free(buff->buffer);
    buff->buffer = NULL;
    buff->length = 0;
    buff->capacity = 0;
}

int Buffer2D_create(
    struct Buffer2D *buff2d, size_t start_capacity
)
{
    if (!start_capacity)
        start_capacity = 128 * 128;
    buff2d->buffer = malloc(start_capacity);
    if (!buff2d->buffer) {
        puts("ERROR: Failed to allocate memory for the Buffer2D");
        return 0;
    }
    buff2d->capacity = start_capacity;
    buff2d->num_lines = buff2d->num_cols = buff2d->length = 0;
    return 1;
}

int Buffer2D_copy(struct Buffer2D *dest, struct Buffer2D *src)
{
    if (dest->buffer && dest->capacity != src->capacity) {
        free(dest->buffer);
        *dest = (struct Buffer2D) {0};
        dest->buffer = malloc(src->capacity);
        if (!dest->buffer) {
            puts("ERROR: Failed to allocate Buffer2D copy");
            return 0;
        }
        dest->capacity = src->capacity;
        dest->num_lines = src->num_lines;
        dest->num_cols = src->num_cols;
        dest->length = src->length;
    }
    memcpy(dest->buffer, src->buffer, src->length);
    return 1;
}

int Buffer2D_cmp(struct Buffer2D *buff2d, struct Buffer2D *other)
{
    if (
        buff2d->length != other->length
        || buff2d->num_lines != other->num_lines
        || buff2d->num_cols != other->num_cols
    )
        return 1;
    return memcmp(buff2d, other, buff2d->length);
}

size_t Buffer2D_hash(struct Buffer2D *buff2d, size_t array_size)
{
    size_t hash, i;
    hash = 5381;
    for (i = 0; i < buff2d->length; ++i)
        hash = ((hash << 5) + hash) + (size_t)(buff2d->buffer[i]);
    return hash % array_size;
}

int Buffer2D_grow(struct Buffer2D *buff2d)
{
    char *temp;
    temp = realloc(buff2d->buffer, 2 * buff2d->capacity);
    if (!temp) {
        puts("WARNING: Could not allocate more memory");
        return 0;
    }
    buff2d->capacity = 2 * buff2d->capacity;
    buff2d->buffer = temp;
    return 1;
}

int Buffer2D_insert_line(
    struct Buffer2D *buff2d, char *line, size_t line_length
)
{
    if (buff2d->num_cols && line_length > buff2d->num_cols) {
        puts("ERROR: Could not insert line: line too long");
        return 0;
    }
    while (buff2d->length + line_length > buff2d->capacity)
        if (!Buffer2D_grow(buff2d)) return 0;
    memset(buff2d->buffer + buff2d->length, 0, buff2d->num_cols);
    memcpy(buff2d->buffer + buff2d->length, line, line_length);
    if (!buff2d->num_cols)
        buff2d->num_cols = line_length;
    buff2d->length = buff2d->length + buff2d->num_cols;
    ++(buff2d->num_lines);
    return 1;
}

char Buffer2D_at(struct Buffer2D *buff2d, size_t line, size_t col)
{
    if (line >= buff2d->num_lines || col >= buff2d->num_cols) {
        printf("ERROR: Index (%zu, %zu) out of range\n", line, col);
        printf(
            "for a Buffer2D of size (%zu, %zu)\n",
            buff2d->num_lines,
            buff2d->num_cols
        );
        return '\0';
    }
    return buff2d->buffer[line * buff2d->num_cols + col];
}

int Buffer2D_set_at(struct Buffer2D *buff2d, size_t line, size_t col, char to)
{
    if (line >= buff2d->num_lines || col >= buff2d->num_cols) {
        printf("ERROR: Index (%zu, %zu) out of range\n", line, col);
        printf(
            "for a Buffer2D of size (%zu, %zu)\n",
            buff2d->num_lines,
            buff2d->num_cols
        );
        return 0;
    }
    buff2d->buffer[line * buff2d->num_cols + col] = to;
    return 1;
}

int Buffer2D_load_file(struct Buffer2D *buff2d)
{
    struct Buffer buff;
    
    if (!Buffer_create(&buff, 0)) {
        puts("ERROR: Could not load file: Failed to create line buffer");
        return 0;
    }

    Buffer_get_line(&buff);
    while (buff.length) {
        if (!Buffer2D_insert_line(buff2d, buff.buffer, buff.length))
            return 0;
        Buffer_get_line(&buff);
    }
    Buffer_free_internals(&buff);
    return 1;
}

void Buffer2D_free_internals(struct Buffer2D *buff2d)
{
    free(buff2d->buffer);
    buff2d->buffer = NULL;
    buff2d->length = 0;
    buff2d->capacity = 0;
    buff2d->num_cols = 0;
    buff2d->num_lines = 0;
}

void tilt_north(struct Buffer2D *buff2d)
{
    size_t col, line, line_ahead;
    for (col = 0; col < buff2d->num_cols; ++col) {
        line = 0;
        line_ahead = 1;
        while (line_ahead < buff2d->num_lines) {
            if (Buffer2D_at(buff2d, line, col) == EMTPY) {
                while (
                    line_ahead < buff2d->num_lines
                    && Buffer2D_at(buff2d, line_ahead, col) == EMTPY
                )
                    ++line_ahead;
                if (line_ahead < buff2d->num_lines) {
                    switch (Buffer2D_at(buff2d, line_ahead, col)) {
                    case ROUND_ROCK:
                        Buffer2D_set_at(buff2d, line, col, ROUND_ROCK);
                        Buffer2D_set_at(buff2d, line_ahead, col, EMTPY);
                        ++line;
                        ++line_ahead;
                        break;
                    case CUBE_ROCK:
                        line = ++line_ahead;
                        ++line_ahead;
                        break;
                    }
                }
            } else {
                ++line;
                if (line_ahead <= line)
                    line_ahead = line + 1;
            }
        }
    }
}

void tilt_south(struct Buffer2D *buff2d)
{
    size_t col, line, line_ahead;
    for (col = 0; col < buff2d->num_cols; ++col) {
        line = buff2d->num_lines - 1;
        line_ahead = line;
        while (1 <= line_ahead && line_ahead <= buff2d->num_lines) {
            if (Buffer2D_at(buff2d, line, col) == EMTPY) {
                while (
                    1 <= line_ahead && line_ahead <= buff2d->num_lines
                    && Buffer2D_at(buff2d, line_ahead - 1, col) == EMTPY
                )
                    --line_ahead;
                if (1 <= line_ahead && line_ahead <= buff2d->num_lines) {
                    switch (Buffer2D_at(buff2d, line_ahead - 1, col)) {
                    case ROUND_ROCK:
                        Buffer2D_set_at(buff2d, line, col, ROUND_ROCK);
                        Buffer2D_set_at(buff2d, line_ahead - 1, col, EMTPY);
                        --line;
                        --line_ahead;
                        break;
                    case CUBE_ROCK:
                        line = --line_ahead;
                        --line_ahead;
                        break;
                    }
                }
            } else {
                --line;
                if (line_ahead > line)
                    line_ahead = line;
            }
        }
    }
}

void tilt_west(struct Buffer2D *buff2d)
{
    size_t line, col, col_ahead;
    for (line = 0; line < buff2d->num_lines; ++line) {
        col = 0;
        col_ahead = 1;
        while (col_ahead < buff2d->num_cols) {
            if (Buffer2D_at(buff2d, line, col) == EMTPY) {
                while (
                    col_ahead < buff2d->num_cols
                    && Buffer2D_at(buff2d, line, col_ahead) == EMTPY
                )
                    ++col_ahead;
                if (col_ahead < buff2d->num_cols) {
                    switch (Buffer2D_at(buff2d, line, col_ahead)) {
                    case ROUND_ROCK:
                        Buffer2D_set_at(buff2d, line, col, ROUND_ROCK);
                        Buffer2D_set_at(buff2d, line, col_ahead, EMTPY);
                        ++col;
                        ++col_ahead;
                        break;
                    case CUBE_ROCK:
                        col = ++col_ahead;
                        ++col_ahead;
                        break;
                    }
                }
            } else {
                ++col;
                if (col_ahead <= col)
                    col_ahead = col + 1;
            }
        }
    }
}

void tilt_east(struct Buffer2D *buff2d)
{
    size_t line, col, col_ahead;
    for (line = 0; line < buff2d->num_lines; ++line) {
        col = buff2d->num_cols - 1;
        col_ahead = col;
        while (1 <= col_ahead && col_ahead <= buff2d->num_cols) {
            if (Buffer2D_at(buff2d, line, col) == EMTPY) {
                while (
                    1 <= col_ahead && col_ahead <= buff2d->num_cols
                    && Buffer2D_at(buff2d, line, col_ahead - 1) == EMTPY
                )
                    --col_ahead;
                if (1 <= col_ahead && col_ahead <= buff2d->num_cols) {
                    switch (Buffer2D_at(buff2d, line, col_ahead - 1)) {
                    case ROUND_ROCK:
                        Buffer2D_set_at(buff2d, line, col, ROUND_ROCK);
                        Buffer2D_set_at(buff2d, line, col_ahead - 1, EMTPY);
                        --col;
                        --col_ahead;
                        break;
                    case CUBE_ROCK:
                        col = --col_ahead;
                        --col_ahead;
                        break;
                    }
                }
            } else {
                --col;
                if (col_ahead > col)
                    col_ahead = col;
            }
        }
    }
}

void cycle(struct Buffer2D *buff2d)
{
    tilt_north(buff2d);
    tilt_west(buff2d);
    tilt_south(buff2d);
    tilt_east(buff2d);
}

size_t get_north_load(struct Buffer2D *buff2d)
{
    size_t load, line, col;
    load = 0;
    for (line = 0; line < buff2d->num_lines; ++line) {
        for (col = 0; col < buff2d->num_cols; ++col) {
            if (Buffer2D_at(buff2d, line, col) == ROUND_ROCK) {
                load = load + (buff2d->num_lines - line);
            }
        }
    }
    return load;
}

int store_cache(
    struct HashMap_Buffer2D_Cycle cache[],
    size_t cache_size,
    struct Buffer2D *buff2d,
    size_t cycle
)
{
    size_t start_index, i;

    start_index = Buffer2D_hash(buff2d, cache_size);
    for (i = start_index; i < cache_size; ++i) {
        if (!cache[i].buffer) {
            cache[i].buffer = malloc(buff2d->length);
            if (!cache[i].buffer) {
                puts("ERROR: Failed to allocate memory for the cache");
                return 0;
            }
            memcpy(cache[i].buffer, buff2d->buffer, buff2d->length);
            cache[i].cycle = cycle;
            return 1;
        } else {
            if (!memcmp(cache[i].buffer, buff2d->buffer, buff2d->length)) {
                return 0;
            }
        }
    }
    for (i = 0; i < start_index; ++i) {
        if (!cache[i].buffer) {
            cache[i].buffer = malloc(buff2d->length);
            if (!cache[i].buffer) {
                puts("ERROR: Failed to allocate memory for the cache");
                return 0;
            }
            memcpy(cache[i].buffer, buff2d->buffer, buff2d->length);
            return 1;
        } else {
            if (!memcmp(cache[i].buffer, buff2d->buffer, buff2d->length)) {
                return 0;
            }
        }
    }
    puts("ERROR: Ran out of space in the cache");
    return 0;
}

struct HashMap_Buffer2D_Cycle *get_cache(
    struct HashMap_Buffer2D_Cycle cache[],
    size_t cache_size,
    struct Buffer2D *buff2d
)
{
    size_t start_index, i;

    start_index = Buffer2D_hash(buff2d, cache_size);
    for (i = start_index; i < cache_size; ++i) {
        if (!cache[i].buffer) return NULL;
        if (!memcmp(cache[i].buffer, buff2d->buffer, buff2d->length))
            return cache + i;
    }
    for (i = 0; i < start_index; ++i) {
        if (!cache[i].buffer) return NULL;
        if (!memcmp(cache[i].buffer, buff2d->buffer, buff2d->length))
            return cache + i;
    }
    return NULL;
}

int main(void)
{
    const size_t cache_size = 200;
    const size_t num_cycles = 1000000000;
    struct Buffer2D buff2d;
    struct HashMap_Buffer2D_Cycle cache[cache_size], *node;
    size_t i, cycle_end, cycle_start, period;

    for (i = 0; i < cache_size; ++i)
        cache[i] = (struct HashMap_Buffer2D_Cycle) {
            .buffer = NULL,
            .cycle = 0
        };

    if (!Buffer2D_create(&buff2d, 0)) {
        puts("ERROR: Failed to create Buffer2D");
        return 1;
    }
    Buffer2D_load_file(&buff2d);
    store_cache(cache, cache_size, &buff2d, 0);
    for (
        i = 1;
        i < num_cycles;
        ++i
    ) {
        cycle(&buff2d);
        if (!store_cache(cache, cache_size, &buff2d, i)) break;
    }
    node = get_cache(cache, cache_size, &buff2d);
    if (!node) return 1;
    cycle_start = node->cycle;
    cycle_end = i;
    period = cycle_end - cycle_start;
    printf("cycle_start = %zu\n", cycle_start);
    printf("cycle_end = %zu\n", cycle_end);
    printf("period = %zu\n", period);
    i = (num_cycles - cycle_start) % period;
    printf("in cycle index = %zu\n", i);
    while (i) {
        cycle(&buff2d);
        --i;
    }
    printf("North load: %zu\n", get_north_load(&buff2d));
    Buffer2D_free_internals(&buff2d);
    for (i = 0; i < cache_size; ++i)
        if (cache[i].buffer)
            free(cache[i].buffer);
    return 0;
}
