#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>

#define MIN_CHARBUFFER          32U
#define MIN_HAILSTONE_BUFFER    16U

/*
 * #define MIN_RANGE   (double)7L
 * #define MAX_RANGE   (double)27L
 */

#define MIN_RANGE   (double)200000000000000L
#define MAX_RANGE   (double)400000000000000L

struct vec3 {
    int64_t x;
    int64_t y;
    int64_t z;
};

struct vec3lf {
    double x;
    double y;
    double z;
};

struct Hailstone {
    struct vec3 pos;
    struct vec3 vel;
};

struct HailstoneBuffer {
    struct Hailstone *buffer;
    uint32_t length;
    uint32_t capacity;
};

struct CharBuffer {
    char *buffer;
    uint32_t length;
    uint32_t capacity;
};

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
    free(cb);
}

void CharBuffer_grow(struct CharBuffer *cb)
{
    uint32_t new_capacity;
    char *temp;
    new_capacity = cb->capacity << 1;
    temp = realloc(cb->buffer, new_capacity * sizeof(*(cb->buffer)));
    assert(temp);
    cb->buffer = temp;
    cb->capacity = new_capacity;
}

void CharBuffer_shrink(struct CharBuffer *cb)
{
    uint32_t new_capacity;
    char *temp;
    new_capacity = cb->capacity >> 1;
    if (new_capacity < MIN_CHARBUFFER)
        new_capacity = MIN_CHARBUFFER;
    temp = realloc(cb->buffer, new_capacity * sizeof(*(cb->buffer)));
    assert(temp);
    cb->buffer = temp;
    cb->capacity = new_capacity;
}

void CharBuffer_insert(struct CharBuffer *cb, char value)
{
    if (cb->length + 1 >= cb->capacity)
        CharBuffer_grow(cb);
    cb->buffer[(cb->length)++] = value;
}

uint32_t CharBuffer_get_line(struct CharBuffer *cb)
{
    uint32_t i;
    int c;
    cb->length = 0;
    for(i = 0; (c = getchar()) != EOF && c != '\n'; ++i)
        CharBuffer_insert(cb, c);
    CharBuffer_insert(cb, '\0');
    while (cb->length <= cb->capacity >> 2 && cb->capacity > MIN_CHARBUFFER)
        CharBuffer_shrink(cb);
    return i;
}

struct Hailstone Hailstone_from_string(const char *str)
{
    struct Hailstone hailstone;
    int result;
    result = sscanf(
        str, "%ld, %ld, %ld @ %ld, %ld, %ld",
        &hailstone.pos.x, &hailstone.pos.y, &hailstone.pos.z,
        &hailstone.vel.x, &hailstone.vel.y, &hailstone.vel.z
    );
    assert(result == 6);
    return hailstone;
}

int Hailstone_intersect_xy(
    struct Hailstone hailstone, struct Hailstone other, struct vec3lf *result
)
{
    double x, y, t, t_p, determinant;
    determinant = (double)(
        other.vel.x * hailstone.vel.y - hailstone.vel.x * other.vel.y
    );
    if (determinant == 0.0)
        return 0;
    t = (
        (double)(
            other.vel.x * (other.pos.y - hailstone.pos.y)
            - other.vel.y * (other.pos.x - hailstone.pos.x)
        ) / determinant
    );
    t_p = (
        (double)(
            hailstone.vel.x * (other.pos.y - hailstone.pos.y)
            - hailstone.vel.y * (other.pos.x - hailstone.pos.x)
        ) / determinant
    );
    if (t < 0.0 || t_p < 0.0)
        return 0;
    x = (double)hailstone.pos.x + t * (double)hailstone.vel.x;
    y = (double)hailstone.pos.y + t * (double)hailstone.vel.y;
    *result = (struct vec3lf) {
        .x = x, .y = y, .z = 0.0
    };
    return 1;
}

struct HailstoneBuffer *HailstoneBuffer_create(uint32_t start_capacity)
{
    struct HailstoneBuffer *hs_buff;
    if (start_capacity < MIN_HAILSTONE_BUFFER)
        start_capacity = MIN_HAILSTONE_BUFFER;
    hs_buff = malloc(sizeof(*hs_buff));
    assert(hs_buff);
    *hs_buff = (struct HailstoneBuffer) {
        .buffer = malloc(start_capacity * sizeof(*(hs_buff->buffer))),
        .length = 0,
        .capacity = start_capacity
    };
    assert(hs_buff->buffer);
    return hs_buff;
}

void HailstoneBuffer_free(struct HailstoneBuffer *hs_buff)
{
    if (!hs_buff) return;
    free(hs_buff->buffer);
    free(hs_buff);
}

void HailstoneBuffer_grow(struct HailstoneBuffer *hs_buff)
{
    uint32_t new_capacity;
    struct Hailstone *temp;
    new_capacity = hs_buff->capacity << 1;
    temp = realloc(
        hs_buff->buffer, new_capacity * sizeof(*(hs_buff->buffer))
    );
    assert(temp);
    hs_buff->buffer = temp;
    hs_buff->capacity = new_capacity;
}

void HailstoneBuffer_insert(
    struct HailstoneBuffer *hs_buff, struct Hailstone hailstone
)
{
    if (hs_buff->length + 1 >= hs_buff->capacity)
        HailstoneBuffer_grow(hs_buff);
    hs_buff->buffer[(hs_buff->length)++] = hailstone;
}

void HailstoneBuffer_load(struct HailstoneBuffer *hs_buff)
{
    struct CharBuffer *cb;
    cb = CharBuffer_create(0);
    while (CharBuffer_get_line(cb))
        HailstoneBuffer_insert(hs_buff, Hailstone_from_string(cb->buffer));
    CharBuffer_free(cb);
}

uint32_t HailstoneBuffer_count_intersects_in(
    struct HailstoneBuffer *hs_buff, double min, double max
)
{
    uint32_t i, j, total;
    struct vec3lf intersection;
    total = 0;
    for (i = 0; i < hs_buff->length; ++i) {
        for (j = i + 1; j < hs_buff->length; ++j) {
            if (
                Hailstone_intersect_xy(
                    hs_buff->buffer[i], hs_buff->buffer[j], &intersection
                )
            )
                if (
                    min <= intersection.x && intersection.x <= max
                    && min <= intersection.y && intersection.y <= max
                )
                    ++total;
        }
    }
    return total;
}

int main(void)
{
    struct HailstoneBuffer *hs_buff;
    uint32_t total;
    hs_buff = HailstoneBuffer_create(0);
    HailstoneBuffer_load(hs_buff);
    total = HailstoneBuffer_count_intersects_in(
        hs_buff, MIN_RANGE, MAX_RANGE
    );
    printf("Total = %u\n", total);
    HailstoneBuffer_free(hs_buff);
    return 0;
}
