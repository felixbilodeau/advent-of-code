#include <stdio.h>
#include <string.h>
#include <assert.h>

#define NODE_CODE_LEN 4
#define MAX_NODES 4096 * 8
#define MAX_LINE 32
#define MAX_LRS 300
#define START_NODE_CODE "AAA"
#define END_NODE_CODE "ZZZ"

struct Node {
    char self[NODE_CODE_LEN];
    char left[NODE_CODE_LEN];
    char right[NODE_CODE_LEN];
};

size_t get_line(char line[], size_t max_line)
{
    size_t i;
    int c;

    for (
        i = 0;
        i < max_line - 1
        && (c = getchar()) != EOF
        && c != '\n';
        ++i
    )
        line[i] = c;

    if (i == max_line - 1)
        puts("WARNING: Could not load entire line into buffer");
    line[i] = '\0';
    return i;
}

struct Node parse_node(char line[MAX_LINE])
{
    /* strcture: AAA = (AAA, AAA) */
    /* --------- 0123456789012345 */
    struct Node node;

    node = (struct Node){0};

    node.self[0] = line[0];
    node.self[1] = line[1];
    node.self[2] = line[2];
    node.self[3] = '\0';

    node.left[0] = line[7];
    node.left[1] = line[8];
    node.left[2] = line[9];
    node.left[3] = '\0';

    node.right[0] = line[12];
    node.right[1] = line[13];
    node.right[2] = line[14];
    node.right[3] = '\0';

    return node;
}

int isupper(int c)
{
    return c >= 'A' && c <= 'Z';
}

size_t hash(const char *word, size_t array_length)
{
    size_t hash = 5381;
    int c;

    while ((c = *word++))        /* str++ is going to the next address in memory, where the next char in the string is stored */
    {
        if (isupper(c))
        {
            c = c + 32;
        }

        hash = ((hash << 5) + hash) + c; /* hash * 33 + c   // hash << 5 = hash * 2^5 */
    }

    return hash % array_length;
}

struct Node find_node(
    struct Node nodes[], size_t nodes_max_len, char code[NODE_CODE_LEN]
)
{
    size_t i, start;

    start = hash(code, nodes_max_len);
    if (!strncmp(nodes[start].self, code, NODE_CODE_LEN))
        return nodes[start];
    
    if (start + 1 >= nodes_max_len) i = 0;
    else i = start + 1;
    while (i != start) {
        if (!strncmp(nodes[i].self, code, NODE_CODE_LEN))
            return nodes[i];
        if (++i >= nodes_max_len)
            i = 0;
    }
    puts("Could not find node!");
    return nodes[0];
}

int main(void)
{
    struct Node nodes[MAX_NODES], current_node;
    char lrs[MAX_LRS], line[MAX_LINE];
    size_t lrs_length, line_length, i, start, total, collisions;

    for (i = 0; i < MAX_NODES; ++i)
        nodes[i] = (struct Node){ 0 };

    lrs_length = get_line(lrs, MAX_LRS);
    getchar();

    collisions = 0;
    while ((line_length = get_line(line, MAX_LINE)) != 0) {
        current_node = parse_node(line);
        start = hash(current_node.self, MAX_NODES);
        if (!*(nodes[start].self)) {
            nodes[start] = current_node;
        } else {
            ++collisions;
            if (start + 1 >= MAX_NODES) i = 0;
            else i = start + 1;
            while (i != start) {
                if (!*(nodes[i].self)) {
                    nodes[i] = current_node;
                    break;
                }
                if (++i >= MAX_NODES)
                    i = 0;
            }
            if (i == start) {
                printf(
                    "Could not insert %s into hash map!\n", current_node.self
                );
                assert(0);
            }
        }
    }

    printf("Collisions: %zu\n", collisions);

    i = total = 0;
    current_node = find_node(nodes, MAX_NODES, START_NODE_CODE);
    while (strncmp(current_node.self, END_NODE_CODE, NODE_CODE_LEN)) {
        switch (lrs[i]) {
        case 'L':
            current_node = find_node(nodes, MAX_NODES, current_node.left);
            break;
        case 'R':
            current_node = find_node(nodes, MAX_NODES, current_node.right);
            break;
        default:
            puts("Got an invalid Left/Right instruction!");
            assert(0);
        }
        
        if (++i >= lrs_length)
            i = 0;
        ++total;
    }
    printf("total steps = %zu\n", total);
    
    return 0;
}
