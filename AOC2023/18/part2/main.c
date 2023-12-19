#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define MAX_LINE 32

#define RIGHT   '0'
#define DOWN    '1'
#define LEFT    '2'
#define UP      '3'

#define POUND '#'

#define MIN_BUFFER 8

struct BufferDescriptor {
    char **buffer;
    size_t length;
    size_t capacity;
};

struct Vertex {
    int64_t x;
    int64_t y;
};

struct VertexDynamicArray {
    struct Vertex *vertices;
    size_t length;
    size_t capacity;
};

int64_t absi64(int64_t number)
{
    if (number < 0) return -number;
    return number;
}

struct VertexDynamicArray *VertexDynamicArray_create(size_t start_capacity)
{
    struct VertexDynamicArray *vda;
    vda = malloc(sizeof(*vda));
    if (!vda) {
        perror("malloc");
        puts("Failed to allocate VertexDynamicArray");
        return NULL;
    }
    if (start_capacity < MIN_BUFFER) start_capacity = MIN_BUFFER;
    vda->vertices = malloc(start_capacity * sizeof(*(vda->vertices)));
    if (!vda->vertices) {
        perror("malloc");
        puts("Failed to allocate Vertex array");
        free(vda);
        return NULL;
    }
    vda->capacity = start_capacity;
    vda->length = 0;
    return vda;
}

int VertexDynamicArray_grow(struct VertexDynamicArray *vda)
{
    struct Vertex *temp;
    size_t new_capacity;
    new_capacity = 2 * vda->capacity;
    temp = realloc(vda->vertices, new_capacity * sizeof(*(vda->vertices)));
    if (!temp) {
        perror("realloc");
        puts("Failed to grow VertexDynamicArray");
        return 0;
    }
    vda->vertices = temp;
    vda->capacity = new_capacity;
    return 1;
}

int VertexDynamicArray_insert(struct VertexDynamicArray *vda, struct Vertex vertex)
{
    if (vda->length + 1 >= vda->capacity)
        if (!VertexDynamicArray_grow(vda)) return 0;
    vda->vertices[(vda->length)++] = vertex;
    return 1;
}

void VertexDynamicArray_free(struct VertexDynamicArray *vda)
{
    free(vda->vertices);
    free(vda);
}

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

int is_decimal_digit(char c)
{
    return '0' <= c && c <= '9';
}

int is_hex_digit(char c)
{
    return is_decimal_digit(c) || ('a' <= c && c <= 'f');
}

int64_t parse_hex(char **p_line)
{
    int64_t number, multiplier, current;
    char *base, *end;
    base = *p_line;
    while (is_hex_digit(**p_line)) ++(*p_line);
    end = (*p_line)--;
    number = 0;
    multiplier = 1;
    while (*p_line != base) {
        current = 0;
        if (is_decimal_digit(**p_line))
            current = (int64_t)(**p_line - '0');
        else
            current = 10 + (int64_t)(**p_line - 'a');
        number = number + multiplier * current;
        multiplier = 16 * multiplier;
        --(*p_line);
    }
    current = 0;
    if (is_decimal_digit(**p_line))
        current = (int64_t)(**p_line - '0');
    else
        current = 10 + (int64_t)(**p_line - 'a');
    number = number + multiplier * current;
    *p_line = end;
    return number;
}

int seek_pound(char **p_line)
{
    while (**p_line != POUND) ++(*p_line);
    return !!(**p_line);
}

struct Vertex parse_step(char *line, int64_t *current_x, int64_t *current_y)
{
    char *p_line, direction;
    int64_t step;
    struct Vertex vertex;
    p_line = line;
    seek_pound(&p_line);
    direction = *(p_line + 6);
    *(p_line + 6) = '\0';
    ++p_line;
    step = parse_hex(&p_line);
    switch (direction) {
    case UP:
        *current_y = *current_y + step;
        break;
    case DOWN:
        *current_y = *current_y - step;
        break;
    case LEFT:
        *current_x = *current_x - step;
        break;
    case RIGHT:
        *current_x = *current_x + step;
        break;
    }
    vertex.x = *current_x;
    vertex.y = *current_y;
    return vertex;
}

size_t shoelace_area(struct VertexDynamicArray *vda)
{
    size_t area, vertex, i, j;
    area = 0;
    for (vertex = 1; vertex <= vda->length; ++vertex) {
        i = vertex - 1;
        j = vertex + 1;
        if (j == vda->length) j = 0;
        else if (j == vda->length + 1) j = 1;
        area = area + vda->vertices[vertex].y * (
            vda->vertices[j].x - vda->vertices[i].x
        );
    }
    return area / 2;
}

size_t pick_area(size_t perimeter, size_t interior_area)
{
    return interior_area + perimeter / 2 + 1;
}

size_t get_perimeter(struct VertexDynamicArray *vda)
{
    size_t vertex, next, perimeter;
    perimeter = 0;
    for (vertex = 0; vertex < vda->length; ++vertex) {
        next = vertex + 1;
        if (next == vda->length) next = 0;
        perimeter = perimeter + absi64(
            vda->vertices[vertex].x - vda->vertices[next].x
        ) + absi64(
            vda->vertices[vertex].y - vda->vertices[next].y
        );
    }
    return perimeter;
}

size_t get_total_area(struct VertexDynamicArray *vda)
{
    size_t area;
    area = shoelace_area(vda);
    area = pick_area(get_perimeter(vda), area);
    return area;
}

int parse(struct BufferDescriptor *bd)
{
    size_t step, area;
    struct VertexDynamicArray *vda;
    int64_t current_x, current_y;
    vda = VertexDynamicArray_create(0);
    if (!vda) return 0;
    current_x = current_y = 0;
    for (step = 0; step < bd->length; ++step) {
        if (
            !VertexDynamicArray_insert(
                vda,
                parse_step(
                    bd->buffer[step],
                    &current_x,
                    &current_y
                )
            )
        ) {
            VertexDynamicArray_free(vda);
            return 0;
        }
    }
    area = get_total_area(vda);
    printf("Total area: %zu\n", area);
    VertexDynamicArray_free(vda);
    return 1;
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
    if (!parse(bd)) {
        BufferDescriptor_free(bd);
        puts("Failed to parse");
        return 1;
    }
    BufferDescriptor_free(bd);
    return 0;
}
