#include "defs.h"

void lock_two_rooms(struct Room* a, struct Room* b) {
    if (!a && !b) {
        return;
    }
    if (a == b) {
        if (a) {
            sem_wait(&a->mutex);
        }
        return;
    }

    struct Room* first = a;
    struct Room* second = b;

    if (!first) {
        first = b;
        second = NULL;
    } else if (!second) {
        second = NULL;
    } else if (first->index > second->index) {
        struct Room* tmp = first;
        first = second;
        second = tmp;
    }

    if (first) {
        sem_wait(&first->mutex);
    }
    if (second) {
        sem_wait(&second->mutex);
    }
}

void unlock_two_rooms(struct Room* a, struct Room* b) {
    if (!a && !b) {
        return;
    }
    if (a == b) {
        if (a) {
            sem_post(&a->mutex);
        }
        return;
    }

    struct Room* first = a;
    struct Room* second = b;

    if (!first) {
        first = b;
        second = NULL;
    } else if (!second) {
        second = NULL;
    } else if (first->index > second->index) {
        struct Room* tmp = first;
        first = second;
        second = tmp;
    }

    if (second) {
        sem_post(&second->mutex);
    }
    if (first) {
        sem_post(&first->mutex);
    }
}
