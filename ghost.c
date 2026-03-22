#include <stdlib.h>
#include <time.h>
#include "defs.h"
#include "helpers.h"

static void ghost_take_turn(struct Ghost* ghost);

void ghost_init(struct Ghost* ghost, struct House* house) {
    if (!ghost || !house) {
        return;
    }

    ghost->id = DEFAULT_GHOST_ID;
    ghost->house = house;
    ghost->boredom = 0;
    ghost->exited = false;
    ghost->current_room = NULL;

    const enum GhostType* ghost_types = NULL;
    int ghost_count = get_all_ghost_types(&ghost_types);
    if (ghost_count <= 0) {
        ghost->type = GH_SPIRIT;
    } else {
        int index = rand_int_threadsafe(0, ghost_count);
        ghost->type = ghost_types[index];
    }

    int room_index = 0;
    if (house->room_count > 0) {
        room_index = rand_int_threadsafe(0, house->room_count);
    }

    struct Room* start = &house->rooms[room_index];
    ghost->current_room = start;

    lock_two_rooms(start, NULL);
    start->ghost = ghost;
    unlock_two_rooms(start, NULL);

    log_ghost_init(ghost->id, start->name, ghost->type);
}

static void ghost_take_turn(struct Ghost* ghost) {
    if (!ghost || ghost->exited) {
        return;
    }

    struct Room* room = ghost->current_room;
    if (!room) {
        ghost->exited = true;
        return;
    }

    lock_two_rooms(room, NULL);
    bool hunters_here = room_has_hunters(room);
    unlock_two_rooms(room, NULL);

    if (hunters_here) {
        ghost->boredom = 0;
    } else {
        ghost->boredom++;
    }

    if (ghost->boredom > ENTITY_BOREDOM_MAX) {
        lock_two_rooms(room, NULL);
        room->ghost = NULL;
        unlock_two_rooms(room, NULL);

        log_ghost_exit(ghost->id, ghost->boredom, room->name);
        ghost->exited = true;
        return;
    }

    int action = rand_int_threadsafe(0, 3); // 0 = idle, 1 = haunt, 2 = move

    if (action == 0) {
        log_ghost_idle(ghost->id, ghost->boredom, room->name);
        return;
    }

    if (action == 1) {
        enum EvidenceType evs[7];
        int ev_count = 0;
        const enum EvidenceType* all = NULL;
        int all_count = get_all_evidence_types(&all);

        for (int i = 0; i < all_count; i++) {
            if (ghost->type & all[i]) {
                evs[ev_count++] = all[i];
            }
        }

        if (ev_count == 0) {
            log_ghost_idle(ghost->id, ghost->boredom, room->name);
            return;
        }

        enum EvidenceType chosen = evs[rand_int_threadsafe(0, ev_count)];

        lock_two_rooms(room, NULL);
        room->evidence |= chosen;
        unlock_two_rooms(room, NULL);

        log_ghost_evidence(ghost->id, ghost->boredom, room->name, chosen);
        return;
    }

    if (hunters_here) {
        log_ghost_idle(ghost->id, ghost->boredom, room->name);
        return;
    }

    lock_two_rooms(room, NULL);
    int connection_count = room->connection_count;
    struct Room* target = NULL;
    if (connection_count > 0) {
        int idx = rand_int_threadsafe(0, connection_count);
        target = room->connections[idx];
    }
    unlock_two_rooms(room, NULL);

    if (!target) {
        log_ghost_idle(ghost->id, ghost->boredom, room->name);
        return;
    }

    struct Room* from_room = room;
    lock_two_rooms(from_room, target);

    if (from_room->ghost == ghost) {
        from_room->ghost = NULL;
    }
    target->ghost = ghost;
    ghost->current_room = target;

    const char* from_name = from_room->name;
    const char* to_name = target->name;

    unlock_two_rooms(from_room, target);

    log_ghost_move(ghost->id, ghost->boredom, from_name, to_name);
}

void* ghost_thread_func(void* arg) {
    struct Ghost* ghost = (struct Ghost*)arg;
    if (!ghost) {
        return NULL;
    }

    while (!ghost->exited) {
        ghost_take_turn(ghost);

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 10 * 1000 * 1000; // 10 ms
        nanosleep(&ts, NULL);
    }

    return NULL;
}
