#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_CHARBUFFER  32
#define MIN_PARTBUFFER  8
#define MIN_WORKFLOW    8
#define MIN_CACHE       2048

#define REJECTED "R"
#define ACCEPTED "A"

#define LESS_THAN       '<'
#define GREATER_THAN    '>'

#define HASH_REPEAT     23

#define START_WORKFLOW "in"

static size_t collisions = 0;

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
};

struct Part {
    size_t x;
    size_t m;
    size_t a;
    size_t s;
};

struct PartBuffer {
    struct Part *parts;
    size_t length;
    size_t capacity;
};

struct CharBuffer {
    char *buffer;
    size_t length;
    size_t capacity;
};

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

struct PartBuffer *PartBuffer_create(size_t start_capacity)
{
    if (start_capacity < MIN_PARTBUFFER) start_capacity = MIN_PARTBUFFER;
    struct PartBuffer *pb;
    pb = malloc(sizeof(*pb));
    if (!pb) {
        perror("malloc");
        puts("Failed to allocate PartBuffer");
        return NULL;
    }
    pb->parts = malloc(start_capacity * sizeof(*(pb->parts)));
    if (!pb->parts) {
        perror("malloc");
        puts("Failed to allocate PartBuffer buffer");
        free(pb);
        return NULL;
    }
    pb->capacity = start_capacity;
    pb->length = 0;
    return pb;
}

int PartBuffer_grow(struct PartBuffer *pb)
{
    size_t new_capacity;
    struct Part *temp;
    new_capacity = pb->capacity << 1;
    temp = realloc(pb->parts, new_capacity * sizeof(*(pb->parts)));
    if (!temp) {
        perror("realloc");
        puts("Failed to grow PartBuffer");
        return 0;
    }
    pb->parts = temp;
    pb->capacity = new_capacity;
    return 1;
}

int PartBuffer_shrink(struct PartBuffer *pb)
{
    if (pb->capacity == MIN_PARTBUFFER) return 1;
    size_t new_capacity;
    struct Part *temp;
    new_capacity = pb->capacity >> 1;
    if (new_capacity < MIN_PARTBUFFER) new_capacity = MIN_PARTBUFFER;
    temp = realloc(pb->parts, new_capacity * sizeof(*(pb->parts)));
    if (!temp) {
        perror("realloc");
        puts("Failed to shrink PartBuffer");
        return 0;
    }
    pb->parts = temp;
    pb->capacity = new_capacity;
    return 1;
}

int PartBuffer_insert(struct PartBuffer *pb, struct Part part)
{
    if (pb->length + 1 >= pb->capacity)
        if (!PartBuffer_grow(pb)) return 0;
    pb->parts[pb->length] = part;
    ++(pb->length);
    return 1;
}

void PartBuffer_free(struct PartBuffer *pb)
{
    free(pb->parts);
    pb->parts = NULL;
    free(pb);
}

int Part_from_string(struct Part *part, const char* str)
{
    return sscanf(
        str, "{x=%zu,m=%zu,a=%zu,s=%zu}",
        &(part->x), &(part->m), &(part->a), &(part->s)
    );
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
    return p_cache;
}

int WorkflowCache_store(
    struct WorkflowCache *p_cache, struct Workflow *wf
)
{
    size_t hash, i;
    hash = hash_identifier(wf->identifier, p_cache->capacity);
    if (p_cache->cache[hash]) ++collisions;
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

size_t get_line(struct CharBuffer *cb)
{
    if (!CharBuffer_load_from_file(cb)) return 0;
    return cb->length;
}

int test_rule(struct WorkflowRule *rule, struct Part *part)
{
    if (WorkflowRule_is_default(rule)) return 1;
    size_t test_value;
    switch (rule->variable) {
    case 'x':
        test_value = part->x;
        break;
    case 'm':
        test_value = part->m;
        break;
    case 'a':
        test_value = part->a;
        break;
    case 's':
        test_value = part->s;
        break;
    default:
        puts("Invalid rule");
        return -1;
    }
    switch (rule->operator) {
    case LESS_THAN:
        return test_value < rule->value;
    case GREATER_THAN:
        return test_value > rule->value;
    default:
        puts("Invalid rule");
        return -1;
    }
    puts("Invalid rule");
    return -1;
}

char *test_workflow(struct Workflow *wf, struct Part *part)
{
    size_t i;
    int passes;
    for (i = 0; i < wf->length; ++i) {
        passes = test_rule(wf->rules + i, part);
        if (passes == -1) return REJECTED;
        if (passes) return wf->rules[i].dest;
    }
    puts("Invalid workflow");
    return REJECTED;
}

int test_part(struct WorkflowCache *p_cache, struct Part *part)
{
    char *next;
    struct Workflow *wf;
    next = START_WORKFLOW;
    while (strcmp(next, ACCEPTED) != 0 && strcmp(next, REJECTED) != 0) {
        wf = WorkflowCache_retrieve(p_cache, next);
        if (!wf) {
            puts("Missing workflow");
            return 0;
        }
        next = test_workflow(wf, part);
    }
    if (strcmp(next, ACCEPTED) == 0) return 1;
    if (strcmp(next, REJECTED) == 0) return 0;
    puts("Failed to test part");
    return -1;
}

size_t count_parts(struct WorkflowCache *p_cache, struct PartBuffer *pb)
{
    size_t total, part;
    int success;
    total = 0;
    for (part = 0; part < pb->length; ++part) {
        success = test_part(p_cache, pb->parts + part);
        if (success == -1) return 0;
        if (success == 1) {
            total = total + (
                pb->parts[part].x
                + pb->parts[part].m
                + pb->parts[part].a
                + pb->parts[part].s
            );
        }
    }
    return total;
}

int parse_workflows(struct WorkflowCache *p_cache)
{
    struct CharBuffer *cb;
    struct Workflow *wf;
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
    return 1;
}

int parse_parts(struct PartBuffer *pb)
{
    struct CharBuffer *cb;
    struct Part part;
    cb = CharBuffer_create(0);
    while (get_line(cb)) {
        if (!Part_from_string(&part, cb->buffer)) {
            CharBuffer_free(cb);
            return 0;
        }
        if (!PartBuffer_insert(pb, part)) {
            CharBuffer_free(cb);
            return 0;
        }
    }
    CharBuffer_free(cb);
    return 1;
}

int parse(void)
{
    struct PartBuffer *pb;
    struct WorkflowCache *p_cache;
    size_t total;
    p_cache = WorkflowCache_create(0);
    if (!p_cache) return 0;
    if (!parse_workflows(p_cache)) {
        WorkflowCache_free(p_cache);
        return 0;
    }
    pb = PartBuffer_create(0);
    if (!pb) {
        WorkflowCache_free(p_cache);
        return 0;
    }
    if (!parse_parts(pb)) {
        WorkflowCache_free(p_cache);
        PartBuffer_free(pb);
        return 0;
    }
    total = count_parts(p_cache, pb);
    WorkflowCache_free(p_cache);
    PartBuffer_free(pb);
    printf("Total = %zu\n", total);
    return 1;
}

int main(void)
{
    if (!parse()) return 1;
    printf("Collisions = %zu\n", collisions);
    return 0;
}
