#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define MIN_CHARBUFF2D  128U
#define MIN_ADJLIST     16U
#define MIN_U32DYNARRAY 8U
#define MIN_STACK       8U

#define QUEUE_EMPTY 0

#define NORTH   'N'
#define SOUTH   'S'
#define EAST    'E'
#define WEST    'W'

#define PATH    '.'
#define FOREST  '#'
#define VERTEX  'X'

struct U32DynArray {
    uint32_t *buffer;
    uint32_t length;
    uint32_t capacity;
};

struct QueueElement {
    struct QueueElement *next;
    struct U32DynArray *visited;
    uint32_t vertex;
    uint32_t distance;
};

struct Queue {
    struct QueueElement *head;
    struct QueueElement *tail;
};

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

struct U32DynArray *U32DynArray_create(uint32_t start_capacity)
{
    struct U32DynArray *array;
    if (start_capacity < MIN_U32DYNARRAY)
        start_capacity = MIN_U32DYNARRAY;
    array = malloc(sizeof(*array));
    assert(array);
    *array = (struct U32DynArray) {
        .buffer = malloc(start_capacity * sizeof(*(array->buffer))),
        .length = 0,
        .capacity = start_capacity
    };
    assert(array->buffer);
    return array;
}

void U32DynArray_free(struct U32DynArray *array)
{
    if (!array) return;
    free(array->buffer);
    free(array);
}

void U32DynArray_grow(struct U32DynArray *array)
{
    uint32_t new_capacity;
    uint32_t *temp;
    new_capacity = array->capacity << 1;
    temp = realloc(array->buffer, new_capacity * sizeof(*(array->buffer)));
    assert(temp);
    array->buffer = temp;
    array->capacity = new_capacity;
}

void U32DynArray_insert(struct U32DynArray *array, uint32_t value)
{
    if (array->length + 1 >= array->capacity)
        U32DynArray_grow(array);
    array->buffer[(array->length)++] = value;
}

struct U32DynArray *U32DynArray_copy(struct U32DynArray *array)
{
    struct U32DynArray *copy;
    copy = U32DynArray_create(array->capacity);
    copy->length = array->length;
    memcpy(
        copy->buffer, array->buffer, copy->length * sizeof(*(copy->buffer))
    );
    return copy;
}

int U32DynArray_contains(struct U32DynArray *array, uint32_t value)
{
    uint32_t i;
    for (i = 0; i < array->length; ++i)
        if (array->buffer[i] == value) return 1;
    return 0;
}

void QueueElement_free(struct QueueElement *node)
{
    struct QueueElement *next;
    if (!node) return;
    next = node->next;
    U32DynArray_free(node->visited);
    free(node);
    QueueElement_free(next);
}

struct Queue* Queue_create(void)
{
    struct Queue *queue;
    queue = malloc(sizeof(*queue));
    assert(queue);
    *queue = (struct Queue) {
        .head =  NULL,
        .tail = NULL
    };
    return queue;
}

void Queue_free(struct Queue *queue)
{
    if (!queue) return;
    QueueElement_free(queue->head);
    free(queue);
}

void Queue_enqueue(struct Queue *queue, struct QueueElement data)
{
    struct QueueElement *node;
    node = malloc(sizeof(*node));
    assert(node);
    *node = data;
    node->next = NULL;
    if (!queue->tail) {
        queue->head = queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
}

int Queue_dequeue(struct Queue *queue, struct QueueElement *dest)
{
    struct QueueElement *old_head;
    if (!queue->head) return QUEUE_EMPTY;
    *dest = *(queue->head);
    dest->next = NULL;
    old_head = queue->head;
    queue->head = queue->head->next;
    if (!queue->head) queue->tail = NULL;
    free(old_head);
    return 1;
}

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
        if (edge->to_vertex == to && edge->weight == weight) {
            return to;
        }
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

void AdjacencyList_connect(
    struct AdjacencyList *adj, uint32_t from, uint32_t to, uint32_t weight
)
{
    struct Edge *edge;
    edge = adj->vertices[from].edges;
    while (edge) {
        if (edge->to_vertex == to && edge->weight == weight) {
            return;
        }
        edge = edge->next;
    }
    edge = malloc(sizeof(*edge));
    assert(edge);
    *edge = (struct Edge) {
        .next = adj->vertices[from].edges,
        .to_vertex = to,
        .weight = weight
    };
    adj->vertices[from].edges = edge;
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

void CharBuffer2D_set_at(
    struct CharBuffer2D *buff2d, uint32_t line, uint32_t col, char to
)
{
    assert(line < buff2d->num_lines && col < buff2d->num_cols);
    buff2d->buffer[line * buff2d->num_cols + col] = to;
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

void CharBuffer2D_print(struct CharBuffer2D *buff2d)
{
    uint32_t line, col;
    for (line = 0; line < buff2d->num_lines; ++line) {
        for (col = 0; col < buff2d->num_cols; ++col)
            putchar(CharBuffer2D_at(buff2d, line, col));
        putchar('\n');
    }
}

int walk_buffer(
    struct AdjacencyList *adj,
    struct CharBuffer2D *buff2d,
    struct Stack *stack
)
{
    struct StackElement element;
    uint32_t num_outs, last_vertex, distance;
    char terrain, terrain_n, terrain_s, terrain_e, terrain_w;
    int is_out_n, is_out_s, is_out_e, is_out_w;
    int result;

    result = Stack_pop(stack, &element);
    if (result == -1) return 1;
    else if (result == 0) return 0;

    terrain = CharBuffer2D_at(buff2d, element.line, element.col);
    if (terrain == FOREST)
        return walk_buffer(adj, buff2d, stack);

    terrain_n = terrain_s = terrain_e = terrain_w = FOREST;
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
    is_out_n = terrain_n != FOREST && element.last_direction != SOUTH;
    is_out_s = terrain_s != FOREST && element.last_direction != NORTH;
    is_out_e = terrain_e != FOREST && element.last_direction != WEST;
    is_out_w = terrain_w != FOREST && element.last_direction != EAST;
    num_outs = 0;
    num_outs = is_out_n + is_out_s + is_out_e + is_out_w;
    if (num_outs > 1 || num_outs == 0 || terrain == VERTEX) {
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
        last_vertex = AdjacencyList_add_connection(
            adj, element.last_vertex, element.line,
            element.col, element.distance
        );
        AdjacencyList_connect(
            adj, last_vertex, element.last_vertex, element.distance
        );
        distance = 0;
        CharBuffer2D_set_at(buff2d, element.line, element.col, VERTEX);
        if (element.line == buff2d->num_lines - 1 && !adj->end_vertex)
            adj->end_vertex = last_vertex;
    } else {
        CharBuffer2D_set_at(buff2d, element.line, element.col, FOREST);
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
            CharBuffer2D_set_at(buff2d, line, col, VERTEX);
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
                .line = 1,
                .col = col,
                .distance = 1,
                .last_vertex = 0,
                .last_direction = SOUTH
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

int bfs(
    struct AdjacencyList *adj, struct Queue *queue, uint32_t *max_distance
)
{
    struct QueueElement element;
    struct U32DynArray *visited;
    struct Edge *edge;

    if (Queue_dequeue(queue, &element) == QUEUE_EMPTY)
        return 1;

    if (element.vertex == adj->end_vertex) {
        if (element.distance > *max_distance)
            *max_distance = element.distance;
        U32DynArray_free(element.visited);
        return bfs(adj, queue, max_distance);
    }

    edge = adj->vertices[element.vertex].edges;
    while (edge) {
        if (!U32DynArray_contains(element.visited, edge->to_vertex)) {
            visited = U32DynArray_copy(element.visited);
            U32DynArray_insert(visited, element.vertex);
            Queue_enqueue(
                queue,
                (struct QueueElement) {
                    .vertex = edge->to_vertex,
                    .visited = visited,
                    .distance = element.distance + edge->weight
                }
            );
        }
        edge = edge->next;
    }
    U32DynArray_free(element.visited);
    return bfs(adj, queue, max_distance);
}

int main(void)
{
    struct AdjacencyList *adj;
    struct Queue *queue;
    struct U32DynArray *visited;
    uint32_t max_distance;
    adj = from_buffer();
    if (!adj) return 1;
    visited = U32DynArray_create(0);
    max_distance = 0;
    queue = Queue_create();
    Queue_enqueue(
        queue,
        (struct QueueElement){
            .distance = 0, .vertex = 0, .visited = visited
        }
    );
    bfs(adj, queue, &max_distance);
    printf("Max Distance = %u\n", max_distance);
    Queue_free(queue);
    AdjacencyList_free(adj);
    return 0;
}
