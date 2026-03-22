#ifndef HELPERS_H
#define HELPERS_H

#include "defs.h"

const char* evidence_to_string(enum EvidenceType evidence);
const char* ghost_to_string(enum GhostType ghost);
const char* exit_reason_to_string(enum LogReason reason);

int get_all_evidence_types(const enum EvidenceType** list);
int get_all_ghost_types(const enum GhostType** list);

int rand_int_threadsafe(int lower_inclusive, int upper_exclusive);

bool evidence_is_valid_ghost(EvidenceByte mask);
bool evidence_has_three_unique(EvidenceByte mask);

void house_populate_rooms(struct House* house);

void log_move(int id, int boredom, int fear, const char* from, const char* to, enum EvidenceType device);
void log_evidence(int id, int boredom, int fear, const char* room, enum EvidenceType device);
void log_swap(int id, int boredom, int fear, enum EvidenceType from, enum EvidenceType to);
void log_exit(int id, int boredom, int fear, const char* room, enum EvidenceType device, enum LogReason reason);

void log_ghost_move(int id, int boredom, const char* from, const char* to);
void log_ghost_evidence(int id, int boredom, const char* room, enum EvidenceType evidence);
void log_ghost_exit(int id, int boredom, const char* room);
void log_ghost_idle(int id, int boredom, const char* room);

void log_return_to_van(int id, int boredom, int fear, const char* room, enum EvidenceType device, bool heading_home);

void log_hunter_init(int id, const char* room, const char* name, enum EvidenceType device);
void log_ghost_init(int id, const char* room, enum GhostType type);

#endif // HELPERS_H
