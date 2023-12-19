#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Buffer2D {
    char *buffer;
    size_t num_lines;
    size_t num_cols;
    size_t length;
    size_t capacity;
};

struct Buffer {
    char *buffer;
    size_t length;
    size_t capacity;
};

#define ROUND_ROCK 'O'
#define CUBE_ROCK '#'
#define EMTPY '.'

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
        puts("ERROR: Index out of range");
        return '\0';
    }
    return buff2d->buffer[line * buff2d->num_cols + col];
}

int Buffer2D_set_at(struct Buffer2D *buff2d, size_t line, size_t col, char to)
{
    if (line >= buff2d->num_lines || col >= buff2d->num_cols) {
        puts("ERROR: Index out of range");
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

int main(void)
{
    struct Buffer2D buff2d;
    size_t i, j;

    if (!Buffer2D_create(&buff2d, 0)) {
        puts("ERROR: Failed to create Buffer2D");
        return 1;
    }
    Buffer2D_load_file(&buff2d);
    tilt_north(&buff2d);
    for (i = 0; i < buff2d.num_lines; ++i) {
        for (j = 0; j < buff2d.num_cols; ++j)
            putchar(Buffer2D_at(&buff2d, i, j));
        putchar('\n');
    }
    printf("North load: %zu\n", get_north_load(&buff2d));
    Buffer2D_free_internals(&buff2d);

    return 0;
}
