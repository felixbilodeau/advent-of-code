#include <stdio.h>

#define MAX_HANDS 1024
#define CARDS_COUNT 5
#define NUM_UNIQUE_CARDS 13
#define MAX_LINE 16
#define JOKER 'J'

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
    case 'J':
        return 0;
    case '2':
        return 1;
    case '3':
        return 2;
    case '4':
        return 3;
    case '5':
        return 4;
    case '6':
        return 5;
    case '7':
        return 6;
    case '8':
        return 7;
    case '9':
        return 8;
    case 'T':
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

int is_n_of_a_kind(size_t counts[], size_t num_unique, size_t n)
{
    size_t i, num_jokers;
    int joker_strength;

    joker_strength = get_card_strength(JOKER);
    num_jokers = counts[joker_strength];
    if (!num_jokers) {
        for (i = 0; i < num_unique; ++i) {
            if (counts[i] == n)
                return 1;
        }
    } else {
        if (num_jokers >= n)
            return 1;
        for (i = 0; i < num_unique; ++i)
            if (i != (size_t)joker_strength && counts[i] + num_jokers >= n)
                return 1;
    }
    return 0;
}

int is_five_of_a_kind(size_t counts[], size_t num_unique)
{
    return is_n_of_a_kind(counts, num_unique, 5);
}

int is_four_of_a_kind(size_t counts[], size_t num_unique)
{
    return is_n_of_a_kind(counts, num_unique, 4);
}

int is_full_house(size_t counts[], size_t num_unique)
{
    size_t i, count_triple, count_double, num_jokers;
    int joker_strength;

    joker_strength = get_card_strength(JOKER);
    num_jokers = counts[joker_strength];

    count_double = count_triple = 0;
    for (i = 0; i < num_unique; ++i) {
        if (i != (size_t)joker_strength && counts[i] == 2)
            ++count_double;
        else if (i != (size_t)joker_strength && counts[i] == 3)
            ++count_triple;
    }

    switch (num_jokers) {
    case 0:
        return count_double == 1 && count_triple == 1;
    case 1:
        return count_double == 2;
    case 2:
        return count_double == 1;
    case 3:
    case 4:
    case 5:
        return 1;
    default:
        return 0;
    }
}

int is_three_of_a_kind(size_t counts[], size_t num_unique)
{
    return is_n_of_a_kind(counts, num_unique, 3);
}

int is_two_pairs(size_t counts[], size_t num_unique)
{
    size_t i, count_double, num_jokers;
    int joker_strength;

    joker_strength = get_card_strength(JOKER);
    num_jokers = counts[joker_strength];

    count_double = 0;
    for (i = 0; i < num_unique; ++i)
        if (i != (size_t)joker_strength && counts[i] == 2)
            ++count_double;

    switch (num_jokers) {
    case 0:
        return count_double == 2;
    case 1:
        return count_double == 1;
    case 2:
    case 3:
    case 4:
    case 5:
        return 1;
    default:
        return 0;
    }

    return count_double == 2;
}

int is_one_pair(size_t counts[], size_t num_unique)
{
    return is_n_of_a_kind(counts, num_unique, 2);
}

int is_high_card(size_t counts[], size_t num_unique)
{
    size_t i, count, num_jokers;
    int joker_strength;

    joker_strength = get_card_strength(JOKER);
    num_jokers = counts[joker_strength];

    count = 0;
    for (i = 0; i < num_unique; ++i)
        if (i != (size_t)joker_strength && counts[i] > 1)
            ++count;

    return !count && !num_jokers;
}

int get_type_strength(size_t counts[], size_t num_unique)
{
    if (is_five_of_a_kind(counts, num_unique))
        return 6;
    else if (is_four_of_a_kind(counts, num_unique))
        return 5;
    else if (is_full_house(counts, num_unique))
        return 4;
    else if (is_three_of_a_kind(counts, num_unique))
        return 3;
    else if (is_two_pairs(counts, num_unique))
        return 2;
    else if (is_one_pair(counts, num_unique))
        return 1;
    else if (is_high_card(counts, num_unique))
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
    size_t swaps, i, counts[num_unique];
    struct Hand temp;
    int this_hand_type_strength, last_hand_type_strength;

    do {
        swaps = 0;
        count_cards(hands[0].cards, counts, num_unique);
        last_hand_type_strength = get_type_strength(counts, num_unique);
        for (i = 1; i < num_hands; ++i) {
            count_cards(hands[i].cards, counts, num_unique);
            this_hand_type_strength = get_type_strength(counts, num_unique);
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
