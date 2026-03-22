#include <stdlib.h>
#include "defs.h"

void roomstack_init(struct RoomStack* stack) {
    if (!stack) {
        return;
    }
    stack->top = NULL;
}

void roomstack_push(struct RoomStack* stack, struct Room* room) {
    if (!stack || !room) {
        return;
    }

    struct RoomNode* node = (struct RoomNode*)malloc(sizeof(struct RoomNode));
    if (!node) {
        return;
    }

    node->room = room;
    node->next = stack->top;
    stack->top = node;
}

struct Room* roomstack_pop(struct RoomStack* stack) {
    if (!stack || !stack->top) {
        return NULL;
    }

    struct RoomNode* node = stack->top;
    struct Room* room = node->room;
    stack->top = node->next;
    free(node);
    return room;
}

void roomstack_clear(struct RoomStack* stack) {
    if (!stack) {
        return;
    }

    while (stack->top) {
        struct RoomNode* node = stack->top;
        stack->top = node->next;
        free(node);
    }
}
