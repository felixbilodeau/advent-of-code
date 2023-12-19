#include <stdio.h>
#include <limits.h>

#define MAX_SIZE 65536
#define MAX_MAPS 50
#define NUM_SEED_RANGES 10

struct Range {
    long min;
    long max;
};

struct Map {
    struct Range source;
    struct Range destination;
};

long min(long number, long other)
{
    if (number <= other)
        return number;
    return other;
}

long max(long number, long other)
{
    if (number >= other)
        return number;
    return other;
}

int Range_is_overlapping(struct Range range, struct Range other)
{
    return (
        (range.min <= other.max && range.max >= other.min)
        || (other.min <= range.max && other.max >= range.min)
    );
}

void Range_bubble_sort(struct Range ranges[], size_t ranges_count)
{
    size_t swaps, i;
    struct Range temp;

    do {
        swaps = 0;
        for (i = 0; i < ranges_count - 1; ++i) {
            if (ranges[i].min > ranges[i + 1].min) {
                temp = ranges[i + 1];
                ranges[i + 1] = ranges[i];
                ranges[i] = temp;
                ++swaps;
            }
        }
    } while (swaps);
}

void Map_bubble_sort(struct Map maps[], size_t maps_count)
{
    size_t swaps, i;
    struct Map temp;

    do {
        swaps = 0;
        for (i = 0; i < maps_count - 1; ++i) {
            if (maps[i].destination.min > maps[i + 1].destination.min) {
                temp = maps[i + 1];
                maps[i + 1] = maps[i];
                maps[i] = temp;
                ++swaps;
            }
        }
    } while (swaps);
}

int is_digit(char c)
{
    return c >= '0' && c <= '9';
}

int load_file(char buf[], size_t max_size)
{
    size_t i;
    int c;
    for (i = 0; (c = getchar()) != EOF && i < max_size - 1; ++i)
        buf[i] = c;
    buf[i] = '\0';
    if (c != EOF) {
        puts("Could not load entire file into memory!");
        return 0;
    }
    return 1;
}

int seek_next_colon(char **p_file)
{
    while (**p_file && **p_file != ':')
        ++(*p_file);
    return **p_file != '\0';
}

long get_number(char **p_file)
{
    long number, multiplier;
    char *start, *end;

    start = *p_file;
    while (**p_file && is_digit(**p_file))
        ++(*p_file);

    end = *p_file;
    if (!is_digit(**p_file))
        --(*p_file);

    number = 0;
    multiplier = 1;
    while (*p_file != start) {
        number = number + multiplier * (long)(**p_file - '0');
        multiplier = multiplier * 10;
        --(*p_file);
    }
    number = number + multiplier * (long)(**p_file - '0');

    *p_file = end;
    return number;
}

int load_seed_ranges(
    char **p_file, struct Range seed_ranges[], size_t num_ranges
)
{
    size_t i;
    long min, max;
    seek_next_colon(p_file);
    if (!*(++(*p_file))) {
        puts("ERROR: Reached EOF while parsing");
        return 1;
    }
    
    i = 0;
    while (is_digit(*(++(*p_file))) && i < num_ranges) {
        min = get_number(p_file);
        if (!**p_file || !*(++(*p_file))) return 0;
        max = min + get_number(p_file) - 1;
        seed_ranges[i] = (struct Range){.min = min, .max = max};
        ++i;
    }
    return !is_digit(**p_file);
}

size_t load_next_map(char **p_file, struct Map map[], size_t max_size)
{
    long min, max, destination;
    size_t i;

    seek_next_colon(p_file);
    if (!*(++(*p_file))) {
        puts("ERROR: Reached EOF while parsing");
        return 0;
    }

    i = 0;
    while (*(++(*p_file)) && is_digit(**p_file) && i < max_size) {
        destination = get_number(p_file);
        if (!**p_file || !*(++(*p_file))) return 0;
        min = get_number(p_file);
        if (!**p_file || !*(++(*p_file))) return 0;
        max = min + get_number(p_file) - 1;
        map[i] = (struct Map){
            .source = (struct Range) {
                .min = min,
                .max = max,
            },
            .destination = (struct Range) {
                .min = destination,
                .max = destination + max - min
            }
        };
        ++i;
    }
    return i;
}

size_t patch_map_holes(struct Map maps[], size_t maps_count)
{
    struct Map temp;
    size_t i, j;

    if (maps_count && maps[0].destination.min != 0) {
        temp = (struct Map) {
            .source = (struct Range) {
                .min = 0,
                .max = maps[0].destination.min - 1
            },
            .destination = (struct Range) {
                .min = 0,
                .max = maps[0].destination.min - 1
            }
        };
        for (j = maps_count - 1; j > 0; --j)
            maps[j + 1] = maps[j];
        maps[1] = maps[0];
        maps[0] = temp;
        ++maps_count;
    }

    for (i = 0; i < maps_count - 1; ++i) {
        if (maps[i].destination.max != maps[i + 1].destination.min - 1) {
            temp = (struct Map) {
                .source = (struct Range) {
                    .min = maps[i].destination.max + 1,
                    .max = maps[i + 1].destination.min - 1
                },
                .destination = (struct Range) {
                    .min = maps[i].destination.max + 1,
                    .max = maps[i + 1].destination.min - 1
                }
            };
            for (j = maps_count - 1; j > i; --j) {
                maps[j + 1] = maps[j];
            }
            maps[i + 1] = temp;
            ++maps_count;
        }
    }
    return maps_count;
}

int convert_number(char **p_file, long *number)
{
    long destination, source, range;
    int converted;

    converted = 0;

    while (*(++(*p_file)) && is_digit(**p_file) && !converted) {
        destination = get_number(p_file);
        if (!**p_file || !*(++(*p_file))) return 0;
        source = get_number(p_file);
        if (!**p_file || !*(++(*p_file))) return 0;
        range = get_number(p_file);
        if (*number - source < range && *number - source >= 0) {
            *number = destination + *number - source;
            converted = 1;
        }
    }
    return 1;
}

size_t check_upstream_map(
    struct Range range,
    struct Map upstream[],
    size_t upstream_count,
    struct Range ranges[]
) {
    size_t i, ranges_len;

    for (i = ranges_len = 0; i < upstream_count; ++i) {
        if (
            Range_is_overlapping(
                range,
                upstream[i].destination
            )
        ) {
            ranges[ranges_len] = (struct Range) {
                .min = max(
                    upstream[i].source.min,
                    upstream[i].source.min + range.min - upstream[i].destination.min
                ),
                .max = min(
                    upstream[i].source.max,
                    upstream[i].source.min + range.max - upstream[i].destination.min
                )
            };
            ++ranges_len;
        }
    }

    if (
        Range_is_overlapping(
            range, upstream[upstream_count - 1].destination
        )
        && upstream[upstream_count - 1].destination.max < range.max
    ) {
        ranges[ranges_len] = (struct Range) {
            .min = max(
                upstream[upstream_count - 1].destination.max + 1,
                range.min
            ),
            .max = range.max,
        };
        ++ranges_len;
    }
    return ranges_len;
}

int main(void)
{
    char buf[MAX_SIZE];
    char *file_pointer;
    size_t i1, i2, i3, i4, i5, i6, i7, i8;
    size_t j;
    int converted[2] = {0, 0};

    long min_location, location_min_seed, location_max_seed, min_seed, max_seed;
    
    struct Range ranges[MAX_MAPS];
    size_t ranges_len;
    struct Range ranges1[MAX_MAPS];
    size_t ranges1_len;
    struct Range ranges2[MAX_MAPS];
    size_t ranges2_len;
    struct Range ranges3[MAX_MAPS];
    size_t ranges3_len;
    struct Range ranges4[MAX_MAPS];
    size_t ranges4_len;
    struct Range ranges5[MAX_MAPS];
    size_t ranges5_len;
    struct Range ranges6[MAX_MAPS];
    size_t ranges6_len;

    struct Range seed_ranges[NUM_SEED_RANGES];

    struct Map seed_to_soil[MAX_MAPS];
    size_t seed_to_soil_len;

    struct Map soil_to_fertilizer[MAX_MAPS];
    size_t soil_to_fertilizer_len;

    struct Map fertilizer_to_water[MAX_MAPS];
    size_t fertilizer_to_water_len;

    struct Map water_to_light[MAX_MAPS];
    size_t water_to_light_len;

    struct Map light_to_temperature[MAX_MAPS];
    size_t light_to_temperature_len;

    struct Map temperature_to_humidity[MAX_MAPS];
    size_t temperature_to_humidity_len;

    struct Map humidity_to_location[MAX_MAPS];
    size_t humidity_to_location_len;
    
    if (!load_file(buf, MAX_SIZE)) return 1;
    file_pointer = buf;

    if (!load_seed_ranges(&file_pointer, seed_ranges, NUM_SEED_RANGES))
        return 1;
    if (
        !(
            seed_to_soil_len = load_next_map(
                &file_pointer, seed_to_soil, MAX_MAPS
            )
        )
    ) return 1;
    if (
        !(
            soil_to_fertilizer_len = load_next_map(
                &file_pointer, soil_to_fertilizer, MAX_MAPS
            )
        )
    ) return 1;
    if (
        !(
            fertilizer_to_water_len = load_next_map(
                &file_pointer, fertilizer_to_water, MAX_MAPS
            )
        )
    ) return 1;
    if (
        !(
            water_to_light_len = load_next_map(
                &file_pointer, water_to_light, MAX_MAPS
            )
        )
    ) return 1;
    if (
        !(
            light_to_temperature_len = load_next_map(
                &file_pointer, light_to_temperature, MAX_MAPS
            )
        )
    ) return 1;
    if (
        !(
            temperature_to_humidity_len = load_next_map(
                &file_pointer, temperature_to_humidity, MAX_MAPS
            )
        )
    ) return 1;
    if (
        !(
            humidity_to_location_len = load_next_map(
                &file_pointer, humidity_to_location, MAX_MAPS
            )
        )
    ) return 1;

    Range_bubble_sort(seed_ranges, NUM_SEED_RANGES);
    Map_bubble_sort(seed_to_soil, seed_to_soil_len);
    Map_bubble_sort(soil_to_fertilizer, soil_to_fertilizer_len);
    Map_bubble_sort(fertilizer_to_water, fertilizer_to_water_len);
    Map_bubble_sort(water_to_light, water_to_light_len);
    Map_bubble_sort(light_to_temperature, light_to_temperature_len);
    Map_bubble_sort(temperature_to_humidity, temperature_to_humidity_len);
    Map_bubble_sort(humidity_to_location, humidity_to_location_len);
    seed_to_soil_len = patch_map_holes(
        seed_to_soil, seed_to_soil_len
    );
    soil_to_fertilizer_len = patch_map_holes(
        soil_to_fertilizer, soil_to_fertilizer_len
    );
    fertilizer_to_water_len = patch_map_holes(
        fertilizer_to_water, fertilizer_to_water_len
    );
    water_to_light_len = patch_map_holes(
        water_to_light, water_to_light_len
    );
    light_to_temperature_len = patch_map_holes(
        light_to_temperature, light_to_temperature_len
    );
    temperature_to_humidity_len = patch_map_holes(
        temperature_to_humidity, temperature_to_humidity_len
    );
    humidity_to_location_len = patch_map_holes(
        humidity_to_location, humidity_to_location_len
    );
    for (i1 = 0; i1 < NUM_SEED_RANGES; ++i1)
        printf(
            "Seed Range %2zu: min: %10ld, max: %10ld\n",
            i1 + 1,
            seed_ranges[i1].min,
            seed_ranges[i1].max
        );
    putchar('\n');
    for (i1 = 0; i1 < seed_to_soil_len; ++i1)
        printf(
            "Seed To Soil Map %2zu: source min: %10ld, source max: %10ld, destination min: %10ld, destination max: %10ld\n",
            i1 + 1,
            seed_to_soil[i1].source.min,
            seed_to_soil[i1].source.max,
            seed_to_soil[i1].destination.min,
            seed_to_soil[i1].destination.max
        );
    putchar('\n');
    for (i1 = 0; i1 < soil_to_fertilizer_len; ++i1)
        printf(
            "Soil To Fertilizer Map %2zu: source min: %10ld, source max: %10ld, destination min: %10ld, destination max: %10ld\n",
            i1 + 1,
            soil_to_fertilizer[i1].source.min,
            soil_to_fertilizer[i1].source.max,
            soil_to_fertilizer[i1].destination.min,
            soil_to_fertilizer[i1].destination.max
        );
    putchar('\n');
    for (i1 = 0; i1 < fertilizer_to_water_len; ++i1)
        printf(
            "Fertilizer To Water Map %2zu: source min: %10ld, source max: %10ld, destination min: %10ld, destination max: %10ld\n",
            i1 + 1,
            fertilizer_to_water[i1].source.min,
            fertilizer_to_water[i1].source.max,
            fertilizer_to_water[i1].destination.min,
            fertilizer_to_water[i1].destination.max
        );
    putchar('\n');
    for (i1 = 0; i1 < water_to_light_len; ++i1)
        printf(
            "Water To Light Map %2zu: source min: %10ld, source max: %10ld, destination min: %10ld, destination max: %10ld\n",
            i1 + 1,
            water_to_light[i1].source.min,
            water_to_light[i1].source.max,
            water_to_light[i1].destination.min,
            water_to_light[i1].destination.max
        );
    putchar('\n');
    for (i1 = 0; i1 < light_to_temperature_len; ++i1)
        printf(
            "Light To Temperature Map %2zu: source min: %10ld, source max: %10ld, destination min: %10ld, destination max: %10ld\n",
            i1 + 1,
            light_to_temperature[i1].source.min,
            light_to_temperature[i1].source.max,
            light_to_temperature[i1].destination.min,
            light_to_temperature[i1].destination.max
        );
    putchar('\n');
    for (i1 = 0; i1 < temperature_to_humidity_len; ++i1)
        printf(
            "Temperature To Humidity Map %2zu: source min: %10ld, source max: %10ld, destination min: %10ld, destination max: %10ld\n",
            i1 + 1,
            temperature_to_humidity[i1].source.min,
            temperature_to_humidity[i1].source.max,
            temperature_to_humidity[i1].destination.min,
            temperature_to_humidity[i1].destination.max
        );
    putchar('\n');
    for (i1 = 0; i1 < humidity_to_location_len; ++i1)
        printf(
            "Humidity To Location Map %2zu: source min: %10ld, source max: %10ld, destination min: %10ld, destination max: %10ld\n",
            i1 + 1,
            humidity_to_location[i1].source.min,
            humidity_to_location[i1].source.max,
            humidity_to_location[i1].destination.min,
            humidity_to_location[i1].destination.max
        );
    putchar('\n');
    min_location = 0;
    
    for (i1 = 0; i1 < humidity_to_location_len; ++i1)
        ranges[i1] = humidity_to_location[i1].source;
    ranges_len = humidity_to_location_len;

    for (i1 = 0; i1 < ranges_len; i1++) {
        ranges1_len = check_upstream_map(
            ranges[i1],
            temperature_to_humidity,
            temperature_to_humidity_len,
            ranges1
        );
        for (i2 = 0; i2 < ranges1_len; i2++) {
            ranges2_len = check_upstream_map(
                ranges1[i2],
                light_to_temperature,
                light_to_temperature_len,
                ranges2
            );
            for (i3 = 0; i3 < ranges2_len; i3++) {
                ranges3_len = check_upstream_map(
                    ranges2[i3],
                    water_to_light,
                    water_to_light_len,
                    ranges3
                );
                for (i4 = 0; i4 < ranges3_len; i4++) {
                    ranges4_len = check_upstream_map(
                        ranges3[i4],
                        fertilizer_to_water,
                        fertilizer_to_water_len,
                        ranges4
                    );
                    for (i5 = 0; i5 < ranges4_len; i5++) {
                        ranges5_len = check_upstream_map(
                            ranges4[i5],
                            soil_to_fertilizer,
                            soil_to_fertilizer_len,
                            ranges5
                        );
                        for (i6 = 0; i6 < ranges5_len; i6++) {
                            ranges6_len = check_upstream_map(
                                ranges5[i6],
                                seed_to_soil,
                                seed_to_soil_len,
                                ranges6
                            );
                            for (i7 = 0; i7 < ranges6_len; ++i7) {
                                for (i8 = 0; i8 < NUM_SEED_RANGES; ++i8) {
                                    if (Range_is_overlapping(ranges6[i7], seed_ranges[i8])) {
                                        min_seed = max(ranges6[i7].min, seed_ranges[i8].min);
                                        max_seed = min(ranges6[i7].max, seed_ranges[i8].max);
                                        location_min_seed = min_seed;
                                        location_max_seed = max_seed;
                                        for (j = 0; j < seed_to_soil_len; ++j) {
                                            converted[0] = converted[1] = 0;
                                            if (
                                                !converted[0]
                                                && location_min_seed >= seed_to_soil[j].source.min
                                                && location_min_seed <= seed_to_soil[j].source.max
                                            ) {
                                                location_min_seed = (
                                                    seed_to_soil[j].destination.min
                                                    + location_min_seed
                                                    - seed_to_soil[j].source.min
                                                );
                                                converted[0] = 1;
                                            }
                                            if (
                                                !converted[1]
                                                && location_max_seed >= seed_to_soil[j].source.min
                                                && location_max_seed <= seed_to_soil[j].source.max
                                            ) {
                                                location_max_seed = (
                                                    seed_to_soil[j].destination.min
                                                    + location_max_seed
                                                    - seed_to_soil[j].source.min
                                                );
                                                converted[1] = 1;
                                            }
                                            if (converted[0] && converted[1])
                                                break;
                                        }
                                        for (j = 0; j < soil_to_fertilizer_len; ++j) {
                                            converted[0] = converted[1] = 0;
                                            if (
                                                !converted[0]
                                                && location_min_seed >= soil_to_fertilizer[j].source.min
                                                && location_min_seed <= soil_to_fertilizer[j].source.max
                                            ) {
                                                location_min_seed = (
                                                    soil_to_fertilizer[j].destination.min
                                                    + location_min_seed
                                                    - soil_to_fertilizer[j].source.min
                                                );
                                                converted[0] = 1;
                                            }
                                            if (
                                                !converted[1]
                                                && location_max_seed >= soil_to_fertilizer[j].source.min
                                                && location_max_seed <= soil_to_fertilizer[j].source.max
                                            ) {
                                                location_max_seed = (
                                                    soil_to_fertilizer[j].destination.min
                                                    + location_max_seed
                                                    - soil_to_fertilizer[j].source.min
                                                );
                                                converted[1] = 1;
                                            }
                                            if (converted[0] && converted[1])
                                                break;
                                        }
                                        for (j = 0; j < fertilizer_to_water_len; ++j) {
                                            converted[0] = converted[1] = 0;
                                            if (
                                                !converted[0]
                                                && location_min_seed >= fertilizer_to_water[j].source.min
                                                && location_min_seed <= fertilizer_to_water[j].source.max
                                            ) {
                                                location_min_seed = (
                                                    fertilizer_to_water[j].destination.min
                                                    + location_min_seed
                                                    - fertilizer_to_water[j].source.min
                                                );
                                                converted[0] = 1;
                                            }
                                            if (
                                                !converted[1]
                                                && location_max_seed >= fertilizer_to_water[j].source.min
                                                && location_max_seed <= fertilizer_to_water[j].source.max
                                            ) {
                                                location_max_seed = (
                                                    fertilizer_to_water[j].destination.min
                                                    + location_max_seed
                                                    - fertilizer_to_water[j].source.min
                                                );
                                                converted[1] = 1;
                                            }
                                            if (converted[0] && converted[1])
                                                break;
                                        }
                                        for (j = 0; j < water_to_light_len; ++j) {
                                            converted[0] = converted[1] = 0;
                                            if (
                                                !converted[0]
                                                && location_min_seed >= water_to_light[j].source.min
                                                && location_min_seed <= water_to_light[j].source.max
                                            ) {
                                                location_min_seed = (
                                                    water_to_light[j].destination.min
                                                    + location_min_seed
                                                    - water_to_light[j].source.min
                                                );
                                                converted[0] = 1;
                                            }
                                            if (
                                                !converted[1]
                                                && location_max_seed >= water_to_light[j].source.min
                                                && location_max_seed <= water_to_light[j].source.max
                                            ) {
                                                location_max_seed = (
                                                    water_to_light[j].destination.min
                                                    + location_max_seed
                                                    - water_to_light[j].source.min
                                                );
                                                converted[1] = 1;
                                            }
                                            if (converted[0] && converted[1])
                                                break;
                                        }
                                        for (j = 0; j < light_to_temperature_len; ++j) {
                                            converted[0] = converted[1] = 0;
                                            if (
                                                !converted[0]
                                                && location_min_seed >= light_to_temperature[j].source.min
                                                && location_min_seed <= light_to_temperature[j].source.max
                                            ) {
                                                location_min_seed = (
                                                    light_to_temperature[j].destination.min
                                                    + location_min_seed
                                                    - light_to_temperature[j].source.min
                                                );
                                                converted[0] = 1;
                                            }
                                            if (
                                                !converted[1]
                                                && location_max_seed >= light_to_temperature[j].source.min
                                                && location_max_seed <= light_to_temperature[j].source.max
                                            ) {
                                                location_max_seed = (
                                                    light_to_temperature[j].destination.min
                                                    + location_max_seed
                                                    - light_to_temperature[j].source.min
                                                );
                                                converted[1] = 1;
                                            }
                                            if (converted[0] && converted[1])
                                                break;
                                        }
                                        for (j = 0; j < temperature_to_humidity_len; ++j) {
                                            converted[0] = converted[1] = 0;
                                            if (
                                                !converted[0]
                                                && location_min_seed >= temperature_to_humidity[j].source.min
                                                && location_min_seed <= temperature_to_humidity[j].source.max
                                            ) {
                                                location_min_seed = (
                                                    temperature_to_humidity[j].destination.min
                                                    + location_min_seed
                                                    - temperature_to_humidity[j].source.min
                                                );
                                                converted[0] = 1;
                                            }
                                            if (
                                                !converted[1]
                                                && location_max_seed >= temperature_to_humidity[j].source.min
                                                && location_max_seed <= temperature_to_humidity[j].source.max
                                            ) {
                                                location_max_seed = (
                                                    temperature_to_humidity[j].destination.min
                                                    + location_max_seed
                                                    - temperature_to_humidity[j].source.min
                                                );
                                                converted[1] = 1;
                                            }
                                            if (converted[0] && converted[1])
                                                break;
                                        }
                                        for (j = 0; j < humidity_to_location_len; ++j) {
                                            converted[0] = converted[1] = 0;
                                            if (
                                                !converted[0]
                                                && location_min_seed >= humidity_to_location[j].source.min
                                                && location_min_seed <= humidity_to_location[j].source.max
                                            ) {
                                                location_min_seed = (
                                                    humidity_to_location[j].destination.min
                                                    + location_min_seed
                                                    - humidity_to_location[j].source.min
                                                );
                                                converted[0] = 1;
                                            }
                                            if (
                                                !converted[1]
                                                && location_max_seed >= humidity_to_location[j].source.min
                                                && location_max_seed <= humidity_to_location[j].source.max
                                            ) {
                                                location_max_seed = (
                                                    humidity_to_location[j].destination.min
                                                    + location_max_seed
                                                    - humidity_to_location[j].source.min
                                                );
                                                converted[1] = 1;
                                            }
                                            if (converted[0] && converted[1])
                                                break;
                                        }
                                        if (min_location == 0) {
                                            min_location = min(location_min_seed, location_max_seed);
                                        } else {
                                            if (min(location_min_seed, location_max_seed) < min_location)
                                                min_location = min(location_min_seed, location_max_seed);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    printf("min location = %ld\n", min_location);
    return 0;
}
