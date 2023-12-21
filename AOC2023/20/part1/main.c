#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

/* Signals */
#define LOW     0U
#define HIGH    1U

/* Modules */
#define BROADCASTER 'B'
#define UNTYPED     'U'
#define FLIPFLOP    '%'
#define CONJUNCTION '&'

#define START_SRC   "button"
#define START_DEST  "broadcaster"

/* Internal States */
#define OFF 0U
#define ON  1U

/* Limits */
#define MIN_STRINGARRAY     8U
#define MIN_CONJSTATE       8U
#define MIN_CHARBUFFER      32U
#define HASHMAP_CAPACITY    256U

struct StringArray {
    char **strings;
    uint32_t length;
    uint32_t capacity;
};

struct Signal {
    char *src;
    char *dest;
    uint8_t type;
};

struct Module {
    char *identifier;
    void *internal_state;
    struct StringArray *dests;
    char type;
};

struct ModuleHashMap {
    struct Module **modules;
    uint32_t capacity;
    uint32_t collisions;
};

struct FlipFlopState {
    uint32_t state;
};

struct ConjunctionStateElement {
    char *src;
    uint8_t last_signal;
};

struct ConjunctionState {
    struct ConjunctionStateElement *sources;
    uint32_t length;
    uint32_t capacity;
};

struct QueueNode {
    struct QueueNode *next;
    struct Signal *signal;
};

struct Queue {
    struct QueueNode *head;
    struct QueueNode *tail;
};

struct CharBuffer {
    char *buffer;
    uint32_t length;
    uint32_t capacity;
};

uint32_t hash(char *str, uint32_t capacity)
{
    uint32_t hash, count;
    char *p_str;
    char c;

    hash = 5381;
    count = 23;
    while (count) {
        p_str = str;
        while ((c = *(p_str++)))
            hash = ((hash << 5) + hash) + c;
        --count;
    }
    return hash % capacity;
}

struct FlipFlopState *FlipFlopState_create(void)
{
    struct FlipFlopState *flipflop_state;
    flipflop_state = malloc(sizeof(*flipflop_state));
    if (!flipflop_state) {
        perror("malloc");
        puts("Failed to allocate FlipFlopState");
        return NULL;
    }
    flipflop_state->state = OFF;
    return flipflop_state;
}

void FlipFlopState_free(struct FlipFlopState *flipflop_state)
{
    free(flipflop_state);
}

struct ConjunctionState *ConjunctionState_create(uint32_t start_capacity)
{
    struct ConjunctionState *conj_state;
    if (start_capacity < MIN_CONJSTATE)
        start_capacity = MIN_CONJSTATE;
    conj_state = malloc(sizeof(*conj_state));
    if (!conj_state) {
        perror("malloc");
        puts("Failed to allocate ConjunctionState");
        return NULL;
    }
    conj_state->sources = malloc(
        start_capacity * sizeof(*(conj_state->sources))
    );
    if (!conj_state->sources) {
        perror("malloc");
        puts("Failed to allocate ConjunctionState->sources");
        free(conj_state);
        return NULL;
    }
    conj_state->length = 0;
    conj_state->capacity = start_capacity;
    return conj_state;
}

void ConjunctionState_free(struct ConjunctionState *conj_state)
{
    free(conj_state->sources);
    conj_state->sources = NULL;
    free(conj_state);
}

int ConjunctionState_grow(struct ConjunctionState *conj_state)
{
    uint32_t new_capacity;
    struct ConjunctionStateElement *temp;
    new_capacity = conj_state->capacity << 1;
    temp = realloc(
        conj_state->sources, new_capacity * sizeof(*(conj_state->sources))
    );
    if (!temp) {
        perror("realloc");
        puts("Failed to grow ConjunctionState");
        return 0;
    }
    conj_state->sources = temp;
    conj_state->capacity = new_capacity;
    return 1;
}

int ConjunctionState_insert(struct ConjunctionState *conj_state, char *src)
{
    if (conj_state->length + 1 >= conj_state->capacity)
        if (!ConjunctionState_grow(conj_state))
            return 0;
    conj_state->sources[(conj_state->length)++] = (
        (struct ConjunctionStateElement) {
            .src = src,
            .last_signal = LOW
        }
    );
    return 1;
}

struct CharBuffer *CharBuffer_create(uint32_t start_capacity)
{
    struct CharBuffer *cb;
    if (start_capacity < MIN_CHARBUFFER)
        start_capacity = MIN_CHARBUFFER;
    cb = malloc(sizeof(*cb));
    if (!cb) {
        perror("malloc");
        puts("Failed to allocate CharBuffer");
        return NULL;
    }
    cb->buffer = malloc(start_capacity * sizeof(*(cb->buffer)));
    if (!cb->buffer) {
        perror("malloc");
        puts("Failed to allocate CharBuffer->buffer");
        free(cb);
        return NULL;
    }
    cb->length = 0;
    cb->capacity = start_capacity;
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
    cb->buffer[i + 1] = '\0';
    return i;
}

struct StringArray *StringArray_create(uint32_t start_capacity)
{
    struct StringArray *sa;
    if (start_capacity < MIN_STRINGARRAY)
        start_capacity = MIN_STRINGARRAY;
    sa = malloc(sizeof(*sa));
    if (!sa) {
        perror("malloc");
        puts("Failed to allocate StringArray");
        return NULL;
    }
    sa->strings = malloc(start_capacity * sizeof(*(sa->strings)));
    if (!sa->strings) {
        perror("malloc");
        puts("Failed to allocate StringArray->strings");
        free(sa);
        return NULL;
    }
    sa->length = 0;
    sa->capacity = start_capacity;
    return sa;
}

void StringArray_free(struct StringArray *sa)
{
    if (!sa) return;
    uint32_t i;
    for (i = 0; i < sa->length; ++i)
        free(sa->strings[i]);
    free(sa->strings);
    sa->strings = NULL;
    free(sa);
}

int StringArray_grow(struct StringArray *sa)
{
    uint32_t new_capacity;
    char **temp;
    new_capacity = sa->capacity << 1;
    temp = realloc(sa->strings, new_capacity * sizeof(*(sa->strings)));
    if (!temp) {
        perror("realloc");
        puts("Failed to grow StringArray");
        return 0;
    }
    sa->strings = temp;
    sa->capacity = new_capacity;
    return 1;
}

int StringArray_insert(struct StringArray *sa, char *str, uint32_t length)
{
    char *string;
    if (sa->length + 1 >= sa->capacity)
        if (!StringArray_grow(sa))
            return 0;
    string = malloc(length + 1);
    if (!string) {
        perror("malloc");
        puts("Failed to copy string");
        return 0;
    }
    memcpy(string, str, (length + 1) * sizeof(*str));
    string[length] = '\0';
    sa->strings[sa->length] = string;
    ++(sa->length);
    return 1;
}

int StringArray_load(struct StringArray *sa)
{
    struct CharBuffer *cb;
    cb = CharBuffer_create(0);
    if (!cb) return 0;
    while (CharBuffer_get_line(cb)) {
        if (!StringArray_insert(sa, cb->buffer, cb->length)) {
            puts("Failed to load stdin into StringArray");
            CharBuffer_free(cb);
            return 0;
        }
    }
    CharBuffer_free(cb);
    return 1;
}

struct StringArray *StringArray_from_delimiter(char *str, char del)
{
    char format_get_length[32], format_get_str[32], *token;
    uint32_t length, offset;
    struct StringArray *sa;
    sa = StringArray_create(0);
    if (!sa) return NULL;
    if (
        sprintf(
            format_get_length,
            "%s%c%s", "%*[^", del, "]%n%*s"
        ) < 0
    ) {
        perror("snprintf");
        puts("Failed to create format string");
        StringArray_free(sa);
        return NULL;
    }
    if (
        sprintf(
            format_get_str,
            "%s%c%s", "%[^", del, "]%*c%n%*s"
        ) < 0
    ) {
        perror("snprintf");
        puts("Failed to create format string");
        StringArray_free(sa);
        return NULL;
    }
    offset = 1;
    while (offset) {
        offset = 0;
        if (sscanf(str, format_get_length, &length) != 0) {
            if (!StringArray_insert(sa, str, strlen(str))) {
                StringArray_free(sa);
                return NULL;
            }
            continue;
        }
        token = malloc((length + 1) * sizeof(*token));
        if (!token) {
            perror("malloc");
            puts("Failed to allocate token string");
            StringArray_free(sa);
            return NULL;
        }
        if (sscanf(str, format_get_str, token, &offset) != 1) {
            perror("sscanf");
            puts("Failed to parse token");
            free(token);
            StringArray_free(sa);
            return NULL;
        }
        token[length - 1] = '\0';
        if (!StringArray_insert(sa, token, length)) {
            free(token);
            StringArray_free(sa);
            return NULL;
        }
        free(token);
        str = str + offset;
    }
    return sa;
}

struct Queue *Queue_create(void)
{
    struct Queue *queue;
    queue = malloc(sizeof(*queue));
    if (!queue) {
        perror("malloc");
        puts("Failed to allocate Queue");
        return NULL;
    }
    *queue = (struct Queue) {
        .head = NULL,
        .tail = NULL
    };
    return queue;
}

void Queue_free(struct Queue *queue)
{
    if (!queue) return;
    struct QueueNode *node, *last;
    if (queue->head) {
        last = queue->head;
        node = last->next;
        while (node) {
            free(last->signal);
            free(last);
            last = node;
            node = node->next;
        }
        free(last->signal);
        free(last);
    }
    queue->head = NULL;
    queue->tail = NULL;
    free(queue);
}

int Queue_enqueue(struct Queue *queue, struct Signal signal)
{
    struct QueueNode *node;
    node = malloc(sizeof(*node));
    if (!node) {
        perror("malloc");
        puts("Failed to allocate QueueNode");
        return 0;
    }
    node->signal = malloc(sizeof(*(node->signal)));
    if (!node->signal) {
        perror("malloc");
        puts("Failed to allocate QueueNode->Signal");
        free(node);
        return 0;
    }
    *(node->signal) = signal;
    node->next = NULL;
    if (!queue->head && !queue->tail) {
        queue->head = node;
        queue->tail = node;
    } else {
        queue->tail->next = node;
        queue->tail = node;
    }
    return 1;
}

struct Signal *Queue_dequeue(struct Queue *queue)
{
    if (!queue->head) return NULL;
    struct Signal *signal;
    struct QueueNode *old_head;
    old_head = queue->head;
    signal = old_head->signal;
    queue->head = old_head->next;
    if (!queue->head) queue->tail = NULL;
    free(old_head);
    return signal;
}

struct Module *Module_create(void)
{
    struct Module *module;
    module = malloc(sizeof(*module));
    if (!module) {
        perror("malloc");
        puts("Failed to allocate Module");
        return NULL;
    }
    module->identifier = NULL;
    module->internal_state = NULL;
    module->dests = NULL;
    module->type = UNTYPED;
    return module;
}

void Module_free(struct Module *module)
{
    if (!module) return;
    switch (module->type) {
    case FLIPFLOP:
        FlipFlopState_free(module->internal_state);
        module->internal_state = NULL;
        break;
    case CONJUNCTION:
        ConjunctionState_free(module->internal_state);
        module->internal_state = NULL;
        break;
    }
    if (module->internal_state)
        free(module->internal_state);
    free(module->identifier);
    StringArray_free(module->dests);
    module->identifier = NULL;
    module->internal_state = NULL;
    module->dests = NULL;
    free(module);
}

struct Module *Module_from_string(char *str)
{
    uint32_t ident_length, offset;
    char *identifier;
    struct Module *module;
    struct StringArray *sa;

    module = Module_create();
    if (!module)
        goto error_module_alloc;
    if (*str == FLIPFLOP || *str == CONJUNCTION)
        module->type = *(str++);
    if (sscanf(str, "%*[^ ]%n%*s", &ident_length) != 0)
        goto error_ident_length;
    identifier = malloc((ident_length + 1) * sizeof(*identifier));
    if (!identifier)
        goto error_alloc_identifier;
    if (
        sscanf(
            str, "%[^ ]%*[^>]%*[^ ]%*c%n%*s", identifier, &offset
        ) != 1
    ) goto error_identifier;
    module->identifier = identifier;
    if (strcmp(identifier, START_DEST) == 0)
        module->type = BROADCASTER;
    if (!module->type)
        module->type = UNTYPED;
    sa = StringArray_from_delimiter(str + offset, ' ');
    if (!sa)
        goto error_dests;
    module->dests = sa;
    module->internal_state = NULL;
    return module;
    
    /* Error Handling */
error_module_alloc:
    goto error;
error_ident_length:
    puts("Failed to calculate identifier length");
    goto free_module;
error_alloc_identifier:
    perror("malloc");
    puts("Failed to allocate identifier string");
    goto free_module;
error_identifier:
    perror("sscanf");
    puts("Failed to retrieve identifier");
    goto free_ident;
error_dests:
    puts("Failed to parse destinations");
    goto free_ident;

free_ident:
    free(identifier);
free_module:
    Module_free(module);
error:
    puts("Failed to create module from string");
    return NULL;
}

struct ModuleHashMap *ModuleHashMap_create(void)
{
    struct ModuleHashMap *hashmap;
    uint32_t i;
    hashmap = malloc(sizeof(*hashmap));
    if (!hashmap) {
        perror("malloc");
        puts("Failed to allocate ModuleHashMap");
        return NULL;
    }
    hashmap->modules = malloc(
        HASHMAP_CAPACITY * sizeof(*(hashmap->modules))
    );
    if (!hashmap->modules) {
        perror("malloc");
        puts("Failed to allocate ModuleHashMap->modules");
        free(hashmap);
        return NULL;
    }
    hashmap->capacity = HASHMAP_CAPACITY;
    for (i = 0; i < hashmap->capacity; ++i)
        hashmap->modules[i] = NULL;
    hashmap->collisions = 0;
    return hashmap;
}

void ModuleHashMap_free(struct ModuleHashMap *hashmap, int print_collisions)
{
    if (!hashmap) return;
    uint32_t i;
    if (print_collisions)
        printf("Collisions: %u\n", hashmap->collisions);
    for (i = 0; i < hashmap->capacity; ++i)
        Module_free(hashmap->modules[i]);
    free(hashmap->modules);
    hashmap->modules = NULL;
    hashmap->capacity = 0;
    hashmap->collisions = 0;
    free(hashmap);
}

int ModuleHashMap_store(struct ModuleHashMap *hashmap, struct Module *module)
{
    uint32_t start_index, i;
    start_index = hash(module->identifier, hashmap->capacity);
    if (hashmap->modules[start_index]) ++(hashmap->collisions);
    for (i = start_index; i < hashmap->capacity; ++i) {
        if (!hashmap->modules[i]) {
            hashmap->modules[i] = module;
            return 1;
        }
    }
    for (i = 0; i < start_index; ++i) {
        if (!hashmap->modules[i]) {
            hashmap->modules[i] = module;
            return 1;
        }
    }
    puts("Ran out of space in the ModuleHashMap");
    return 0;
}

struct Module *ModuleHashMap_retrieve(
    struct ModuleHashMap *hashmap, char *identifier
)
{
    uint32_t start_index, i;
    start_index = hash(identifier, hashmap->capacity);
    for (i = start_index; i < hashmap->capacity; ++i)
        if (!hashmap->modules[i]) return NULL;
        else if (strcmp(hashmap->modules[i]->identifier, identifier) == 0)
            return hashmap->modules[i];
    for (i = 0; i < start_index; ++i)
        if (!hashmap->modules[i]) return NULL;
        else if (strcmp(hashmap->modules[i]->identifier, identifier) == 0)
            return hashmap->modules[i];
    return NULL;
}

int ModuleHashMap_load(struct ModuleHashMap *hashmap)
{
    struct Module *module;
    struct StringArray *sa;
    uint32_t i;
    sa = StringArray_create(0);
    if (!sa) return 1;
    if (!StringArray_load(sa)) {
        puts("Failed to load file");
        StringArray_free(sa);
        return 0;
    }
    for (i = 0; i < sa->length; ++i) {
        module = Module_from_string(sa->strings[i]);
        if (!module) {
            puts("Failed to load modules from file");
            StringArray_free(sa);
            return 0;
        }
        if (!ModuleHashMap_store(hashmap, module)) {
            puts("Failed to load module into hashmap");
            Module_free(module);
            StringArray_free(sa);
            return 0;
        }
    }
    StringArray_free(sa);
    return 1;
}

int ModuleHashMap_init_flipflop(struct Module *module)
{
    struct FlipFlopState *flipflop_state;
    if (module->type != FLIPFLOP) {
        puts("Module is not a flip flop");
        return 0;
    }
    flipflop_state = FlipFlopState_create();
    if (!flipflop_state) return 0;
    module->internal_state = flipflop_state;
    return 1;
}

int ModuleHashMap_init_conjunction(struct Module *module)
{
    struct ConjunctionState *conj_state;
    if (module->type != CONJUNCTION) {
        puts("Module is not a conjunction");
        return 0;
    }
    if (module->internal_state) return 1;
    conj_state = ConjunctionState_create(0);
    if (!conj_state) return 0;
    module->internal_state = conj_state;
    return 1;
}

int ModuleHashMap_init(struct ModuleHashMap *hashmap)
{
    uint32_t i, j;
    char *dest_identifier;
    struct Module *dest;
    struct ConjunctionState *conj_state;

    for (i = 0; i < hashmap->capacity; ++i) {
        if (!hashmap->modules[i]) continue;
        switch (hashmap->modules[i]->type) {
        case FLIPFLOP:
            if (!ModuleHashMap_init_flipflop(hashmap->modules[i]))
                return 0;
            break;
        case CONJUNCTION:
            if (!ModuleHashMap_init_conjunction(hashmap->modules[i]))
                return 0;
            break;
        }
        for (j = 0; j < hashmap->modules[i]->dests->length; ++j) {
            dest_identifier = hashmap->modules[i]->dests->strings[j];
            dest = ModuleHashMap_retrieve(hashmap, dest_identifier);
            if (dest && dest->type == CONJUNCTION) {
                conj_state = dest->internal_state;
                if (!conj_state) {
                    if (!ModuleHashMap_init_conjunction(dest))
                        return 0;
                    conj_state = dest->internal_state;
                }
                if (
                    !ConjunctionState_insert(
                        conj_state, hashmap->modules[i]->identifier
                    )
                ) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int ModuleHashMap_handle_broadcaster(
    struct Queue *queue,
    struct Module *module,
    struct Signal *signal
)
{
    uint32_t i;
    for (i = 0; i < module->dests->length; ++i) {
        if (!
            Queue_enqueue(
                queue,
                (struct Signal) {
                    .src = module->identifier,
                    .dest = module->dests->strings[i],
                    .type = signal->type
                } 
            )
        ) {
            puts("Failed to enqueue signal");
            return 0;
        }
    }
    return 1;
}

int ModuleHashMap_handle_flipflop(
    struct Queue *queue,
    struct Module *module,
    struct Signal *signal
)
{
    struct FlipFlopState *state;
    uint32_t i;
    uint8_t type;

    if (signal->type == HIGH) return 1;
    state = module->internal_state;
    if (state->state == ON) {
        state->state = OFF;
        type = LOW;
    } else {
        state->state = ON;
        type = HIGH;
    }
    for (i = 0; i < module->dests->length; ++i) {
        if (
            !Queue_enqueue(
                queue,
                (struct Signal) {
                    .src = module->identifier,
                    .dest = module->dests->strings[i],
                    .type = type
                }
            )
        ) {
            puts("Failed to enqueue signal");
            return 0;
        }
    }
    return 1;
}

int ModuleHashMap_handle_conjunction(
    struct Queue *queue,
    struct Module *module,
    struct Signal *signal
)
{
    struct ConjunctionState *conj_state;
    uint8_t type;
    uint32_t i;

    conj_state = module->internal_state;
    type = LOW;
    for (i = 0; i < conj_state->length; ++i) {
        if (strcmp(signal->src, conj_state->sources[i].src) == 0) {
            conj_state->sources[i].last_signal = signal->type;
        }
        if (conj_state->sources[i].last_signal == LOW)
            type = HIGH;
    }
    for (i = 0; i < module->dests->length; ++i) {
        if (
            !Queue_enqueue(
                queue,
                (struct Signal) {
                    .src = module->identifier,
                    .dest = module->dests->strings[i],
                    .type = type
                }
            )
        ) {
            puts("Failed to enqueue signal");
            return 0;
        }
    }
    return 1;
}

int ModuleHashMap_handle_signals(
    struct ModuleHashMap *hashmap,
    struct Queue *queue,
    uint32_t *num_lows,
    uint32_t *num_highs
)
{
    struct Signal *signal;
    struct Module *dest;
    signal = Queue_dequeue(queue);
    if (!signal) return 1;
    if (signal->type == HIGH) ++(*num_highs);
    if (signal->type == LOW) ++(*num_lows);
    dest = ModuleHashMap_retrieve(hashmap, signal->dest);
    if (!dest) {
        free(signal);
        return ModuleHashMap_handle_signals(
            hashmap, queue, num_lows, num_highs
        );
    }
    switch (dest->type) {
    case BROADCASTER:
        if (
            !ModuleHashMap_handle_broadcaster(
                queue, dest, signal
            )
        ) {
            free(signal);
            return 0;
        }
        break;
    case FLIPFLOP:
        if (
            !ModuleHashMap_handle_flipflop(
                queue, dest, signal
            )
        ) {
            free(signal);
            return 0;
        }
        break;
    case CONJUNCTION:
        if (
            !ModuleHashMap_handle_conjunction(
                queue, dest, signal
            )
        ) {
            free(signal);
            return 0;
        }
        break;
    }
    free(signal);
    return ModuleHashMap_handle_signals(
        hashmap, queue, num_lows, num_highs
    );
}

int main(void)
{
    struct ModuleHashMap *hashmap;
    struct Queue *queue;
    uint32_t num_lows, num_highs, i;

    num_lows = num_highs = 0;
    queue = Queue_create();
    if (!queue) return 1;
    hashmap = ModuleHashMap_create();
    if (!hashmap) {
        Queue_free(queue);
        return 1;
    }
    if (!ModuleHashMap_load(hashmap)) {
        Queue_free(queue);
        ModuleHashMap_free(hashmap, 0);
        return 1;
    }
    if (!ModuleHashMap_init(hashmap)) {
        Queue_free(queue);
        ModuleHashMap_free(hashmap, 0);
        return 1;
    }
    for (i = 0; i < 1000; ++i) {
        if (
            !Queue_enqueue(
                queue,
                (struct Signal) {
                    .src = START_SRC,
                    .dest = START_DEST,
                    .type = LOW
                }
            )
        ) {
            Queue_free(queue);
            ModuleHashMap_free(hashmap, 0);
            return 1;
        }
        if (
            !ModuleHashMap_handle_signals(
                hashmap, queue, &num_lows, &num_highs
            )
        ) {
            Queue_free(queue);
            ModuleHashMap_free(hashmap, 0);
            return 1;
        }
    }
    printf("Num lows: %u, Num highs: %u\n", num_lows, num_highs);
    printf("Product: %u\n", num_lows * num_highs);
    Queue_free(queue);
    ModuleHashMap_free(hashmap, 1);
    return 0;
}
