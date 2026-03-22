#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

/*
    You are free to rename all of the types and functions defined here.

    The ghost ID must remain the same for the validator to work correctly.
*/

#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057

typedef unsigned char EvidenceByte; // Just giving a helpful name to unsigned char for evidence bitmasks

enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2
};

enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
};

enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

struct CaseFile {
    EvidenceByte collected; // Union of all of the evidence bits collected between all hunters
    bool         solved;    // True when >=3 unique bits set
    sem_t        mutex;     // Used for synchronizing both fields when multithreading
};

// Forward declarations for cross-references
struct House;
struct Room;
struct Hunter;
struct Ghost;

// Stack of rooms used as breadcrumbs for hunters
struct RoomNode {
    struct Room* room;
    struct RoomNode* next;
};

struct RoomStack {
    struct RoomNode* top;
};

// Room stored in House.rooms[]
struct Room {
    char name[MAX_ROOM_NAME];

    // Graph connections
    struct Room* connections[MAX_CONNECTIONS];
    int          connection_count;

    // Occupancy
    struct Ghost*  ghost;                                  // NULL when ghost not present
    struct Hunter* hunters[MAX_ROOM_OCCUPANCY];
    int            hunter_count;

    // Room properties
    bool         is_exit;
    EvidenceByte evidence;                                 // Evidence currently in this room

    // Index into the House.rooms array, used for deterministic locking order
    int          index;

    // Synchronization for multi-threading; protects occupancy and evidence
    sem_t        mutex;
};

// Ghost stored inside the House
struct Ghost {
    int             id;
    enum GhostType  type;
    struct Room*    current_room;
    int             boredom;
    bool            exited;
    pthread_t       thread;

    struct House*   house;   // Back-pointer to the house for accessing rooms and case file
};

struct Hunter {
    char             name[MAX_HUNTER_NAME];
    int              id;

    struct Room*     current_room;
    struct CaseFile* case_file;
    struct House*    house;

    enum EvidenceType device;

    struct RoomStack path;

    int              boredom;
    int              fear;
    bool             returning;      // true when heading back to the van
    bool             exited;
    enum LogReason   exit_reason;

    pthread_t        thread;
};

// Can be either stack or heap allocated
struct House {
    struct Room  rooms[MAX_ROOMS];
    int          room_count;

    struct Room* starting_room; // the Van

    struct Hunter* hunters;
    int            hunter_count;
    int            hunter_capacity;

    struct CaseFile case_file;
    struct Ghost    ghost;
};

/* Required by house_populate_rooms */
void room_init(struct Room* room, const char* name, bool is_exit);
void room_connect(struct Room* a, struct Room* b); // Bidirectional connection

// Room stack helpers
void roomstack_init(struct RoomStack* stack);
void roomstack_push(struct RoomStack* stack, struct Room* room);
struct Room* roomstack_pop(struct RoomStack* stack);
void roomstack_clear(struct RoomStack* stack);

// Case file helpers
void casefile_init(struct CaseFile* file);
void casefile_destroy(struct CaseFile* file);
void casefile_add_evidence(struct CaseFile* file, EvidenceByte evidence);

// House / collection helpers
void house_init(struct House* house);
void house_cleanup(struct House* house);
struct Hunter* house_add_hunter(struct House* house, const char* name, int id);

// Room occupancy helpers
bool room_add_hunter(struct Room* room, struct Hunter* hunter);
void room_remove_hunter(struct Room* room, struct Hunter* hunter);
bool room_has_hunters(const struct Room* room);

// Entity initialization and threads
void ghost_init(struct Ghost* ghost, struct House* house);
void* ghost_thread_func(void* arg);

void hunter_init(struct Hunter* hunter, const char* name, int id, struct House* house);
void hunter_cleanup(struct Hunter* hunter);
void* hunter_thread_func(void* arg);

// Evidence helper
bool evidence_has_three_unique(EvidenceByte mask);

// Synchronization helpers for safely locking two rooms at once
void lock_two_rooms(struct Room* a, struct Room* b);
void unlock_two_rooms(struct Room* a, struct Room* b);

#endif // DEFS_H
