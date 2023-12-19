#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENERGIZED   '#'
#define EMPTY       '.'
#define F_MIRROR    '/'
#define B_MIRROR    '\\'
#define V_SPLITTER  '|'
#define H_SPLITTER  '-'

#define RIGHT   1U
#define LEFT    2U
#define UP      4U
#define DOWN    8U

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
    *dest = (struct Buffer2D) {0};
    dest->buffer = malloc(src->capacity);
    if (!dest->buffer) {
        perror("malloc");
        puts("Failed to allocate memory for Buffer2D copy");
        return 0;
    }
    dest->capacity = src->capacity;
    dest->num_lines = src->num_lines;
    dest->num_cols = src->num_cols;
    dest->length = src->length;
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

void Buffer2D_print(struct Buffer2D *buff2d)
{
    size_t line, col;
    for (line = 0; line < buff2d->num_lines; ++line) {
        for (col = 0; col < buff2d->num_cols; ++col) {
            putchar(Buffer2D_at(buff2d, line, col));
        }
        putchar('\n');
    }
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

void follow_beam(
    struct Buffer2D *arrangement,
    struct Buffer2D *energy,
    unsigned int movement,
    size_t line,
    size_t col
)
{
    char directions;
    directions = Buffer2D_at(energy, line, col);
    if (!!(directions & movement)) return;
    else Buffer2D_set_at(energy, line, col, directions | movement);

    /* Base case */
    if (Buffer2D_at(arrangement, line, col) == EMPTY) {
        switch (movement)
        {
        case RIGHT:
            if (col >= arrangement->num_cols - 1) return;
            break;
        case LEFT:
            if (col == 0) return;
            break;
        case UP:
            if (line == 0) return;
            break;
        case DOWN:
            if (line >= arrangement->num_lines - 1) return;
            break;
        }
    }

    /* Recurse */
    switch (Buffer2D_at(arrangement, line, col)) {
    case EMPTY:
        switch (movement) {
        case RIGHT:
            follow_beam(arrangement, energy, RIGHT, line, col + 1);
            break;
        case LEFT:
            follow_beam(arrangement, energy, LEFT, line, col - 1);
            break;
        case UP:
            follow_beam(arrangement, energy, UP, line - 1, col);
            break;
        case DOWN:
            follow_beam(arrangement, energy, DOWN, line + 1, col);
            break;
        }
        break;
    case F_MIRROR:
        switch (movement) {
        case RIGHT:
            if (line == 0) return;
            follow_beam(arrangement, energy, UP, line - 1, col);
            break;
        case LEFT:
            if (line == arrangement->num_lines - 1) return;
            follow_beam(arrangement, energy, DOWN, line + 1, col);
            break;
        case UP:
            if (col == arrangement->num_cols - 1) return;
            follow_beam(arrangement, energy, RIGHT, line, col + 1);
            break;
        case DOWN:
            if (col == 0) return;
            follow_beam(arrangement, energy, LEFT, line, col - 1);
            break;
        }
        break;
    case B_MIRROR:
        switch (movement) {
        case RIGHT:
            if (line == arrangement->num_lines - 1) return;
            follow_beam(arrangement, energy, DOWN, line + 1, col);
            break;
        case LEFT:
            if (line == 0) return;
            follow_beam(arrangement, energy, UP, line - 1, col);
            break;
        case UP:
            if (col == 0) return;
            follow_beam(arrangement, energy, LEFT, line, col - 1);
            break;
        case DOWN:
            if (col == arrangement->num_cols - 1) return;
            follow_beam(arrangement, energy, RIGHT, line, col + 1);
            break;
        }
        break;
    case V_SPLITTER:
        switch (movement) {
        case RIGHT:
        case LEFT:
            follow_beam(arrangement, energy, UP, line, col);
            follow_beam(arrangement, energy, DOWN, line, col);
            break;
        case UP:
            if (line == 0) return;
            follow_beam(arrangement, energy, UP, line - 1, col);
            break;
        case DOWN:
            if (line == arrangement->num_lines - 1) return;
            follow_beam(arrangement, energy, DOWN, line + 1, col);
            break;
        }
        break;
    case H_SPLITTER:
        switch (movement) {
        case RIGHT:
            if (col == arrangement->num_cols - 1) return;
            follow_beam(arrangement, energy, RIGHT, line, col + 1);
            break;
        case LEFT:
            if (col == 0) return;
            follow_beam(arrangement, energy, LEFT, line, col - 1);
            break;
        case UP:
        case DOWN:
            follow_beam(arrangement, energy, RIGHT, line, col);
            follow_beam(arrangement, energy, LEFT, line, col);
            break;
        }
        break;
    }
}

int main(void)
{
    struct Buffer2D arrangement, energy;
    size_t total, line, col;
    Buffer2D_create(&arrangement, 0);
    Buffer2D_load_file(&arrangement);
    Buffer2D_copy(&energy, &arrangement);
    memset(energy.buffer, '\0', energy.length);
    follow_beam(&arrangement, &energy, RIGHT, 0, 0);
    total = 0;
    for (line = 0; line < energy.num_lines; ++line) {
        for (col = 0; col < energy.num_cols; ++col) {
            if (Buffer2D_at(&energy, line, col) == '\0') {
                Buffer2D_set_at(&energy, line, col, EMPTY);
            } else {
                Buffer2D_set_at(&energy, line, col, ENERGIZED);
                ++total;
            }
        }
    }
    Buffer2D_print(&energy);
    printf("Total energized = %zu\n", total);
    Buffer2D_free_internals(&arrangement);
    Buffer2D_free_internals(&energy);
    return 0;
}
