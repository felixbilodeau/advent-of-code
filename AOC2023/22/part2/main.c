#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>
#include <string.h>

#define MIN_CHARBUFFER  32U
#define MIN_BRICKARRAY  4U

struct CharBuffer {
    char *buffer;
    uint32_t length;
    uint32_t capacity;
};

struct vec3 {
    uint32_t x;
    uint32_t y;
    uint32_t z;
};

struct Brick {
    struct vec3 start;
    struct vec3 end;
};

struct BrickArray {
    struct Brick *bricks;
    uint32_t length;
    uint32_t capacity;
};

struct QueueNode {
    struct QueueNode *next;
    uint32_t index;
};

struct Queue {
    struct QueueNode *head;
    struct QueueNode *tail;
};

uint32_t min(uint32_t number, uint32_t other)
{
    return (number < other) * number + !(number < other) * other;
}

uint32_t max(uint32_t number, uint32_t other)
{
    return (number > other) * number + !(number > other) * other;
}

void QueueNode_free(struct QueueNode *node)
{
    if (!node) return;
    struct QueueNode *next;
    next = node->next;
    free(node);
    QueueNode_free(next);
}

struct Queue *Queue_create(void)
{
    struct Queue *queue;
    queue = malloc(sizeof(*queue));
    assert(queue);
    *queue = (struct Queue) {
        .head = NULL,
        .tail = NULL
    };
    return queue;
}

void Queue_free(struct Queue *queue)
{
    if (!queue) return;
    QueueNode_free(queue->head);
    free(queue);
}

void Queue_enqueue(struct Queue *queue, uint32_t index)
{
    struct QueueNode *node;
    node = malloc(sizeof(*node));
    assert(node);
    *node = (struct QueueNode) {
        .next = NULL,
        .index = index
    };
    if (!queue->tail) {
        queue->head = queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
}

int Queue_dequeue(struct Queue *queue, uint32_t *dest)
{
    struct QueueNode *old_head;
    if (!queue->head) return 0;
    old_head = queue->head;
    *dest = old_head->index;
    queue->head = old_head->next;
    free(old_head);
    if (!queue->head)
        queue->tail = NULL;
    return 1;
}

struct CharBuffer *CharBuffer_create(uint32_t start_capacity)
{
    struct CharBuffer *cb;
    if (start_capacity < MIN_CHARBUFFER)
        start_capacity = MIN_CHARBUFFER;
    cb = malloc(sizeof(*cb));
    assert(cb);
    *cb = (struct CharBuffer) {
        .buffer = malloc(start_capacity * sizeof(*(cb->buffer))),
        .length = 0,
        .capacity = start_capacity
    };
    assert(cb->buffer);
    return cb;
}

void CharBuffer_free(struct CharBuffer *cb)
{
    if (!cb) return;
    free(cb->buffer);
    cb->buffer = NULL;
    cb->length = 0;
    cb->capacity = 0;
    free(cb);
}

int CharBuffer_grow(struct CharBuffer *cb)
{
    uint32_t new_capacity;
    char *temp;
    new_capacity = cb->capacity << 1;
    temp = realloc(cb->buffer, new_capacity * sizeof(*(cb->buffer)));
    if (!temp) {
        perror("realloc");
        puts("Failed to grow CharBuffer");
        return 0;
    }
    cb->buffer = temp;
    cb->capacity = new_capacity;
    return 1;
}

int CharBuffer_shrink(struct CharBuffer *cb)
{
    uint32_t new_capacity;
    char *temp;
    new_capacity = cb->capacity >> 1;
    if (new_capacity < MIN_CHARBUFFER)
        new_capacity = MIN_CHARBUFFER;
    temp = realloc(cb->buffer, new_capacity * sizeof(*(cb->buffer)));
    if (!temp) {
        perror("realloc");
        puts("Failed to shrink CharBuffer");
        return 0;
    }
    cb->buffer = temp;
    cb->capacity = new_capacity;
    return 1;
}

uint32_t CharBuffer_get_line(struct CharBuffer *cb)
{
    uint32_t i;
    int c;
    for (
        i = 0;
        (c = getchar()) != EOF
        && c != '\n';
        ++i
    ) {
        if (i + 1 >= cb->capacity) {
            if (!CharBuffer_grow(cb)) {
                cb->length = 0;
                cb->buffer[0] = '\0';
                return 0;
            }
        }
        cb->buffer[i] = c;
    }
    if (i + 1 >= cb->capacity) {
        if (!CharBuffer_grow(cb)) {
            cb->length = 0;
            cb->buffer[0] = '\0';
            return 0;
        }
    }
    else if (i + 1 <= cb->capacity >> 2) {
        if (!CharBuffer_shrink(cb)) {
            cb->length = 0;
            cb->buffer[0] = '\0';
            return 0;
        }
    }
    cb->length = i;
    cb->buffer[i] = '\0';
    return i;
}

int Brick_overlap_xy(struct Brick brick, struct Brick other)
{
    int overlaps_x, overlaps_y;
    uint32_t x, y;

    overlaps_x = 0;
    for (x = brick.start.x; x <= brick.end.x; ++x)
        if (other.start.x <= x && x <= other.end.x)
            overlaps_x = 1;

    overlaps_y = 0;
    for (y = brick.start.y; y <= brick.end.y; ++y)
        if (other.start.y <= y && y <= other.end.y)
            overlaps_y = 1;

    return overlaps_x && overlaps_y;
}

int Brick_from_str(const char *str, struct Brick *brick)
{
    int result;
    struct vec3 start, end;
    result = sscanf(
        str,
        "%u,%u,%u~%u,%u,%u",
        &start.x, &start.y, &start.z,
        &end.x, &end.y, &end.z
    );
    if (result != 2 * sizeof(start) / sizeof(start.x)) {
        puts("Failed to parse Brick from string");
        return 0;
    }
    *brick = (struct Brick) {
        .start = (struct vec3) {
            .x = min(start.x, end.x),
            .y = min(start.y, end.y),
            .z = min(start.z, end.z)
        },
        .end = (struct vec3) {
            .x = max(start.x, end.x),
            .y = max(start.y, end.y),
            .z = max(start.z, end.z)
        }
    };
    return 1;
}

struct BrickArray *BrickArray_create(uint32_t start_capacity)
{
    struct BrickArray *ba;
    if (start_capacity < MIN_BRICKARRAY)
        start_capacity = MIN_BRICKARRAY;
    ba = malloc(sizeof(*ba));
    assert(ba);
    *ba = (struct BrickArray) {
        .bricks = malloc(start_capacity * sizeof(*(ba->bricks))),
        .length = 0,
        .capacity = start_capacity
    };
    assert(ba->bricks);
    return ba;
}

void BrickArray_free(struct BrickArray *ba)
{
    if (!ba) return;
    free(ba->bricks);
    free(ba);
}

int BrickArray_grow(struct BrickArray *ba)
{
    uint32_t new_capacity;
    struct Brick *temp;

    new_capacity = ba->capacity << 1;
    temp = realloc(ba->bricks, new_capacity * sizeof(*(ba->bricks)));
    if (!temp) {
        perror("realloc");
        return 0;
    }
    ba->bricks = temp;
    ba->capacity = new_capacity;
    return 1;
}

int BrickArray_insert(struct BrickArray *ba, struct Brick *brick)
{
    if (ba->length + 1 >= ba->capacity)
        if (!BrickArray_grow(ba))
            return 0;
    ba->bricks[ba->length] = *brick;
    ++(ba->length);
    return 1;
}

int BrickArray_load(struct BrickArray *ba)
{
    struct CharBuffer *cb;
    struct Brick brick;

    cb = CharBuffer_create(0);
    while (CharBuffer_get_line(cb)) {
        if (!Brick_from_str(cb->buffer, &brick)) {
            CharBuffer_free(cb);
            return 0;
        }
        if (!BrickArray_insert(ba, &brick)) return 0;
    }
    CharBuffer_free(cb);
    return 1;
}

uint32_t BrickArray_num_supports(struct BrickArray *ba, uint32_t index)
{
    uint32_t num, i;
    assert(index < ba->length);

    num = 0;
    for (i = index; i; --i)
        if (
            Brick_overlap_xy(ba->bricks[index], ba->bricks[i - 1])
            && ba->bricks[i - 1].end.z == ba->bricks[index].start.z - 1
        )
            ++num;
    return num;
}

int BrickArray_count_chain_rec(
    struct BrickArray *ba,
    struct Queue *queue,
    uint32_t *fallen,
    uint32_t *chain
)
{
    uint32_t index, i, is_supported;

    if (!Queue_dequeue(queue, &index))
        return 1;

    if (fallen[index])
        return BrickArray_count_chain_rec(ba, queue, fallen, chain);

    is_supported = 0;
    for (i = index; i; --i) {
        if (
            ba->bricks[i - 1].end.z == ba->bricks[index].start.z - 1
            && Brick_overlap_xy(ba->bricks[index], ba->bricks[i - 1])
            && !fallen[i - 1]
        )
            is_supported = 1;
    }
    if (is_supported)
        return BrickArray_count_chain_rec(ba, queue, fallen, chain);
    fallen[index] = 1;
    ++(*chain);

    for (i = index; i < ba->length; ++i)
        if (ba->bricks[i].start.z == ba->bricks[index].end.z + 1)
            break;

    while (
        i < ba->length
        && ba->bricks[i].start.z == ba->bricks[index].end.z + 1
    ) {
        if (Brick_overlap_xy(ba->bricks[index], ba->bricks[i]))
            Queue_enqueue(queue, i);
        ++i;
    }
    return BrickArray_count_chain_rec(ba, queue, fallen, chain);
}

uint32_t BrickArray_count_chain(struct BrickArray *ba, uint32_t index)
{
    uint32_t chain, *fallen, i;
    struct Queue *queue;
    fallen = malloc(ba->length * sizeof(*fallen));
    assert(fallen);
    memset(fallen, 0, ba->length * sizeof(*fallen));
    fallen[index] = 1;
    queue = Queue_create();
    for (i = index; i < ba->length; ++i)
        if (ba->bricks[i].start.z == ba->bricks[index].end.z + 1)
            break;

    while (
        i < ba->length
        && ba->bricks[i].start.z == ba->bricks[index].end.z + 1
    ) {
        if (Brick_overlap_xy(ba->bricks[index], ba->bricks[i]))
            Queue_enqueue(queue, i);
        ++i;
    }
    chain = 0;
    if (!BrickArray_count_chain_rec(ba, queue, fallen, &chain))
        assert(0);
    free(fallen);
    Queue_free(queue);
    return chain;
}

void BrickArray_zsort_end(struct BrickArray *ba)
{
    uint32_t i;
    int sorted;
    struct Brick temp;
    sorted = 0;
    while (!sorted) {
        sorted = 1;
        for (i = 1; i < ba->length; ++i) {
            if (ba->bricks[i - 1].end.z > ba->bricks[i].end.z) {
                temp = ba->bricks[i - 1];
                ba->bricks[i - 1] = ba->bricks[i];
                ba->bricks[i] = temp;
                sorted = 0;
            }
        }
    }
}

void BrickArray_zsort_start(struct BrickArray *ba)
{
    uint32_t i;
    int sorted;
    struct Brick temp;
    sorted = 0;
    while (!sorted) {
        sorted = 1;
        for (i = 1; i < ba->length; ++i) {
            if (ba->bricks[i - 1].start.z > ba->bricks[i].start.z) {
                temp = ba->bricks[i - 1];
                ba->bricks[i - 1] = ba->bricks[i];
                ba->bricks[i] = temp;
                sorted = 0;
            }
        }
    }
}

void BrickArray_layer(struct BrickArray *ba)
{
    uint32_t i, j, zlength, max_z;

    BrickArray_zsort_end(ba);
    for (i = 0; i < ba->length; ++i) {
        max_z = 0;
        for (j = 0; j < i; ++j) {
            if (Brick_overlap_xy(ba->bricks[i], ba->bricks[j])) {
                if (ba->bricks[j].end.z > max_z)
                    max_z = ba->bricks[j].end.z;
            }
        }
        zlength = ba->bricks[i].end.z - ba->bricks[i].start.z;
        ba->bricks[i].start.z = max_z + 1;
        ba->bricks[i].end.z = max_z + 1 + zlength;
    }
    BrickArray_zsort_start(ba);
}

void BrickArray_print(struct BrickArray *ba)
{
    uint32_t i;
    struct Brick *current;
    for (i = 0; i < ba->length; ++i) {
        current = ba->bricks + i;
        printf(
            "Brick %4u: Start = (%3u,%3u,%3u), End = (%3u,%3u,%3u)\n",
            i + 1,
            current->start.x, current->start.y, current->start.z,
            current->end.x, current->end.y, current->end.z
        );
    }
}

int main(void)
{
    struct BrickArray *ba;
    uint32_t total, i;
    
    ba = BrickArray_create(0);
    if (!BrickArray_load(ba)) {
        BrickArray_free(ba);
        return 1;
    }
    BrickArray_layer(ba);
    BrickArray_print(ba);
    for (i = total = 0; i < ba->length; ++i) {
        total = total + BrickArray_count_chain(ba, i);
    }
    printf("Total chain: %u\n", total);
    BrickArray_free(ba);
    return 0;
}
