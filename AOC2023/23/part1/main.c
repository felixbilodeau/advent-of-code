#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>

#define MIN_CHARBUFF2D  128U
#define MIN_ADJLIST     16U
#define MIN_STACK       8U

#define NORTH   'N'
#define SOUTH   'S'
#define EAST    'E'
#define WEST    'W'

#define PATH    '.'
#define FOREST  '#'
#define SLOPE_N '^'
#define SLOPE_S 'v'
#define SLOPE_E '>'
#define SLOPE_W '<'

struct StackElement {
    uint32_t line;
    uint32_t col;
    uint32_t distance;
    uint32_t last_vertex;
    char last_direction;
};

struct Stack {
    struct StackElement *stack;
    uint32_t length;
    uint32_t capacity;
};

struct CharBuffer2D {
    char *buffer;
    uint32_t length;
    uint32_t capacity;
    uint32_t num_lines;
    uint32_t num_cols;
};

struct Edge {
    struct Edge *next;
    uint32_t to_vertex;
    uint32_t weight;
};

struct Vertex {
    struct Edge *edges;
    uint32_t line;
    uint32_t col;
};

struct AdjacencyList {
    struct Vertex *vertices;
    uint32_t num_vertices;
    uint32_t capacity;
    uint32_t end_vertex;
};

struct Stack *Stack_create(uint32_t start_capacity)
{
    struct Stack *stack;
    if (start_capacity < MIN_STACK)
        start_capacity = MIN_STACK;
    stack = malloc(sizeof(*stack));
    assert(stack);
    *stack = (struct Stack) {
        .stack = malloc(start_capacity * sizeof(*(stack->stack))),
        .length = 0,
        .capacity = start_capacity
    };
    assert(stack->stack);
    return stack;
}

void Stack_free(struct Stack *stack)
{
    if (!stack) return;
    free(stack->stack);
    free(stack);
}

int Stack_grow(struct Stack *stack)
{
    uint32_t new_capacity;
    struct StackElement *temp;
    new_capacity = stack->capacity << 1;
    temp = realloc(stack->stack, new_capacity * sizeof(*(stack->stack)));
    if (!temp) {
        perror("realloc");
        puts("Failed to grow Stack");
        return 0;
    }
    stack->stack = temp;
    stack->capacity = new_capacity;
    return 1;
}

int Stack_shrink(struct Stack *stack)
{
    uint32_t new_capacity;
    struct StackElement *temp;
    new_capacity = stack->capacity >> 1;
    if (new_capacity < MIN_STACK)
        new_capacity = MIN_STACK;
    temp = realloc(stack->stack, new_capacity * sizeof(*(stack->stack)));
    if (!temp) {
        perror("realloc");
        puts("Failed to shrink Stack");
        return 0;
    }
    stack->stack = temp;
    stack->capacity = new_capacity;
    return 1;
}

int Stack_push(struct Stack *stack, struct StackElement element)
{
    if (stack->length + 1 >= stack->capacity)
        if (!Stack_grow(stack))
            return 0;
    stack->stack[(stack->length)++] = element;
    return 1;
}

int Stack_pop(struct Stack *stack, struct StackElement *dest)
{
    if (!stack->length)
        return -1;
    if (stack->length - 1 <= stack->capacity >> 2)
        if (!Stack_shrink(stack))
            return 0;
    *dest = stack->stack[--(stack->length)];
    return 1;
}

void Edge_free(struct Edge *edge)
{
    struct Edge *next;
    if (!edge) return;
    next = edge->next;
    free(edge);
    Edge_free(next);
}

struct AdjacencyList *AdjacencyList_create(uint32_t start_capacity)
{
    struct AdjacencyList *adj;
    if (start_capacity < MIN_ADJLIST)
        start_capacity = MIN_ADJLIST;
    adj = malloc(sizeof(*adj));
    assert(adj);
    *adj = (struct AdjacencyList) {
        .vertices = malloc(start_capacity * sizeof(*(adj->vertices))),
        .num_vertices = 0,
        .capacity = start_capacity,
        .end_vertex = 0
    };
    assert(adj->vertices);
    return adj;
}

void AdjacencyList_free(struct AdjacencyList *adj)
{
    if (!adj) return;
    uint32_t i;
    for (i = 0; i < adj->num_vertices; ++i)
        Edge_free(adj->vertices[i].edges);
    free(adj->vertices);
    free(adj);
}

int AdjacencyList_grow(struct AdjacencyList *adj)
{
    uint32_t new_capacity;
    struct Vertex *temp;
    new_capacity = adj->capacity << 1;
    temp = realloc(adj->vertices, new_capacity *sizeof(*(adj->vertices)));
    if (!temp) {
        perror("realloc");
        puts("Failed to grow AdjacencyList");
        return 0;
    }
    adj->vertices = temp;
    adj->capacity = new_capacity;
    return 1;
}

int AdjacencyList_add_vertex(
    struct AdjacencyList *adj, uint32_t line, uint32_t col
)
{
    if (adj->num_vertices + 1 >= adj->capacity)
        if (!AdjacencyList_grow(adj))
            return 0;
    adj->vertices[adj->num_vertices] = (struct Vertex) {
        .edges = NULL,
        .line = line,
        .col = col
    };
    ++(adj->num_vertices);
    return 1;
}

uint32_t AdjacencyList_add_connection(
    struct AdjacencyList *adj,
    uint32_t from,
    uint32_t to_line, uint32_t to_col,
    uint32_t weight
)
{
    uint32_t to;
    struct Edge *edge;
    for (to = 0; to < adj->num_vertices; ++to) {
        if (
            adj->vertices[to].line == to_line
            && adj->vertices[to].col == to_col
        )
            break;
    }
    if (to == adj->num_vertices) {
        if (!AdjacencyList_add_vertex(adj, to_line, to_col)) {
            puts("Failed to add Vertex");
            return 0;
        }
    }
    edge = adj->vertices[from].edges;
    while (edge) {
        if (edge->to_vertex == to && edge->weight == weight)
            return to;
        edge = edge->next;
    }
    edge = malloc(sizeof(*edge));
    if (!edge) {
        perror("malloc");
        puts("Failed to allocate Edge");
        return 0;
    }
    *edge = (struct Edge) {
        .next = adj->vertices[from].edges,
        .to_vertex = to,
        .weight = weight
    };
    adj->vertices[from].edges = edge;
    return to;
}

struct CharBuffer2D *CharBuffer2D_create(uint32_t start_capacity)
{
    struct CharBuffer2D *buff2d;
    if (start_capacity < MIN_CHARBUFF2D)
        start_capacity = MIN_CHARBUFF2D;
    buff2d = malloc(sizeof(*buff2d));
    assert(buff2d);
    *buff2d = (struct CharBuffer2D) {
        .buffer = malloc(start_capacity * sizeof(*(buff2d->buffer))),
        .length = 0,
        .capacity = start_capacity,
        .num_lines = 0,
        .num_cols = 0
    };
    assert(buff2d->buffer);
    return buff2d;
}

void CharBuffer2D_free(struct CharBuffer2D *buff2d)
{
    if (!buff2d) return;
    free(buff2d->buffer);
    free(buff2d);
}

int CharBuffer2D_grow(struct CharBuffer2D *buff2d)
{
    uint32_t new_capacity;
    char *temp;
    new_capacity = buff2d->capacity << 1;
    temp = realloc(buff2d->buffer, new_capacity * sizeof(*(buff2d->buffer)));
    if (!temp) {
        perror("relloc");
        puts("Failed to grow CharBuffer2D");
        return 0;
    }
    buff2d->buffer = temp;
    buff2d->capacity = new_capacity;
    return 1;
}

char CharBuffer2D_at(struct CharBuffer2D *buff2d, uint32_t line, uint32_t col)
{
    assert(line < buff2d->num_lines && col < buff2d->num_cols);
    return buff2d->buffer[line * buff2d->num_cols + col];
}

int CharBuffer2D_load_line(struct CharBuffer2D *buff2d)
{
    int c;
    uint32_t i;

    for (i = 0; (c = getchar()) != EOF && c != '\n'; ++i) {
        if (buff2d->length + i + 1 >= buff2d->capacity)
            if (!CharBuffer2D_grow(buff2d))
                return 0;
        buff2d->buffer[buff2d->length + i] = c;
    }
    if (!buff2d->num_cols) {
        buff2d->num_cols = i;
    } else if (!i) {
        return 0;
    } else if (i != buff2d->num_cols) {
        puts("Incompatible number of columns");
        return 0;
    }
    buff2d->length = buff2d->length + i;
    ++(buff2d->num_lines);
    return 1;
}

struct CharBuffer2D *CharBuffer2D_load(void)
{
    struct CharBuffer2D *buff2d;
    buff2d = CharBuffer2D_create(0);
    while (CharBuffer2D_load_line(buff2d));
    return buff2d;
}

int walk_buffer(
    struct AdjacencyList *adj,
    struct CharBuffer2D *buff2d,
    struct Stack *stack
)
{
    struct StackElement element;
    uint32_t num_outs, last_vertex, distance;
    char terrain_n, terrain_s, terrain_e, terrain_w;
    int is_out_n, is_out_s, is_out_e, is_out_w;
    int result;

    result = Stack_pop(stack, &element);
    if (result == -1) return 1;
    else if (result == 0) return 0;

    terrain_n = terrain_s = terrain_e = terrain_w = 0;
    if (element.last_direction != SOUTH && element.line) {
        terrain_n = CharBuffer2D_at(
            buff2d, element.line - 1, element.col
        );
    }
    if (
        element.last_direction != NORTH
        && element.line < buff2d->num_lines - 1
    ) {
        terrain_s = CharBuffer2D_at(
            buff2d, element.line + 1, element.col
        );
    }
    if (
        element.last_direction != WEST
        && element.col < buff2d->num_cols - 1
    ) {
        terrain_e = CharBuffer2D_at(
            buff2d, element.line, element.col + 1
        );
    }
    if (element.last_direction != EAST && element.col) {
        terrain_w = CharBuffer2D_at(
            buff2d, element.line, element.col - 1
        );
    }
    is_out_n = terrain_n == PATH || terrain_n == SLOPE_N;
    is_out_s = terrain_s == PATH || terrain_s == SLOPE_S;
    is_out_e = terrain_e == PATH || terrain_e == SLOPE_E;
    is_out_w = terrain_w == PATH || terrain_w == SLOPE_W;
    num_outs = 0;
    num_outs = is_out_n + is_out_s + is_out_e + is_out_w;
    if (num_outs > 1 || num_outs == 0) {
        for (
            last_vertex = 0;
            last_vertex < adj->num_vertices;
            ++last_vertex
        ) {
            if (
                adj->vertices[last_vertex].line == element.line
                && adj->vertices[last_vertex].col == element.col
            )
                break;
        }
        if (
            !(
                last_vertex = AdjacencyList_add_connection(
                    adj, element.last_vertex, element.line,
                    element.col, element.distance
                )
            )
        ) {
            puts("Failed to add connection");
            return 0;
        }
        distance = 0;
        if (num_outs == 0 && !adj->end_vertex)
            adj->end_vertex = last_vertex;
    } else {
        last_vertex = element.last_vertex;
        distance = element.distance;
    }
    if (is_out_n) {
        if (
            !Stack_push(
                stack,
                (struct StackElement) {
                    .line = element.line - 1,
                    .col = element.col,
                    .distance = distance + 1,
                    .last_vertex = last_vertex,
                    .last_direction = NORTH
                }
            )
        ) {
            puts("Failed to push to stack");
            return 0;
        }
    }
    if (is_out_s) {
        if (
            !Stack_push(
                stack,
                (struct StackElement) {
                    .line = element.line + 1,
                    .col = element.col,
                    .distance = distance + 1,
                    .last_vertex = last_vertex,
                    .last_direction = SOUTH
                }
            )
        ) {
            puts("Failed to push to stack");
            return 0;
        }
    }
    if (is_out_e) {
        if (
            !Stack_push(
                stack,
                (struct StackElement) {
                    .line = element.line,
                    .col = element.col + 1,
                    distance = distance + 1,
                    .last_vertex = last_vertex,
                    .last_direction = EAST
                }
            )
        ) {
            puts("Failed to push to stack");
            return 0;
        }
    }
    if (is_out_w) {
        if (
            !Stack_push(
                stack,
                (struct StackElement) {
                    .line = element.line,
                    .col = element.col - 1,
                    distance = distance + 1,
                    .last_vertex = last_vertex,
                    .last_direction = WEST
                }
            )
        ) {
            puts("Failed to push to stack");
            return 0;
        }
    }
    return walk_buffer(adj, buff2d, stack);
}

struct AdjacencyList *from_buffer(void)
{
    uint32_t line, col;
    struct AdjacencyList *adj;
    struct CharBuffer2D *buff2d;
    struct Stack *stack;
    buff2d = CharBuffer2D_load();
    adj = AdjacencyList_create(0);
    for (line = col = 0; col < buff2d->num_cols; ++col) {
        if (CharBuffer2D_at(buff2d, line, col) == PATH) {
            if (!AdjacencyList_add_vertex(adj, line, col)) {
                AdjacencyList_free(adj);
                CharBuffer2D_free(buff2d);
                return NULL;
            }
            break;
        }
    }
    if (col == buff2d->num_cols) {
        puts("Did not find start vertex");
        AdjacencyList_free(adj);
        CharBuffer2D_free(buff2d);
        return NULL;
    }
    stack = Stack_create(0);
    if (
        !Stack_push(
            stack, (struct StackElement) {
                .line = 0,
                .col = col,
                .distance = 0,
                .last_vertex = 0,
                .last_direction = '\0'
            }
        )
    ) {
        Stack_free(stack);
        AdjacencyList_free(adj);
        CharBuffer2D_free(buff2d);
        return NULL;
    }
    if (!walk_buffer(adj, buff2d, stack)) {
        puts("Failed to generate AdjacencyList");
        Stack_free(stack);
        AdjacencyList_free(adj);
        CharBuffer2D_free(buff2d);
        return NULL;
    }
    Stack_free(stack);
    CharBuffer2D_free(buff2d);
    return adj;
}

int dfs(
    struct AdjacencyList *adj,
    struct Stack *stack,
    uint32_t *max_distance
)
{
    uint32_t vertex;
    struct StackElement element;
    struct Edge *edge;
    int result;

    result = Stack_pop(stack, &element);
    if (result == -1) return 1;
    else if (result == 0) return 0;

    vertex = element.last_vertex;
    if (vertex == adj->end_vertex && element.distance > *max_distance) {
        *max_distance = element.distance;
        return dfs(adj, stack, max_distance);
    }

    edge = adj->vertices[vertex].edges;
    while (edge) {
        if (
            !Stack_push(
                stack, (struct StackElement) {
                    .line = element.line,
                    .col = element.col,
                    .last_vertex = edge->to_vertex,
                    .distance = element.distance + edge->weight,
                    .last_direction = '\0'
                }
            )
        )
            return 0;
        edge = edge->next;
    }
    return dfs(adj, stack, max_distance);
}

int main(void)
{
    struct AdjacencyList *adj;
    struct Stack *stack;
    uint32_t max_distance;
    adj = from_buffer();
    if (!adj) return 1;
    stack = Stack_create(0);
    if (
        !Stack_push(
            stack, (struct StackElement) {
                .line = adj->vertices[0].line,
                .col = adj->vertices[0].col,
                .distance = 0,
                .last_vertex = 0,
                .last_direction = '\0'
            }
        )
    ) {
        Stack_free(stack);
        AdjacencyList_free(adj);
        return 1;
    }
    max_distance = 0;
    if (!dfs(adj, stack, &max_distance)) {
        Stack_free(stack);
        AdjacencyList_free(adj);
        return 1;
    }
    printf("Max distance = %u\n", max_distance);
    Stack_free(stack);
    AdjacencyList_free(adj);
    return 0;
}
