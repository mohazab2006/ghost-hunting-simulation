#include <string.h>
#include <stdio.h>
#include "defs.h"

void room_init(struct Room* room, const char* name, bool is_exit) {
    if (!room) {
        return;
    }

    memset(room, 0, sizeof(*room));

    if (name) {
        strncpy(room->name, name, MAX_ROOM_NAME - 1);
        room->name[MAX_ROOM_NAME - 1] = '\0';
    } else {
        room->name[0] = '\0';
    }

    room->is_exit = is_exit;
    room->connection_count = 0;
    room->hunter_count = 0;
    room->ghost = NULL;
    room->evidence = 0;
    room->index = 0;

    sem_init(&room->mutex, 0, 1);
}

void room_connect(struct Room* a, struct Room* b) {
    if (!a || !b || a == b) {
        return;
    }

    if (a->connection_count < MAX_CONNECTIONS) {
        a->connections[a->connection_count++] = b;
    }

    if (b->connection_count < MAX_CONNECTIONS) {
        b->connections[b->connection_count++] = a;
    }
}

bool room_add_hunter(struct Room* room, struct Hunter* hunter) {
    if (!room || !hunter) {
        return false;
    }

    if (room->hunter_count >= MAX_ROOM_OCCUPANCY) {
        return false;
    }

    for (int i = 0; i < room->hunter_count; i++) {
        if (room->hunters[i] == hunter) {
            return true;
        }
    }

    room->hunters[room->hunter_count++] = hunter;
    return true;
}

void room_remove_hunter(struct Room* room, struct Hunter* hunter) {
    if (!room || !hunter) {
        return;
    }

    for (int i = 0; i < room->hunter_count; i++) {
        if (room->hunters[i] == hunter) {
            for (int j = i + 1; j < room->hunter_count; j++) {
                room->hunters[j - 1] = room->hunters[j];
            }
            room->hunter_count--;
            break;
        }
    }
}

bool room_has_hunters(const struct Room* room) {
    if (!room) {
        return false;
    }

    return room->hunter_count > 0;
}
