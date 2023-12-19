#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define BUFF_SIZE 64
#define NUM_BOXES 256

#define STORE '='
#define REMOVE '-'

struct Node {
    char *label;
    size_t focal_length;
    struct Node *next;
};

void Node_free_single(struct Node *node)
{
    if (!node) return;
    free(node->label);
    free(node);
}

void Node_free(struct Node *node)
{
    if (!node) return;
    struct Node *last;
    last = node;
    node = node->next;
    while (node) {
        Node_free_single(last);
        last = node;
        node = node->next;
    }
    Node_free_single(last);
}

int is_digit(char c)
{
    return '0' <= c && c <= '9';
}

size_t parse_number(char *str)
{
    if (!is_digit(*str)) return 0;
    char *base;
    size_t number, multiplier;
    base = str;
    while (*str && is_digit(*str)) ++str;
    if (!*str) --str;
    number = 0;
    multiplier = 1;
    while (str != base) {
        number = number + multiplier * (size_t)(*str - '0');
        --str;
    }
    number = number + multiplier * (size_t)(*str - '0');
    return number;
}

size_t get_next_step(char buff[], size_t buff_size)
{
    size_t i;
    int c;
    for (
        i = 0;
        (c = getchar()) != EOF
        && c != '\n'
        && c != ','
        && i < buff_size;
        ++i
    )
        buff[i] = (char)c;
    if (i == buff_size)
        puts("WARNING: Could not read entire step into buffer");
    buff[i] = '\0';
    return i;
}

char get_operation(char *str)
{
    while (*str) {
        switch (*str) {
        case STORE:
            return STORE;
        case REMOVE:
            return REMOVE;
        default:
            ++str;
            break;
        }
    }
    return '\0';
}

size_t hash_single_char(char c, size_t last_hash, size_t array_size)
{
    last_hash = last_hash + (size_t)c;
    last_hash = (last_hash << 4) + last_hash;
    return last_hash % array_size;
}

size_t hash_string(char *str, size_t array_size)
{
    size_t hash;
    hash = 0;
    while (*str && *str != STORE && *str != REMOVE) {
        hash = hash_single_char(*str, hash, array_size);
        ++str;
    }
    return hash;
}

void execute_instruction(char *buff, struct Node *boxes[], size_t num_boxes)
{
    size_t hash, i;
    struct Node *current, *last;

    hash = hash_string(buff, num_boxes);
    switch (get_operation(buff)) {
    case REMOVE:
        for (i = 0; buff[i] && buff[i] != REMOVE; ++i);
        buff[i] = '\0';
        current = boxes[hash];
        if (!current) break;
        last = NULL;
        while (current) {
            if (strcmp(current->label, buff) == 0) {
                if (!last) {
                    boxes[hash] = current->next;
                } else {
                    last->next = current->next;
                }
                Node_free_single(current);
                break;
            }
            last = current;
            current = current->next;
        }
        break;
    case STORE:
        for (i = 0; buff[i] && buff[i] != STORE; ++i);
        buff[i++] = '\0';
        i = parse_number(buff + i);
        current = boxes[hash];
        last = NULL;
        while (current && strcmp(current->label, buff) != 0) {
            last = current;
            current = current->next;
        }
        if (current) {
            current->focal_length = i;
        } else {
            current = malloc(sizeof(*current));
            if (!current) {
                perror("malloc");
                break;
            }
            current->focal_length = i;
            current->next = NULL;
            i = strlen(buff) + 1;
            current->label = malloc(i);
            if (!current->label) {
                perror("malloc");
                Node_free_single(current);
                break;
            }
            memcpy(current->label, buff, i);
            if (!last) {
                boxes[hash] = current;
            } else {
                last->next = current;
            }
        }
        break;
    default:
        puts("ERROR: Got invalid operation");
        break;
    }
}

size_t get_focusing_power(struct Node *boxes[], size_t num_boxes)
{
    size_t total, i, j;
    struct Node *node;

    for (i = total = 0; i < num_boxes; ++i) {
        node = boxes[i];
        j = 1;
        while (node) {
            total = total + (1 + i) * j * node->focal_length;
            node = node->next;
            ++j;
        }
    }
    return total;
}

int main(void)
{
    char buff[BUFF_SIZE];
    struct Node *boxes[NUM_BOXES];
    size_t total, i;

    for (i = 0; i < NUM_BOXES; ++i)
        boxes[i] = NULL;

    while (get_next_step(buff, BUFF_SIZE)) {
        execute_instruction(buff, boxes, NUM_BOXES);
    }
    total = get_focusing_power(boxes, NUM_BOXES);
    printf("Total = %zu\n", total);
    for (i = 0; i < NUM_BOXES; ++i)
        Node_free(boxes[i]);
    return 0;
}
