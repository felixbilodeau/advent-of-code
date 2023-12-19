#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_CACHE       2048
#define MIN_STACK       32
#define MIN_CHARBUFFER  32
#define MIN_WORKFLOW    8

#define STACK_EMPTY -1

#define REJECTED "R"
#define ACCEPTED "A"

#define LESS_THAN       '<'
#define GREATER_THAN    '>'

#define RANGE_START 1
#define RANGE_END   4000

#define HASH_REPEAT     23

#define START_WORKFLOW "in"

struct Range {
    size_t start;
    size_t end;
};

struct InputRange {
    char *wf_identifier;
    struct Range x_range;
    struct Range m_range;
    struct Range a_range;
    struct Range s_range;
};

struct InputRangeStack {
    struct InputRange *stack;
    size_t length;
    size_t capacity;
};

struct WorkflowRule {
    char variable;
    char operator;
    size_t value;
    char *dest;
};

struct Workflow {
    char *identifier;
    struct WorkflowRule *rules;
    size_t length;
    size_t capacity;
};

struct WorkflowCache {
    struct Workflow **cache;
    size_t capacity;
    size_t collisions;
};

struct CharBuffer {
    char *buffer;
    size_t length;
    size_t capacity;
};

int InputRange_is_valid(struct InputRange *range)
{
    return (
        range->x_range.end > range->x_range.start
        && range->m_range.end > range->m_range.start
        && range->a_range.end > range->a_range.start
        && range->s_range.end > range->s_range.start
    );
}

struct InputRangeStack *InputRangeStack_create(size_t start_capacity)
{
    struct InputRangeStack *irs;
    if (start_capacity < MIN_STACK) start_capacity = MIN_STACK;
    irs = malloc(sizeof(*irs));
    if (!irs) {
        perror("malloc");
        puts("Failed to allocate InputRangeStack");
        return NULL;
    }
    irs->stack = malloc(start_capacity * sizeof(*(irs->stack)));
    if (!irs->stack) {
        perror("malloc");
        puts("Failed to allocate InputRangeStack->stack");
        free(irs);
        return NULL;
    }
    irs->length = 0;
    irs->capacity = start_capacity;
    return irs;
}

int InputRangeStack_grow(struct InputRangeStack *irs)
{
    size_t new_capacity;
    struct InputRange *temp;
    new_capacity = irs->capacity << 1;
    temp = realloc(irs->stack, new_capacity * sizeof(*(irs->stack)));
    if (!temp) {
        perror("realloc");
        puts("Failed to grow InputRangeStack");
        return 0;
    }
    irs->stack = temp;
    irs->capacity = new_capacity;
    return 1;
}

int InputRangeStack_shrink(struct InputRangeStack *irs)
{
    size_t new_capacity;
    struct InputRange *temp;
    new_capacity = irs->capacity >> 1;
    if (new_capacity < MIN_STACK) new_capacity = MIN_STACK;
    temp = realloc(irs->stack, new_capacity * sizeof(*(irs->stack)));
    if (!temp) {
        perror("realloc");
        puts("Failed to shrink InputRangeStack");
        return 0;
    }
    irs->stack = temp;
    irs->capacity = new_capacity;
    return 1;
}

int InputRangeStack_push(
    struct InputRangeStack *irs, struct InputRange range
)
{
    if (irs->length + 1 >= irs->capacity)
        if (!InputRangeStack_grow(irs)) return 0;
    irs->stack[irs->length] = range;
    ++(irs->length);
    return 1;
}

int InputRangeStack_pop(
    struct InputRangeStack *irs, struct InputRange *range
)
{
    if (!irs->length) return STACK_EMPTY;
    if (
        irs->length - 1 <= irs->capacity >> 2
        && irs->capacity > MIN_STACK
    )
        if (!InputRangeStack_shrink(irs)) return 0;
    *range = irs->stack[irs->length - 1];
    --(irs->length);
    return 1;
}

void InputRangeStack_free(struct InputRangeStack *irs)
{
    free(irs->stack);
    irs->stack = NULL;
    free(irs);
}

struct CharBuffer *CharBuffer_create(size_t start_capacity)
{
    if (start_capacity < MIN_CHARBUFFER) start_capacity = MIN_CHARBUFFER;
    struct CharBuffer *cb;
    cb = malloc(sizeof(*cb));
    if (!cb) {
        perror("malloc");
        puts("Failed to allocate CharBuffer");
        return NULL;
    }
    cb->buffer = malloc(start_capacity * sizeof(*(cb->buffer)));
    if (!cb->buffer) {
        perror("malloc");
        puts("Failed to allocate CharBuffer buffer");
        free(cb);
        return NULL;
    }
    cb->capacity = start_capacity;
    cb->length = 0;
    return cb;
}

int CharBuffer_grow(struct CharBuffer *cb)
{
    size_t new_capacity;
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
    if (cb->capacity == MIN_CHARBUFFER) return 1;
    size_t new_capacity;
    char *temp;
    new_capacity = cb->capacity >> 1;
    if (new_capacity < MIN_CHARBUFFER) new_capacity = MIN_CHARBUFFER;
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

int CharBuffer_load_from_file(struct CharBuffer *cb)
{
    int c;
    for (
        cb->length = 0;
        (c = getchar()) != EOF
        && c != '\n';
        ++(cb->length)
    ) {
        if (cb->length + 1 >= cb->capacity)
            if (!CharBuffer_grow(cb)) return 0;
        cb->buffer[cb->length] = c;
    }
    if (cb->length + 1 >= cb->capacity && !CharBuffer_grow(cb)) return 0;
    cb->buffer[cb->length] = '\0';
    while (cb->length <= cb->capacity >> 2 && cb->capacity > MIN_CHARBUFFER)
        if (!CharBuffer_shrink(cb)) return 0;
    return cb->length > 0;
}

void CharBuffer_free(struct CharBuffer *cb)
{
    free(cb->buffer);
    cb->buffer = NULL;
    free(cb);
}

int str_contains(char *str, char token)
{
    char *p_str;
    p_str = str;
    while (*p_str)
        if (*(p_str++) == token)
            return 1;
    return 0;
}

int str_offset_of(char *str, char token)
{
    char *p_str;
    p_str = str;
    while (*p_str) {
        if (*p_str == token)
            return p_str - str;
        ++p_str;
    }
    return -1;
}

int WorkflowRule_from_string(struct WorkflowRule *rule, char *str)
{
    int is_default, success, offset;
    char *dest, *temp;
    is_default = !str_contains(str, ':');
    if (is_default) {
        dest = malloc((strlen(str) + 1) * sizeof(*dest));
        if (!dest) {
            perror("malloc");
            puts("Failed to allocate dest string");
            return 0;
        }
        strcpy(dest, str);
        *rule = (struct WorkflowRule) {
            .variable = '\0',
            .operator = '\0',
            .value = 0,
            .dest = dest
        };
        return 1;
    } else {
        success = sscanf(
            str, "%c%c%zu:%*s",
            &(rule->variable),
            &(rule->operator),
            &(rule->value)
        );
        if (!success) return 0;
        offset = str_offset_of(str, ':');
        if (offset == -1) return 0;
        temp = str + offset + 1;
        dest = malloc((strlen(temp) + 1) * sizeof(*dest));
        if (!dest) {
            perror("malloc");
            puts("Failed to allocate dest string");
            return 0;
        }
        strcpy(dest, temp);
        rule->dest = dest;
        return 1;
    }
    return 0;
}

int WorkflowRule_is_default(struct WorkflowRule *rule)
{
    return (
        rule->variable == '\0'
        && rule->operator == '\0'
        && rule->value == 0
    );
}

int WorkflowRule_is_accept(struct WorkflowRule *rule)
{
    return strcmp(rule->dest, ACCEPTED) == 0;
}

struct Workflow *Workflow_create(size_t start_capacity)
{
    struct Workflow *wf;
    if (start_capacity < MIN_WORKFLOW) start_capacity = MIN_WORKFLOW;
    wf = malloc(sizeof(*wf));
    if (!wf) {
        perror("malloc");
        puts("Failed to allocate Workflow");
        return NULL;
    }
    wf->rules = malloc(start_capacity * sizeof(*(wf->rules)));
    if (!wf->rules) {
        perror("malloc");
        puts("Failed to allocate WorkflowRule array");
        free(wf);
        return NULL;
    }
    wf->identifier = NULL;
    wf->length = 0;
    wf->capacity = start_capacity;
    return wf;
}

int Workflow_grow(struct Workflow *wf)
{
    size_t new_capacity;
    struct WorkflowRule *temp;
    new_capacity = wf->capacity << 1;
    temp = realloc(wf->rules, new_capacity * sizeof(*(wf->rules)));
    if (!temp) {
        perror("relloc");
        puts("Failed to grow Workflow");
        return 0;
    }
    wf->rules = temp;
    wf->capacity = new_capacity;
    return 1;
}

int Workflow_shrink(struct Workflow *wf)
{
    size_t new_capacity;
    struct WorkflowRule *temp;
    new_capacity = wf->capacity >> 1;
    if (new_capacity < MIN_WORKFLOW) new_capacity = MIN_WORKFLOW;
    temp = realloc(wf->rules, new_capacity * sizeof(*(wf->rules)));
    if (!temp) {
        perror("relloc");
        puts("Failed to shrink Workflow");
        return 0;
    }
    wf->rules = temp;
    wf->capacity = new_capacity;
    return 1;
}

int Workflow_insert(struct Workflow *wf, struct WorkflowRule rule)
{
    if (wf->length + 1 >= wf->capacity)
        if (!Workflow_grow(wf)) return 0;
    wf->rules[wf->length] = rule;
    ++(wf->length);
    return 1;
}

int Workflow_get_rules(struct Workflow *wf, char *str)
{
    int offset;
    char *p_line, *rule_str;
    struct WorkflowRule rule;
    offset = str_offset_of(str, '{');
    if (offset == -1) {
        puts("Failed to parse Workflow");
        return 0;
    }
    p_line = str + offset + 1;
    while (*p_line) {
        offset = str_offset_of(p_line, ',');
        if (offset == -1)
            offset = str_offset_of(p_line, '}');
        if (offset == -1) {
            puts("Failed to parse Workflow");
            return 0;
        }
        rule_str = malloc((offset + 1) * sizeof(*rule_str));
        if (!rule_str) {
            puts("Failed to allocate rule string");
            return 0;
        }
        memcpy(rule_str, p_line, offset);
        rule_str[offset] = '\0';
        if (!WorkflowRule_from_string(&rule, rule_str)) {
            puts("Failed to parse WorkflowRule");
            free(rule_str);
            return 0;
        }
        free(rule_str);
        if (!Workflow_insert(wf, rule)) {
            puts("Failed to insert new rule into Workflow");
            return 0;
        }
        p_line = p_line + offset + 1;
    }
    return 1;
}

int Workflow_from_string(struct Workflow *wf, char *str)
{
    int offset;
    char *identifier;
    offset = str_offset_of(str, '{');
    if (offset == -1) {
        puts("Invalid Workflow string");
        return 0;
    }
    if (!Workflow_get_rules(wf, str)) {
        puts("Failed to parse rules from string");
        return 0;
    }
    identifier = malloc((offset + 1) * sizeof(*identifier));
    if (!identifier) {
        perror("malloc");
        puts("Failed to allocate identifier string");
        return 0;
    }
    strncpy(identifier, str, offset + 1);
    identifier[offset] = '\0';
    wf->identifier = identifier;
    return 1;
}

size_t hash_identifier(char *identifier, size_t max_hash_size)
{
    size_t hash, count;
    char *str;
    char c;

    hash = 5381;
    count = HASH_REPEAT;
    while (count) {
        str = identifier;
        while ((c = *str++))
            hash = ((hash << 5) + hash) + c;
        --count;
    }
    return hash % max_hash_size;
}

void Workflow_free(struct Workflow *wf)
{
    size_t i;
    for (i = 0; i < wf->length; ++i)
        free(wf->rules[i].dest);
    free(wf->rules);
    wf->rules = NULL;
    free(wf->identifier);
    wf->identifier = NULL;
    free(wf);
}

struct WorkflowCache *WorkflowCache_create(size_t capacity)
{
    size_t i;
    struct WorkflowCache *p_cache;
    if (capacity < MIN_CACHE) capacity = MIN_CACHE;
    p_cache = malloc(sizeof(*p_cache));
    if (!p_cache) {
        perror("malloc");
        puts("Failed to allocate WorkflowCache");
        return NULL;
    }
    p_cache->cache = malloc(capacity * sizeof(*(p_cache->cache)));
    if (!p_cache->cache) {
        perror("malloc");
        puts("Failed to allocate WorflowCache->Cache");
        free(p_cache);
        return NULL;
    }
    for (i = 0; i < capacity; ++i)
        p_cache->cache[i] = NULL;
    p_cache->capacity = capacity;
    p_cache->collisions = 0;
    return p_cache;
}

int WorkflowCache_store(
    struct WorkflowCache *p_cache, struct Workflow *wf
)
{
    size_t hash, i;
    hash = hash_identifier(wf->identifier, p_cache->capacity);
    if (p_cache->cache[hash]) ++(p_cache->collisions);
    for (i = hash; i < p_cache->capacity; ++i) {
        if (!p_cache->cache[i]) {
            p_cache->cache[i] = wf;
            return 1;
        }
        if (strcmp(p_cache->cache[i]->identifier, wf->identifier) == 0) {
            puts(
                "Workflow with this identifier "
                "already exists in the cache"
            );
            return 0;
        }
    }
    for (i = 0; i < hash; ++i) {
        if (!p_cache->cache[i]) {
            p_cache->cache[i] = wf;
            return 1;
        }
        if (strcmp(p_cache->cache[i]->identifier, wf->identifier) == 0) {
            puts(
                "Workflow with this identifier "
                "already exists in the cache"
            );
            return 0;
        }
    }
    puts("Cache full");
    return 0;
}

struct Workflow *WorkflowCache_retrieve(
    struct WorkflowCache *p_cache, char *identifier
)
{
    size_t hash, i;
    hash = hash_identifier(identifier, p_cache->capacity);
    for (i = hash; i < p_cache->capacity; ++i)
        if (!p_cache->cache[i])
            return NULL;
        else if (strcmp(p_cache->cache[i]->identifier, identifier) == 0)
            return p_cache->cache[i];
    for (i = 0; i < hash; ++i)
        if (!p_cache->cache[i])
            return NULL;
        else if (strcmp(p_cache->cache[i]->identifier, identifier) == 0)
            return p_cache->cache[i];
    return NULL;
}

void WorkflowCache_free(struct WorkflowCache *p_cache)
{
    size_t i;
    for (i = 0; i < p_cache->capacity; ++i)
        if (p_cache->cache[i])
            Workflow_free(p_cache->cache[i]);
    free(p_cache->cache);
    p_cache->cache = NULL;
    free(p_cache);
}

int is_accepted(struct InputRange *range)
{
    return strcmp(range->wf_identifier, ACCEPTED) == 0;
}

int is_rejected(struct InputRange *range)
{
    return strcmp(range->wf_identifier, REJECTED) == 0;
}

struct InputRange cleave(
    struct InputRange *range, struct WorkflowRule *rule
)
{
    struct InputRange rejected_range;
    size_t *cleaved, *rejected_variable, new, rejected;
    if (WorkflowRule_is_default(rule))
        return (struct InputRange) { 0 };
    rejected_range = *range;
    rejected = rule->value;
    cleaved = rejected_variable = NULL;
    new = 0;
    switch (rule->operator) {
    case LESS_THAN:
        switch (rule->variable) {
        case 'x':
            cleaved = &(range->x_range.end);
            rejected_variable = &(rejected_range.x_range.start);
            new = rejected - 1;
            break;
        case 'm':
            cleaved = &(range->m_range.end);
            rejected_variable = &(rejected_range.m_range.start);
            new = rejected - 1;
            break;
        case 'a':
            cleaved = &(range->a_range.end);
            rejected_variable = &(rejected_range.a_range.start);
            new = rejected - 1;
            break;
        case 's':
            cleaved = &(range->s_range.end);
            rejected_variable = &(rejected_range.s_range.start);
            new = rejected - 1;
            break;
        }
        break;
    case GREATER_THAN:
        switch (rule->variable) {
        case 'x':
            cleaved = &(range->x_range.start);
            rejected_variable = &(rejected_range.x_range.end);
            new = rejected + 1;
            break;
        case 'm':
            cleaved = &(range->m_range.start);
            rejected_variable = &(rejected_range.m_range.end);
            new = rejected + 1;
            break;
        case 'a':
            cleaved = &(range->a_range.start);
            rejected_variable = &(rejected_range.a_range.end);
            new = rejected + 1;
            break;
        case 's':
            cleaved = &(range->s_range.start);
            rejected_variable = &(rejected_range.s_range.end);
            new = rejected + 1;
            break;
        }
        break;
    default:
        puts("Got invalid operator");
        return (struct InputRange) { 0 };
    }
    if (!cleaved || !rejected_variable) {
        puts("Invalid rule");
        return (struct InputRange) { 0 };
    }
    *cleaved = new;
    *rejected_variable = rejected;
    return rejected_range;
}

int count_possibilities(
    struct WorkflowCache *p_cache,
    struct InputRangeStack *irs,
    size_t *total
)
{
    struct InputRange range, rejected;
    struct Workflow *wf;
    size_t rule;
    int pop_result;
    pop_result = InputRangeStack_pop(irs, &range);
    if (pop_result == STACK_EMPTY) return 1;
    if (!pop_result) {
        puts("Failed to walk the tree");
        return 0;
    }
    if (is_rejected(&range))
        return count_possibilities(p_cache, irs, total);
    if (is_accepted(&range)) {
        *total = *total + (
            (range.x_range.end - range.x_range.start + 1)
            * (range.m_range.end - range.m_range.start + 1)
            * (range.a_range.end - range.a_range.start + 1)
            * (range.s_range.end - range.s_range.start + 1)
        );
        return count_possibilities(p_cache, irs, total);
    }
    wf = WorkflowCache_retrieve(p_cache, range.wf_identifier);
    if (!wf) {
        puts("Invalid Workflow");
        return 0;
    }
    for (rule = 0; rule < wf->length; ++rule) {
        rejected = cleave(&range, wf->rules + rule);
        range.wf_identifier = wf->rules[rule].dest;
        if (InputRange_is_valid(&range)) {
            if (!InputRangeStack_push(irs, range)) {
                puts("Stack failure");
                return 0;
            }
        }
        range = rejected;
    }
    return count_possibilities(p_cache, irs, total);
}

size_t get_line(struct CharBuffer *cb)
{
    if (!CharBuffer_load_from_file(cb)) return 0;
    return cb->length;
}

int parse_workflows(struct WorkflowCache *p_cache)
{
    struct CharBuffer *cb;
    struct Workflow *wf;
    struct InputRangeStack *irs;
    struct InputRange start_range;
    size_t total;
    cb = CharBuffer_create(0);
    while (get_line(cb)) {
        wf = Workflow_create(0);
        if (!wf) {
            CharBuffer_free(cb);
            return 0;
        }
        if (!Workflow_from_string(wf, cb->buffer)) {
            Workflow_free(wf);
            CharBuffer_free(cb);
            return 0;
        }
        if (!WorkflowCache_store(p_cache, wf)) {
            Workflow_free(wf);
            CharBuffer_free(cb);
            return 0;
        }
    }
    CharBuffer_free(cb);
    irs = InputRangeStack_create(0);
    if (!irs) return 0;
    start_range = (struct InputRange) {
        .wf_identifier = START_WORKFLOW,
        .x_range = (struct Range) {
            .start = RANGE_START,
            .end = RANGE_END
        },
        .m_range = (struct Range) {
            .start = RANGE_START,
            .end = RANGE_END
        },
        .a_range = (struct Range) {
            .start = RANGE_START,
            .end = RANGE_END
        },
        .s_range = (struct Range) {
            .start = RANGE_START,
            .end = RANGE_END
        }
    };
    if (!InputRangeStack_push(irs, start_range)) {
        InputRangeStack_free(irs);
        return 0;
    }
    total = 0;
    if (!count_possibilities(p_cache, irs, &total)) {
        InputRangeStack_free(irs);
        return 0;
    }
    printf("Total = %zu\n", total);
    InputRangeStack_free(irs);
    return 1;
}

int parse(void)
{
    struct WorkflowCache *p_cache;
    p_cache = WorkflowCache_create(0);
    if (!p_cache) return 0;
    if (!parse_workflows(p_cache)) {
        WorkflowCache_free(p_cache);
        return 0;
    }
    printf("Collisions = %zu\n", p_cache->collisions);
    WorkflowCache_free(p_cache);
    return 1;
}

int main(void)
{
    if (!parse()) return 1;
    return 0;
}
