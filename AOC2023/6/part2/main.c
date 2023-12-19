#include <stdio.h>

struct Race {
    size_t total_time;
    size_t record;
};

int is_win(struct Race race, size_t hold_time)
{
    size_t move_time;

    if (hold_time > race.total_time) return 0;

    move_time = race.total_time - hold_time;
    return hold_time * move_time > race.record;
}

int main(void)
{
    struct Race race;
    size_t i, total;
    
    race = (struct Race) {
        .total_time = 62649190,
        .record = 553101014731074
    };

    total = 0;
    for (i = 0; i <= race.total_time; ++i)
        if (is_win(race, i))
            ++total;

    printf("total = %zu\n", total);
    return 0;
}
