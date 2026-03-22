#include "defs.h"
#include "helpers.h"

void casefile_init(struct CaseFile* file) {
    if (!file) {
        return;
    }
    file->collected = 0;
    file->solved = false;
    sem_init(&file->mutex, 0, 1);
}

void casefile_destroy(struct CaseFile* file) {
    if (!file) {
        return;
    }
    sem_destroy(&file->mutex);
}

void casefile_add_evidence(struct CaseFile* file, EvidenceByte evidence) {
    if (!file || evidence == 0) {
        return;
    }

    sem_wait(&file->mutex);
    file->collected |= evidence;
    file->solved = evidence_has_three_unique(file->collected);
    sem_post(&file->mutex);
}
