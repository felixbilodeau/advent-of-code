#include <stdio.h>

#define NUM_RACES 4

struct Race {
    int total_time;
    int record;
};

int is_win(struct Race race, int hold_time)
{
    int move_time;

    if (hold_time > race.total_time) return 0;

    move_time = race.total_time - hold_time;
    return hold_time * move_time > race.record;
}

int main(void)
{
    struct Race races[NUM_RACES];
    size_t i, j, win_count, total;
    
    races[0] = (struct Race) {
        .total_time = 62,
        .record = 553
    };
    races[1] = (struct Race) {
        .total_time = 64,
        .record = 1010
    };
    races[2] = (struct Race) {
        .total_time = 91,
        .record = 1473
    };
    races[3] = (struct Race) {
        .total_time = 90,
        .record = 1074
    };

    total = 1;
    for (i = 0; i < NUM_RACES; ++i) {
        win_count = 0;
        for (j = 0; j <= (size_t)races[i].total_time; ++j)
            if (is_win(races[i], j))
                ++win_count;
        total = total * win_count;
    }

    printf("total = %zu\n", total);
    return 0;
}
