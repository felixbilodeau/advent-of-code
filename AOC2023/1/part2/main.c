#include <stdio.h>

#define MAX_LINE 200

#define ONE "one"
#define ONE_LENGTH 3
#define ONE_CHAR '1'

#define TWO "two"
#define TWO_LENGTH 3
#define TWO_CHAR '2'

#define THREE "three"
#define THREE_LENGTH 5
#define THREE_CHAR '3'

#define FOUR "four"
#define FOUR_LENGTH 4
#define FOUR_CHAR '4'

#define FIVE "five"
#define FIVE_LENGTH 4
#define FIVE_CHAR '5'

#define SIX "six"
#define SIX_LENGTH 3
#define SIX_CHAR '6'

#define SEVEN "seven"
#define SEVEN_LENGTH 5
#define SEVEN_CHAR '7'

#define EIGHT "eight"
#define EIGHT_LENGTH 5
#define EIGHT_CHAR '8'

#define NINE "nine"
#define NINE_LENGTH 4
#define NINE_CHAR '9'


size_t get_line(char line[], size_t max_line)
{
    int c;
    size_t i;

    for (
        i = 0;
        (c = getchar()) != EOF && c != '\n' && i < max_line - 1;
        ++i
    ) {
        line[i] = (char)c;
    }
    line[i] = '\0';
    return i;
}

int is_substring(
    char line[],
    size_t index,
    size_t line_length,
    const char *substring,
    size_t substring_length
)
{
    size_t i;

    for (
        i = 0;
        i + index < line_length
        && i < substring_length
        && line[i + index] == substring[i];
        ++i
    );

    return i == substring_length;
}

int main()
{
    char line[MAX_LINE];
    size_t line_length, i, result;
    char first_digit, last_digit;
    
    result = 0;
    first_digit = last_digit = '\0';
    while ((line_length = get_line(line, MAX_LINE)) != 0) {
        for (i = 0; i < line_length; ++i) {
            if (line[i] >= '0' && line[i] <= '9') {
                if (!first_digit)
                    first_digit = line[i];
                last_digit = line[i];
            } else if (is_substring(line, i, line_length, ONE, ONE_LENGTH)){
                if (!first_digit)
                    first_digit = ONE_CHAR;
                last_digit = ONE_CHAR;
            } else if (is_substring(line, i, line_length, TWO, TWO_LENGTH)){
                if (!first_digit)
                    first_digit = TWO_CHAR;
                last_digit = TWO_CHAR;
            } else if (is_substring(line, i, line_length, THREE, THREE_LENGTH)){
                if (!first_digit)
                    first_digit = THREE_CHAR;
                last_digit = THREE_CHAR;
            } else if (is_substring(line, i, line_length, FOUR, FOUR_LENGTH)){
                if (!first_digit)
                    first_digit = FOUR_CHAR;
                last_digit = FOUR_CHAR;
            } else if (is_substring(line, i, line_length, FIVE, FIVE_LENGTH)){
                if (!first_digit)
                    first_digit = FIVE_CHAR;
                last_digit = FIVE_CHAR;
            } else if (is_substring(line, i, line_length, SIX, SIX_LENGTH)){
                if (!first_digit)
                    first_digit = SIX_CHAR;
                last_digit = SIX_CHAR;
            } else if (is_substring(line, i, line_length, SEVEN, SEVEN_LENGTH)){
                if (!first_digit)
                    first_digit = SEVEN_CHAR;
                last_digit = SEVEN_CHAR;
            } else if (is_substring(line, i, line_length, EIGHT, EIGHT_LENGTH)){
                if (!first_digit)
                    first_digit = EIGHT_CHAR;
                last_digit = EIGHT_CHAR;
            } else if (is_substring(line, i, line_length, NINE, NINE_LENGTH)){
                if (!first_digit)
                    first_digit = NINE_CHAR;
                last_digit = NINE_CHAR;
            }
        }
        result = result + (size_t)(first_digit - '0') * 10 + (size_t)(last_digit - '0');
        first_digit = last_digit = '\0';
    }
    printf("result = %zu\n", result);
    return 0;
}
