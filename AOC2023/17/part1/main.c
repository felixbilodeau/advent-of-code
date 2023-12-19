#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INFINITY 9999

#define MAX_STRAIGHT_SEGMENT 3

#define NORTH   0
#define SOUTH   1
#define EAST    2
#define WEST    3
#define INVALID 4

#define QUEUE_MIN 8

struct QueueItem {
	size_t priority;
    size_t node;
    size_t direction;
    size_t straight;
};

struct PriorityQueue {
	struct QueueItem *items;
	size_t length;
	size_t capacity;
};

struct Edge {
    struct Edge *next;
    size_t node;
    size_t weight;
    size_t direction;
};

struct AdjacencyList {
    struct Edge **adjacency_list;
    size_t num_nodes;
};

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

int PriorityQueue_grow(struct PriorityQueue *queue)
{
	struct QueueItem *temp;
    size_t new_capacity;
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
	size_t new_capacity;

	new_capacity = queue->capacity / 2;
	if (new_capacity < QUEUE_MIN) new_capacity = QUEUE_MIN;

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

struct PriorityQueue *PriorityQueue_create(size_t start_capacity)
{
	struct PriorityQueue *queue;
	queue = malloc(sizeof(*queue));
	if (!queue) {
		perror("malloc");
		puts("Failed to create PriorityQueue");
		return NULL;
	}
	if (start_capacity < QUEUE_MIN) start_capacity = QUEUE_MIN;
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

void QueueItem_swap(struct QueueItem *a, struct QueueItem *b)
{
	struct QueueItem temp;
	temp = *b;
	*b = *a;
	*a = temp;
}

void PriorityQueue_heapify(struct PriorityQueue *queue, size_t index)
{
	if (queue->length == 1) return;
	size_t smallest, left, right;
	smallest = index;
	left = 2 * index + 1;
	right = left + 1;
	if (
		left < queue->length
		&& queue->items[left].priority < queue->items[smallest].priority
	) smallest = left;
	if (
		right < queue->length
		&& queue->items[right].priority < queue->items[smallest].priority
	) smallest = right;
	if (smallest != index) {
		QueueItem_swap(&(queue->items[smallest]), &(queue->items[index]));
		PriorityQueue_heapify(queue, smallest);
	}
}

int PriorityQueue_enqueue(struct PriorityQueue *queue, struct QueueItem item)
{
	size_t i;
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
	size_t i;
	if (queue->length == 0) return 0;
	*dest = queue->items[0];
	QueueItem_swap(&(queue->items[0]), &(queue->items[queue->length - 1]));
	--(queue->length);
	if (queue->length < queue->capacity / 4 && queue->capacity > QUEUE_MIN)
		if (!PriorityQueue_shrink(queue)) return 0;
	for (i = queue->length / 2; i > 0; --i)
		PriorityQueue_heapify(queue, i - 1);
	return 1;
}

void PriorityQueue_free(struct PriorityQueue *queue)
{
	free(queue->items);
	free(queue);
}

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

void free_edge_list(struct Edge *head)
{
    if (!head) return;
    struct Edge *next;
    next = head->next;
    free(head);
    if (next) free_edge_list(next);
}

void free_adj_list(struct AdjacencyList adjacency_list)
{
    size_t node;
    for (node = 0; node < adjacency_list.num_nodes; ++node)
        free_edge_list(adjacency_list.adjacency_list[node]);
    free(adjacency_list.adjacency_list);
}

struct Edge *create_edge(
    struct Edge *next, size_t to, size_t weight, size_t direction
)
{
    struct Edge *edge;
    edge = malloc(sizeof(*edge));
    if (!edge) {
        perror("malloc");
        puts("Failed to allocate edge");
        return NULL;
    }
    edge->next = next;
    edge->node = to;
    edge->weight = weight;
    edge->direction = direction;
    return edge;
}

struct AdjacencyList init_adj_list(struct Buffer2D *buff)
{
    size_t num_nodes, node, to, line, col, weight;
    struct AdjacencyList adj;
    struct Edge *edge;
    num_nodes = buff->num_lines * buff->num_cols;
    adj.num_nodes = num_nodes;
    adj.adjacency_list = malloc(
        num_nodes * sizeof(*(adj.adjacency_list))
    );
    if (!adj.adjacency_list) {
        perror("malloc");
        puts("Failed to allocate adjacency list");
        return adj;
    }
    for (line = 0; line < buff->num_lines; ++line) {
        for (col = 0; col < buff->num_cols; ++col) {
            node = line * buff->num_cols + col;
            adj.adjacency_list[node] = NULL;
            /* North */
            if (line > 0) {
                to = (line - 1) * buff->num_cols + col;
                weight = (size_t)(buff->buffer[to] - '0');
                edge = create_edge(
                    adj.adjacency_list[node], to, weight, NORTH
                );
                if (!edge) {
                    free_adj_list(adj);
                    return (struct AdjacencyList){.adjacency_list = NULL};
                }
                adj.adjacency_list[node] = edge;
            }
            /* South */
            if (line < buff->num_lines - 1) {
                to = (line + 1) * buff->num_cols + col;
                weight = (size_t)(buff->buffer[to] - '0');
                edge = create_edge(
                    adj.adjacency_list[node], to, weight, SOUTH
                );
                if (!edge) {
                    free_adj_list(adj);
                    return (struct AdjacencyList){.adjacency_list = NULL};
                }
                adj.adjacency_list[node] = edge;
            }
            /* East */
            if (col < buff->num_cols - 1) {
                to = line * buff->num_cols + (col + 1);
                weight = (size_t)(buff->buffer[to] - '0');
                edge = create_edge(
                    adj.adjacency_list[node], to, weight, EAST
                );
                if (!edge) {
                    free_adj_list(adj);
                    return (struct AdjacencyList){.adjacency_list = NULL};
                }
                adj.adjacency_list[node] = edge;
            }
            /* West */
            if (col > 0) {
                to = line * buff->num_cols + (col - 1);
                weight = (size_t)(buff->buffer[to] - '0');
                edge = create_edge(
                    adj.adjacency_list[node], to, weight, WEST
                );
                if (!edge) {
                    free_adj_list(adj);
                    return (struct AdjacencyList){.adjacency_list = NULL};
                }
                adj.adjacency_list[node] = edge;
            }
        }
    }
    return adj;
}

void print_adj_list(struct AdjacencyList adj)
{
    size_t node;
    struct Edge *edge;
    char *direction;
    for (node = 0; node < adj.num_nodes; ++node) {
        printf("node %zu:\n", node);
        edge = adj.adjacency_list[node];
        while (edge) {
            switch (edge->direction) {
            case NORTH:
                direction = "North";
                break;
            case SOUTH:
                direction = "South";
                break;
            case EAST:
                direction = "East";
                break;
            case WEST:
                direction = "West";
                break;
            default:
                direction = "Invalid";
                break;
            }
            printf(
                "    -%zu-> %zu (%s)\n",
                edge->weight,
                edge->node,
                direction
            );
            edge = edge->next;
        }
    }
}

int is_not_reverse(size_t dir1, size_t dir2)
{
    switch (dir1) {
    case NORTH: return dir2 != SOUTH;
    case SOUTH: return dir2 != NORTH;
    case EAST: return dir2 != WEST;
    case WEST: return dir2 != EAST;
    default: return 1;
    }
}

int dijkstra(struct AdjacencyList *adj, size_t start, size_t ***distances)
{
    struct PriorityQueue *queue;
    struct Edge *edge;
    struct QueueItem item;
    size_t node, straight, new_distance, count;
    if (!(queue = PriorityQueue_create(adj->num_nodes / 2))) return 0;
    item = (struct QueueItem) {
        .priority = 0,
        .node = start,
        .direction = INVALID,
        .straight = 0
    };
    if (!PriorityQueue_enqueue(queue, item)) return 0;
    count = 0;
    while (PriorityQueue_dequeue(queue, &item)) {
        node = item.node;
        edge = adj->adjacency_list[node];
        while (edge) {
            if (
                !(
                    edge->direction == item.direction
                    && item.straight + 1 > MAX_STRAIGHT_SEGMENT
                )
                && is_not_reverse(edge->direction, item.direction)
            ) {
                new_distance = item.priority + edge->weight;
                if (edge->direction == item.direction)
                    straight = item.straight + 1;
                else
                    straight = 1;
                if (new_distance < distances[edge->node][edge->direction][straight - 1]) {
                    distances[edge->node][edge->direction][straight - 1] = new_distance;
                    if (
                        !PriorityQueue_enqueue(
                            queue,
                            (struct QueueItem) {
                                .priority = new_distance,
                                .node = edge->node,
                                .direction = edge->direction,
                                .straight = straight
                            }
                        )
                    ) return 0;
                }
            }
            edge = edge->next;
        }
        ++count;
    }
    printf("Count = %zu\n", count);
    PriorityQueue_free(queue);
    return 1;
}

int main(void)
{
    struct AdjacencyList adj;
    struct Buffer2D blocks;
    size_t start_node, end_node, min_distance;
    size_t ***distances, node, direction, segment;
    Buffer2D_create(&blocks, 0);
    Buffer2D_load_file(&blocks);
    adj = init_adj_list(&blocks);
    if (!adj.adjacency_list) {
        puts("Error allocating adjacency list");
        return 1;
    }
    distances = malloc(adj.num_nodes * sizeof(*distances));
    if (!distances) {
        perror("malloc");
        puts("Failed to allocate distances array");
        return 1;
    }
    for (node = 0; node < adj.num_nodes; ++node) {
        distances[node] = malloc(4 * sizeof(**distances));
        if (!distances[node]) {
            perror("malloc");
            puts("Failed to allocate distances array");
            return 1;
        }
        for (direction = NORTH; direction <= WEST; ++direction) {
            distances[node][direction] = malloc(4 * sizeof(***distances));
            if (!distances[node][direction]) {
                perror("malloc");
                puts("Failed to allocate distances array");
                return 1;
            }
            for (segment = 0; segment < MAX_STRAIGHT_SEGMENT; ++segment)
                distances[node][direction][segment] = INFINITY;
        }
    }
    start_node = 0;
    end_node = adj.num_nodes - 1;
    if (!dijkstra(&adj, start_node, distances)) return 1;
    min_distance = INFINITY;
    for (direction = NORTH; direction <= WEST; ++direction)
        for (segment = 0; segment < MAX_STRAIGHT_SEGMENT; ++segment)
            if (distances[end_node][direction][segment] < min_distance)
                min_distance = distances[end_node][direction][segment];
    printf("Minimum distance: %zu\n", min_distance);
    for (node = 0; node < adj.num_nodes; ++node) {
        for (direction = NORTH; direction <= WEST; ++direction)
            free(distances[node][direction]);
        free(distances[node]);
    }
    free(distances);
    free_adj_list(adj);
    Buffer2D_free_internals(&blocks);
    return 0;
}
