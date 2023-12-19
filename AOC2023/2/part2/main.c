#include <stdio.h>

#define MAX_LINE 200

#define RED "red"
#define GREEN "green"
#define BLUE "blue"

struct MinRGB {
    size_t min_red;
    size_t min_green;
    size_t min_blue;
};

size_t get_first_game_index(char line[])
{
    size_t i;

    for (i = 0; line[i] && line[i] != ':'; ++i);

    if (!line[i]) {
        printf("ERROR: Could not get first game index");
        return i;
    }
    return ++i;
}

size_t get_next_game_index(char line[], size_t last_game)
{
    size_t i;

    for (i = last_game; line[i] && line[i] != ';'; ++i);
    if (line[i])
        ++i;
    return i;
}

size_t get_number(char line[], size_t start)
{
    size_t i, number, multiplier;
    for (
        i = start;
        line[i] && line[i] >= '0' && line[i] <= '9';
        ++i
    );

    multiplier = 1;
    number = 0;
    for (--i; i >= start; --i) {
        number = number + multiplier * (line[i] - (size_t)'0');
        multiplier = multiplier * 10;
    }
    return number;
}

int is_substring(char line[], size_t start, const char* substring)
{
    size_t i;
    for (
        i = 0;
        line[start + i] && substring[i] && substring[i] == line[start + i];
        ++i
    );
    return !substring[i];
}

void parse_game(char line[], size_t start, struct MinRGB *min_rgb)
{
    char c;
    size_t number, i, red, green, blue;
    number = red = green = blue = 0;
    for (c = line[start]; (c = line[start]) && c != ';'; ++start) {
        if (c >= '0' && c <= '9') {
            number = get_number(line, start);
            while (line[start] >= '0' && line[start] <= '9')
                ++start;
            if (!line[start] || line[start] == ';')
                break;
        } else if (is_substring(line, start, RED)) {
            red = number;
            number = 0;
            for (i = 0; RED[i]; ++i)
                ++start;
            if (!line[start] || line[start] == ';')
                break;
        } else if (is_substring(line, start, GREEN)) {
            green = number;
            number = 0;
            for (i = 0; GREEN[i]; ++i)
                ++start;
            if (!line[start] || line[start] == ';')
                break;
        } else if (is_substring(line, start, BLUE)) {
            blue = number;
            number = 0;
            for (i = 0; BLUE[i]; ++i)
                ++start;
            if (!line[start] || line[start] == ';')
                break;
        }
    }
    if (red > min_rgb->min_red)
        min_rgb->min_red = red;
    if (green > min_rgb->min_green)
        min_rgb->min_green = green;
    if (blue > min_rgb->min_blue)
        min_rgb->min_blue = blue;
}

size_t get_line(char line[], size_t max_line)
{
    int c;
    size_t length;
    for (
        length = 0;
        (c = getchar()) != EOF && c != '\n' && length < max_line - 1;
        ++length
    ) line[length] = c;
    line[length] = '\0';
    return length;
}

int main(void)
{
    char line[MAX_LINE];
    struct MinRGB min_rgb;
    size_t line_length, game_id, total, current, power;

    game_id = total = power = 0;
    while ((line_length = get_line(line, MAX_LINE)) != 0) {
        ++game_id;
        min_rgb.min_red = 0;
        min_rgb.min_green = 0;
        min_rgb.min_blue = 0;
        current = get_first_game_index(line);
        while (current != line_length) {
            parse_game(line, current, &min_rgb);
            current = get_next_game_index(line, current);
        }
        power = min_rgb.min_red * min_rgb.min_green * min_rgb.min_blue;
        puts(line);
        printf(
            "%zu -> R: %zu, G: %zu, B: %zu -> %zu\n",
            game_id,
            min_rgb.min_red,
            min_rgb.min_green,
            min_rgb.min_blue,
            power
        );
        total = total + power;
    }
    printf("total = %zu\n", total);
    return 0;
}
