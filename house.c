#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "defs.h"
#include "helpers.h"

void house_init(struct House* house) {
    if (!house) {
        return;
    }

    memset(house, 0, sizeof(*house));

    house->hunters = NULL;
    house->hunter_count = 0;
    house->hunter_capacity = 0;

    casefile_init(&house->case_file);

    // Populate rooms and set starting_room/room_count
    house_populate_rooms(house);
}

void house_cleanup(struct House* house) {
    if (!house) {
        return;
    }

    for (int i = 0; i < house->hunter_count; i++) {
        hunter_cleanup(&house->hunters[i]);
    }

    free(house->hunters);
    house->hunters = NULL;
    house->hunter_capacity = 0;
    house->hunter_count = 0;

    for (int i = 0; i < house->room_count; i++) {
        sem_destroy(&house->rooms[i].mutex);
    }

    casefile_destroy(&house->case_file);
}

struct Hunter* house_add_hunter(struct House* house, const char* name, int id) {
    if (!house) {
        return NULL;
    }

    if (house->hunter_count >= house->hunter_capacity) {
        int new_cap = (house->hunter_capacity == 0) ? 1 : house->hunter_capacity * 2;
        struct Hunter* resized = (struct Hunter*)realloc(house->hunters, sizeof(struct Hunter) * new_cap);
        if (!resized) {
            fprintf(stderr, "Failed to grow hunter array.\n");
            return NULL;
        }
        house->hunters = resized;
        house->hunter_capacity = new_cap;
    }

    struct Hunter* hunter = &house->hunters[house->hunter_count++];
    memset(hunter, 0, sizeof(*hunter));

    hunter_init(hunter, name, id, house);

    return hunter;
}
