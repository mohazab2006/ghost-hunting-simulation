#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "defs.h"
#include "helpers.h"

static void hunter_take_turn(struct Hunter* hunter);

void hunter_init(struct Hunter* hunter, const char* name, int id, struct House* house) {
    if (!hunter || !house) {
        return;
    }

    memset(hunter, 0, sizeof(*hunter));

    if (name) {
        strncpy(hunter->name, name, MAX_HUNTER_NAME - 1);
        hunter->name[MAX_HUNTER_NAME - 1] = '\0';
    } else {
        hunter->name[0] = '\0';
    }

    hunter->id = id;
    hunter->house = house;
    hunter->case_file = &house->case_file;

    hunter->current_room = house->starting_room;

    hunter->boredom = 0;
    hunter->fear = 0;
    hunter->returning = false;
    hunter->exited = false;
    hunter->exit_reason = LR_BORED;

    roomstack_init(&hunter->path);

    const enum EvidenceType* devices = NULL;
    int count = get_all_evidence_types(&devices);
    if (count <= 0) {
        hunter->device = EV_EMF;
    } else {
        int idx = rand_int_threadsafe(0, count);
        hunter->device = devices[idx];
    }

    struct Room* room = hunter->current_room;
    if (room) {
        lock_two_rooms(room, NULL);
        room_add_hunter(room, hunter);
        unlock_two_rooms(room, NULL);
    }

    log_hunter_init(hunter->id, room ? room->name : "", hunter->name, hunter->device);
}

void hunter_cleanup(struct Hunter* hunter) {
    if (!hunter) {
        return;
    }
    roomstack_clear(&hunter->path);
}

static void hunter_update_stats(struct Hunter* hunter) {
    struct Room* room = hunter->current_room;
    if (!room) {
        return;
    }

    lock_two_rooms(room, NULL);
    bool ghost_here = (room->ghost != NULL);
    unlock_two_rooms(room, NULL);

    if (ghost_here) {
        hunter->boredom = 0;
        hunter->fear++;
    } else {
        hunter->boredom++;
    }
}

static bool hunter_check_exit_conditions(struct Hunter* hunter) {
    struct Room* room = hunter->current_room;
    if (!room) {
        hunter->exited = true;
        return true;
    }

    enum LogReason reason;
    if (hunter->boredom > ENTITY_BOREDOM_MAX) {
        reason = LR_BORED;
    } else if (hunter->fear > HUNTER_FEAR_MAX) {
        reason = LR_AFRAID;
    } else {
        return false;
    }

    lock_two_rooms(room, NULL);
    room_remove_hunter(room, hunter);
    unlock_two_rooms(room, NULL);

    hunter->exited = true;
    hunter->exit_reason = reason;

    log_exit(hunter->id, hunter->boredom, hunter->fear, room->name, hunter->device, reason);
    return true;
}

static bool hunter_handle_exit_room(struct Hunter* hunter) {
    struct Room* room = hunter->current_room;
    if (!room || !room->is_exit) {
        return false;
    }

    roomstack_clear(&hunter->path);

    if (hunter->returning) {
        log_return_to_van(hunter->id, hunter->boredom, hunter->fear, room->name, hunter->device, false);
        hunter->returning = false;
    }

    bool solved = false;
    sem_wait(&hunter->case_file->mutex);
    solved = hunter->case_file->solved;
    sem_post(&hunter->case_file->mutex);

    if (solved) {
        lock_two_rooms(room, NULL);
        room_remove_hunter(room, hunter);
        unlock_two_rooms(room, NULL);

        hunter->exited = true;
        hunter->exit_reason = LR_EVIDENCE;

        log_exit(hunter->id, hunter->boredom, hunter->fear, room->name, hunter->device, LR_EVIDENCE);
        return true;
    }

    const enum EvidenceType* devices = NULL;
    int count = get_all_evidence_types(&devices);
    if (count > 0) {
        enum EvidenceType old = hunter->device;
        enum EvidenceType new_dev = old;

        if (count == 1) {
            new_dev = devices[0];
        } else {
            int attempts = 0;
            while (attempts < 10 && new_dev == old) {
                int idx = rand_int_threadsafe(0, count);
                new_dev = devices[idx];
                attempts++;
            }
        }

        if (new_dev != old) {
            hunter->device = new_dev;
            log_swap(hunter->id, hunter->boredom, hunter->fear, old, new_dev);
        }
    }

    return false;
}

static void hunter_attempt_gather(struct Hunter* hunter) {
    struct Room* room = hunter->current_room;
    if (!room) {
        return;
    }

    EvidenceByte device_mask = (EvidenceByte)hunter->device;

    lock_two_rooms(room, NULL);
    EvidenceByte available = room->evidence;
    unlock_two_rooms(room, NULL);

    if (available & device_mask) {
        lock_two_rooms(room, NULL);
        room->evidence = (EvidenceByte)(room->evidence & (EvidenceByte)(~device_mask));
        unlock_two_rooms(room, NULL);

        casefile_add_evidence(hunter->case_file, device_mask);

        log_evidence(hunter->id, hunter->boredom, hunter->fear, room->name, hunter->device);

        if (!room->is_exit && !hunter->returning) {
            hunter->returning = true;
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear, room->name, hunter->device, true);
        }
    } else {
        if (!room->is_exit && !hunter->returning) {
            int roll = rand_int_threadsafe(0, 10);
            if (roll == 0) {
                hunter->returning = true;
                log_return_to_van(hunter->id, hunter->boredom, hunter->fear, room->name, hunter->device, true);
            }
        }
    }
}

static void hunter_move(struct Hunter* hunter) {
    struct Room* from = hunter->current_room;
    if (!from) {
        return;
    }

    struct Room* target = NULL;

    if (hunter->returning) {
        if (hunter->path.top) {
            target = hunter->path.top->room;
        } else if (hunter->house && hunter->house->starting_room && from != hunter->house->starting_room) {
            target = hunter->house->starting_room;
        } else {
            return;
        }
    } else {
        lock_two_rooms(from, NULL);
        int conn_count = from->connection_count;
        if (conn_count > 0) {
            int idx = rand_int_threadsafe(0, conn_count);
            target = from->connections[idx];
        }
        unlock_two_rooms(from, NULL);
    }

    if (!target || target == from) {
        return;
    }

    lock_two_rooms(from, target);

    if (target->hunter_count >= MAX_ROOM_OCCUPANCY) {
        unlock_two_rooms(from, target);
        return;
    }

    room_remove_hunter(from, hunter);
    room_add_hunter(target, hunter);
    hunter->current_room = target;

    if (hunter->returning) {
        if (hunter->path.top && hunter->path.top->room == target) {
            roomstack_pop(&hunter->path);
        }
    } else {
        roomstack_push(&hunter->path, from);
    }

    const char* from_name = from->name;
    const char* to_name = target->name;

    unlock_two_rooms(from, target);

    log_move(hunter->id, hunter->boredom, hunter->fear, from_name, to_name, hunter->device);
}

static void hunter_take_turn(struct Hunter* hunter) {
    if (!hunter || hunter->exited) {
        return;
    }

    hunter_update_stats(hunter);

    if (hunter_handle_exit_room(hunter)) {
        return;
    }

    if (hunter_check_exit_conditions(hunter)) {
        return;
    }

    hunter_attempt_gather(hunter);

    hunter_move(hunter);
}

void* hunter_thread_func(void* arg) {
    struct Hunter* hunter = (struct Hunter*)arg;
    if (!hunter) {
        return NULL;
    }

    while (!hunter->exited) {
        hunter_take_turn(hunter);

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 10 * 1000 * 1000; // 10 ms
        nanosleep(&ts, NULL);
    }

    return NULL;
}
