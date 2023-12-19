#include <stdio.h>

#define MAX_HANDS 1024
#define CARDS_COUNT 5
#define NUM_UNIQUE_CARDS 13
#define MAX_LINE 16

struct Hand {
    char cards[CARDS_COUNT + 1];
    unsigned int bid;
};

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int get_card_strength(char card)
{
    switch (card)
    {
    case '2':
        return 0;
    case '3':
        return 1;
    case '4':
        return 2;
    case '5':
        return 3;
    case '6':
        return 4;
    case '7':
        return 5;
    case '8':
        return 6;
    case '9':
        return 7;
    case 'T':
        return 8;
    case 'J':
        return 9;
    case 'Q':
        return 10;
    case 'K':
        return 11;
    case 'A':
        return 12;
    default:
        return -1;
    }
}

void count_cards(char *cards, size_t counts[], size_t counts_len)
{
    size_t i;
    char **p_cards;

    for (i = 0; i < counts_len; ++i)
        counts[i] = 0;

    p_cards = &cards;
    while (**p_cards) {
        ++(counts[get_card_strength(**p_cards)]);
        ++(*p_cards);
    }
}

int is_n_of_a_kind(char *cards, size_t num_unique, size_t n)
{
    size_t i, counts[num_unique];
    
    count_cards(cards, counts, num_unique);

    size_t count_n, count_other;

    count_n = count_other = 0;
    for (i = 0; i < num_unique; ++i)
        if (counts[i] == n)
            ++count_n;
        else if (counts[i] > 1)
            ++count_other;

    return count_n == 1 && count_other == 0;
}

int is_five_of_a_kind(char* cards, size_t num_unique)
{
    return is_n_of_a_kind(cards, num_unique, 5);
}

int is_four_of_a_kind(char* cards, size_t num_unique)
{
    return is_n_of_a_kind(cards, num_unique, 4);
}

int is_full_house(char *cards, size_t num_unique)
{
    size_t i, counts[num_unique];
    
    count_cards(cards, counts, num_unique);

    size_t count_triple, count_double;

    count_double = count_triple = 0;
    for (i = 0; i < num_unique; ++i)
        if (counts[i] == 2)
            ++count_double;
        else if (counts[i] == 3)
            ++count_triple;

    return count_double == 1 && count_triple == 1;
}

int is_three_of_a_kind(char *cards, size_t num_unique)
{
    return is_n_of_a_kind(cards, num_unique, 3);
}

int is_two_pairs(char *cards, size_t num_unique)
{
    size_t i, counts[num_unique];
    
    count_cards(cards, counts, num_unique);

    size_t count_other, count_double;

    count_double = count_other = 0;
    for (i = 0; i < num_unique; ++i)
        if (counts[i] == 2)
            ++count_double;
        else if (counts[i] > 1)
            ++count_other;

    return count_double == 2 && count_other == 0;
}

int is_one_pair(char *cards, size_t num_unique)
{
    return is_n_of_a_kind(cards, num_unique, 2);
}

int is_high_card(char *cards, size_t num_unique)
{
    size_t i, counts[num_unique];
    
    count_cards(cards, counts, num_unique);

    size_t count;

    count = 0;
    for (i = 0; i < num_unique; ++i)
        if (counts[i] > 1)
            ++count;

    return !count;
}

int get_type_strength(char *cards, size_t num_unique)
{
    if (is_five_of_a_kind(cards, num_unique))
        return 6;
    else if (is_four_of_a_kind(cards, num_unique))
        return 5;
    else if (is_full_house(cards, num_unique))
        return 4;
    else if (is_three_of_a_kind(cards, num_unique))
        return 3;
    else if (is_two_pairs(cards, num_unique))
        return 2;
    else if (is_one_pair(cards, num_unique))
        return 1;
    else if (is_high_card(cards, num_unique))
        return 0;
    return -1;
}

int cmp_same_type(char *cards, char *other)
{
    char **p_cards, **p_other;
    int cards_strength, other_strength;

    p_cards = &cards;
    p_other = &other;

    while (
        **p_cards && **p_other
        && **p_cards == **p_other
    ) {
        ++(*p_cards);
        ++(*p_other);
    }

    if (**p_cards && **p_other) {
        cards_strength = get_card_strength(**p_cards);
        other_strength = get_card_strength(**p_other);
        if (cards_strength > other_strength)
            return 1;
        else if (cards_strength < other_strength)
            return -1;
        else
            return 0;
    } else if (**p_cards && !**p_other) {
        return 1;
    } else if (!**p_cards && **p_other) {
        return -1;
    } else {
        return 0;
    }
}

void bubble_sort_hands(
    struct Hand hands[], size_t num_hands, size_t num_unique
)
{
    if (!num_hands) return;
    size_t swaps, i;
    struct Hand temp;
    int this_hand_type_strength, last_hand_type_strength;

    do {
        swaps = 0;
        last_hand_type_strength = get_type_strength(
            hands[0].cards, num_unique
        );
        for (i = 1; i < num_hands; ++i) {
            this_hand_type_strength = get_type_strength(
                hands[i].cards, num_unique
            );
            if (last_hand_type_strength > this_hand_type_strength) {
                temp = hands[i - 1];
                hands[i - 1] = hands[i];
                hands[i] = temp;
                ++swaps;
            } else if (this_hand_type_strength == last_hand_type_strength) {
                if (cmp_same_type(hands[i - 1].cards, hands[i].cards) > 0) {
                    temp = hands[i - 1];
                    hands[i - 1] = hands[i];
                    hands[i] = temp;
                    ++swaps;
                }
            } else {
                last_hand_type_strength = this_hand_type_strength;
            }
        }
    } while (swaps);
}

size_t get_line(char line[], size_t max_line)
{
    size_t i;
    int c;
    for (
        i = 0;
        (c = getchar()) != EOF
        && c != '\n'
        && i < max_line - 1;
        ++i
    )
        line[i] = c;
    line[i] = '\0';
    if (c != EOF && c != '\n')
        puts("WARNING: Could not read entire line into line buffer");
    return i;
}

unsigned int parse_number(char *str)
{
    unsigned int number, multiplier;
    char **p_str, *base;

    base = str;
    p_str = &str;
    while (**p_str && is_digit(**p_str))
        ++(*p_str);
    if (!**p_str || !is_digit(**p_str))
        --(*p_str);

    number = 0;
    multiplier = 1;
    while (*p_str != base) {
        number = number + multiplier * (unsigned int)(**p_str - '0');
        multiplier = multiplier * 10;
        --(*p_str);
    }
    number = number + multiplier * (unsigned int)(**p_str - '0');
    return number;
}

struct Hand parse_line(char line[], size_t line_length)
{
    size_t i;
    struct Hand hand = {0};
    for (i = 0; i < CARDS_COUNT; ++i)
        hand.cards[i] = line[i];
    hand.cards[i] = '\0';
    while (!is_digit(line[i]) && i < line_length)
        ++i;
    hand.bid = parse_number(line + i);
    return hand;
}

int main(void)
{
    char line[MAX_LINE];
    size_t line_length, num_hands, i, total;
    struct Hand hands[MAX_HANDS];

    num_hands = 0;
    while ((line_length = get_line(line, MAX_LINE)))
        hands[num_hands++] = parse_line(line, line_length);

    bubble_sort_hands(hands, num_hands, NUM_UNIQUE_CARDS);

    total = 0;
    for (i = 0; i < num_hands; ++i)
        total = total + (i + 1) * hands[i].bid;

    printf("total = %zu\n", total);
    
    return 0;
}
