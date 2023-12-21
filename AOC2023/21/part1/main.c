#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

#define START   'S'
#define GARDEN  '.'
#define ROCK    '#'

#define NUM_STEPS 64

#define INFINITY            UINT32_MAX
#define MIN_CAHRBUFFER2D    16
#define MIN_QUEUE           8

struct QueueItem {
    uint32_t distance;
    uint32_t vertex;
};

struct PriorityQueue {
    struct QueueItem *items;
    uint32_t length;
    uint32_t capacity;
};

struct AdjListNode {
    struct AdjListNode *next;
    uint32_t vertex;
};

struct AdjacencyList {
    struct AdjListNode **vertices;
    uint32_t num_vertices;
    uint32_t start_vertex;
};

struct CharBuffer2D {
    char *buffer;
    uint32_t length;
    uint32_t capacity;
    uint32_t num_lines;
    uint32_t num_cols;
};

struct PriorityQueue *PriorityQueue_create(uint32_t start_capacity)
{
    struct PriorityQueue *queue;
    queue = malloc(sizeof(*queue));
    if (!queue) {
        perror("malloc");
        puts("Failed to create PriorityQueue");
        return NULL;
    }
    if (start_capacity < MIN_QUEUE)
        start_capacity = MIN_QUEUE;
    queue->items = malloc(start_capacity * sizeof(*(queue->items)));
    if (!queue->items) {
        perror("malloc");
        puts("Failed to initialize PriorityQueue");
        free(queue);
        return NULL;
    }
    queue->capacity = start_capacity;
    queue->length = 0;
    return queue;
}

void PriorityQueue_free(struct PriorityQueue *queue)
{
    free(queue->items);
    free(queue);
}

int PriorityQueue_grow(struct PriorityQueue *queue)
{
    struct QueueItem *temp;
    uint32_t new_capacity;
    new_capacity = 2 * queue->capacity;
    temp = realloc(queue->items, new_capacity * sizeof(*(queue->items)));
    if (!temp) {
        perror("realloc");
        puts("Failed to grow PriorityQueue");
        return 0;
    }
    queue->items = temp;
    queue->capacity = new_capacity;
    return 1;
}

int PriorityQueue_shrink(struct PriorityQueue *queue)
{
    struct QueueItem *temp;
    uint32_t new_capacity;

    new_capacity = queue->capacity >> 1;
    if (new_capacity < MIN_QUEUE)
        new_capacity = MIN_QUEUE;

    temp = realloc(queue->items, new_capacity * sizeof(*(queue->items)));
    if (!temp) {
        perror("realloc");
        puts("Failed to shrink PriorityQueue");
        return 0;
    }
    queue->items = temp;
    queue->capacity = new_capacity;
    return 1;
}

void QueueItem_swap(struct QueueItem *a, struct QueueItem *b)
{
    struct QueueItem temp;
    temp = *b;
    *b = *a;
    *a = temp;
}

void PriorityQueue_heapify(struct PriorityQueue *queue, uint32_t index)
{
    if (queue->length == 1) return;
    uint32_t smallest, left, right;
    smallest = index;
    left = 2 * index + 1;
    right = left + 1;
    if (
        left < queue->length
        && queue->items[left].distance < queue->items[smallest].distance
    ) smallest = left;
    if (
        right < queue->length
        && queue->items[right].distance < queue->items[smallest].distance
    ) smallest = right;
    if (smallest != index) {
        QueueItem_swap(&(queue->items[smallest]), &(queue->items[index]));
        PriorityQueue_heapify(queue, smallest);
    }
}

int PriorityQueue_enqueue(struct PriorityQueue *queue, struct QueueItem item)
{
    uint32_t i;
    if (queue->length == 0) {
        queue->items[0] = item;
        ++(queue->length);
    } else {
        if (queue->length + 1 > queue->capacity)
            if (!PriorityQueue_grow(queue)) return 0;
        queue->items[queue->length] = item;
        ++(queue->length);
        for (i = queue->length / 2; i > 0; --i)
            PriorityQueue_heapify(queue, i - 1);
    }
    return 1;
}

int PriorityQueue_dequeue(struct PriorityQueue *queue, struct QueueItem *dest)
{
    uint32_t i;
    if (queue->length == 0) return 0;
    *dest = queue->items[0];
    QueueItem_swap(&(queue->items[0]), &(queue->items[queue->length - 1]));
    --(queue->length);
    if (queue->length < queue->capacity / 4 && queue->capacity > MIN_QUEUE)
        if (!PriorityQueue_shrink(queue)) return 0;
    for (i = queue->length / 2; i > 0; --i)
        PriorityQueue_heapify(queue, i - 1);
    return 1;
}

struct CharBuffer2D *CharBuffer2D_create(uint32_t start_capacity)
{
    struct CharBuffer2D *cb;
    if (start_capacity < MIN_CAHRBUFFER2D)
        start_capacity = MIN_CAHRBUFFER2D;
    cb = malloc(sizeof(*cb));
    if (!cb) {
        perror("malloc");
        puts("Failed to allocate CharBuffer2D");
        return NULL;
    }
    *cb = (struct CharBuffer2D) {
        .buffer = malloc(start_capacity * sizeof(*(cb->buffer))),
        .length = 0,
        .capacity = start_capacity,
        .num_lines = 0,
        .num_cols = 0
    };
    if (!cb->buffer) {
        perror("malloc");
        puts("Failed to allocate CharBuffer2D->buffer");
        free(cb);
        return NULL;
    }
    return cb;
}

void CharBuffer2D_free(struct CharBuffer2D *cb)
{
    free(cb->buffer);
    free(cb);
}

int CharBuffer2D_grow(struct CharBuffer2D *cb)
{
    uint32_t new_capacity;
    char *temp;
    new_capacity = cb->capacity << 1;
    temp = realloc(cb->buffer, new_capacity * sizeof(*(cb->buffer)));
    if (!temp) {
        perror("realloc");
        puts("Failed to grow CharBuffer2D");
        return 0;
    }
    cb->buffer = temp;
    cb->capacity = new_capacity;
    return 1;
}

uint32_t CharBuffer2D_load_line(struct CharBuffer2D *cb)
{
    uint32_t i;
    int c;
    for (
        i = 0;
        (c = getchar()) != EOF
        && c != '\n';
        ++i
    ) {
        if (cb->length + i >= cb->capacity) {
            if (!CharBuffer2D_grow(cb)) {
                cb->buffer[cb->length] = '\0';
                return 0;
            }
        }
        cb->buffer[cb->length + i] = c;
    }
    if (cb->length + i >= cb->capacity) {
        if (!CharBuffer2D_grow(cb)) {
            cb->buffer[cb->length] = '\0';
            return 0;
        }
    }
    if (!cb->num_cols)
        cb->num_cols = i;
    else if (i && i != cb->num_cols) {
        puts("Got inconsistent line length");
        cb->buffer[cb->length] = 0;
        return 0;
    }
    if (i)
        ++(cb->num_lines);
    cb->length = cb->length + i;
    cb->buffer[cb->length] = '\0';
    return i;
}

int CharBuffer2D_load(struct CharBuffer2D *cb)
{
    while (CharBuffer2D_load_line(cb));
    return 1;
}

char CharBuffer2D_at(struct CharBuffer2D *cb, uint32_t line, uint32_t col)
{
    if (line >= cb->num_lines || col >= cb->num_cols) {
        puts("Index out of bounds");
        return '\0';
    }
    return cb->buffer[line * cb->num_cols + col];
}

struct AdjListNode *AdjListNode_create(uint32_t vertex)
{
    struct AdjListNode *node;
    node = malloc(sizeof(*node));
    if (!node) {
        perror("malloc");
        puts("Failed to allocate AdjListNode");
        return NULL;
    }
    node->vertex = vertex;
    node->next = NULL;
    return node;
}

struct AdjListNode *AdjListNode_prepend(
    struct AdjListNode *node, uint32_t vertex
)
{
    struct AdjListNode *new;
    new = AdjListNode_create(vertex);
    if (!new) return NULL;
    new->next = node;
    return new;
}

void AdjListNode_free(struct AdjListNode *node)
{
    if (!node) return;
    struct AdjListNode *next;
    next = node->next;
    free(node);
    AdjListNode_free(next);
}

struct AdjacencyList *AdjacencyList_create(uint32_t num_vertices)
{
    uint32_t i;
    struct AdjacencyList *adj_list;
    adj_list = malloc(sizeof(*adj_list));
    if (!adj_list) {
        perror("malloc");
        puts("Failed to allocate AdjacencyList");
        return NULL;
    }
    *adj_list = (struct AdjacencyList) {
        .vertices = malloc(num_vertices * sizeof(*(adj_list->vertices))),
        .num_vertices = num_vertices,
        .start_vertex = 0
    };
    if (!adj_list->vertices) {
        perror("malloc");
        puts("Failed to allocate AdjacencyList->vertices");
        free(adj_list);
        return NULL;
    }
    for (i = 0; i < num_vertices; ++i)
        adj_list->vertices[i] = NULL;
    return adj_list;
}

void AdjacencyList_free(struct AdjacencyList *adj_list)
{
    uint32_t i;
    for (i = 0; i < adj_list->num_vertices; ++i)
        AdjListNode_free(adj_list->vertices[i]);
    free(adj_list->vertices);
    free(adj_list);
}

int AdjacencyList_add_connection(
    struct AdjacencyList* adj_list, uint32_t from, uint32_t to
)
{
    struct AdjListNode *node;
    node = AdjListNode_prepend(adj_list->vertices[from], to);
    if (!node) return 0;
    adj_list->vertices[from] = node;
    return 1;
}

struct AdjacencyList *gen_adj_list(void)
{
    struct CharBuffer2D *cb;
    char type;
    struct AdjacencyList *adj_list;
    uint32_t line, col;
    uint32_t vertex;
    cb = CharBuffer2D_create(0);
    if (!cb) goto error;
    if (!CharBuffer2D_load(cb)) goto free_cb;
    adj_list = AdjacencyList_create(cb->num_lines * cb->num_cols);
    if (!adj_list) goto free_cb;
    for (line = 0; line < cb->num_lines; ++line) {
        for (col = 0; col < cb->num_cols; ++col) {
            type = CharBuffer2D_at(cb, line, col);
            vertex = line * cb->num_cols + col;
            if (type == ROCK)
                continue;
            if (type == START)
                adj_list->start_vertex = vertex;

            /* North */
            if (
                line
                && CharBuffer2D_at(cb, line - 1, col) != ROCK
                && !AdjacencyList_add_connection(
                    adj_list, vertex, (line - 1) * cb->num_cols + col
                )
            ) goto free_adj_list;

            /* South */
            if (
                line < cb->num_lines - 1
                && CharBuffer2D_at(cb, line + 1, col) != ROCK
                && !AdjacencyList_add_connection(
                    adj_list, vertex, (line + 1) * cb->num_cols + col
                )
            ) goto free_adj_list;

            /* West */
            if (
                col
                && CharBuffer2D_at(cb, line, col - 1) != ROCK
                && !AdjacencyList_add_connection(
                    adj_list, vertex, line * cb->num_cols + (col - 1)
                )
            ) goto free_adj_list;

            /* East */
            if (
                col < cb->num_cols - 1
                && CharBuffer2D_at(cb, line, col + 1) != ROCK
                && !AdjacencyList_add_connection(
                    adj_list, vertex, line * cb->num_cols + (col + 1)
                )
            ) goto free_adj_list;
        }
    }
    CharBuffer2D_free(cb);
    return adj_list;

free_adj_list:
    AdjacencyList_free(adj_list);
free_cb:
    CharBuffer2D_free(cb);
error:
    return NULL;
}

uint32_t *dijkstra(struct AdjacencyList *adj_list)
{
    struct PriorityQueue *queue;
    struct AdjListNode *node;
    struct QueueItem item;
    uint32_t vertex, new_distance, *distances;
    int result;

    distances = malloc(adj_list->num_vertices * sizeof(*distances));
    if (!distances) goto error_create_distances;
    for (vertex = 0; vertex < adj_list->num_vertices; ++vertex)
        distances[vertex] = INFINITY;

    queue = PriorityQueue_create(adj_list->num_vertices / 2);
    if (!queue) goto error_create_queue;

    item = (struct QueueItem) {
        .distance = 0,
        .vertex = adj_list->start_vertex
    };

    result = PriorityQueue_enqueue(queue, item);
    if (!result) goto error_enqueue;

    while (PriorityQueue_dequeue(queue, &item)) {
        vertex = item.vertex;
        node = adj_list->vertices[vertex];

        while (node) {
            new_distance = item.distance + 1;
            if (new_distance < distances[node->vertex]) {
                distances[node->vertex] = new_distance;
                result = PriorityQueue_enqueue(
                    queue,
                    (struct QueueItem) {
                        .distance = new_distance,
                        .vertex = node->vertex
                    }
                );
                if (!result) goto error_enqueue;
            }
            node = node->next;
        }
    }
    PriorityQueue_free(queue);
    return distances;

error_create_distances:
    perror("malloc");
    puts("Failed to allocate distances array");
    goto error;
error_create_queue:
    puts("Failed to create PriorityQueue");
    goto free_distances;
error_enqueue:
    puts("Failed to enqueue item");
    goto free_queue;
free_queue:
    PriorityQueue_free(queue);
free_distances:
    free(distances);
error:
    return NULL;
}

int run(void)
{
    struct AdjacencyList *adj_list;
    uint32_t i, *distances, count;
    adj_list = gen_adj_list();
    if (!adj_list) return 0;
    distances = dijkstra(adj_list);
    if (!distances) return 0;
    for (i = count = 0; i < adj_list->num_vertices; ++i)
        if (
            distances[i] == NUM_STEPS
            || (
                distances[i] < NUM_STEPS
                && distances[i] % 2 == NUM_STEPS % 2
            )
        )
            ++count;
    printf("Reachable = %u\n", count);
    free(distances);
    AdjacencyList_free(adj_list);
    return 1;
}

int main(void)
{
    if (!run()) return 1;
    return 0;
}
