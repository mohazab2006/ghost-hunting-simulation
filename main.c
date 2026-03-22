#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "defs.h"
#include "helpers.h"

/* Helper to map collected evidence → ghost type (if valid) */
static enum GhostType guess_ghost_from_evidence(EvidenceByte mask, int *out_valid) {
    if (out_valid) {
        *out_valid = 0;
    }

    if (!evidence_is_valid_ghost(mask)) {
        return 0;
    }

    const enum GhostType *ghosts = NULL;
    int count = get_all_ghost_types(&ghosts);

    for (int i = 0; i < count; i++) {
        if ((EvidenceByte)ghosts[i] == mask) {
            if (out_valid) {
                *out_valid = 1;
            }
            return ghosts[i];
        }
    }

    return 0;
}

int main(void) {
    struct House house;
    house_init(&house);

    /* Initialise ghost first so its init line appears before the header */
    ghost_init(&house.ghost, &house);

    printf("=====================================================\n");
    printf("Willow House Investigation\n");
    printf("=====================================================\n");
    printf("Enter hunters one at a time. Type 'done' as the name to finish.\n");

    char name_buf[256];
    char line[256];

    /* Input up to 4 hunters, matching the prof’s prompts */
    while (house.hunter_count < 4) {
        printf("Enter hunter name (max 63 characters) or 'done' to finish: ");
        if (!fgets(name_buf, sizeof(name_buf), stdin)) {
            break;
        }
        name_buf[strcspn(name_buf, "\n")] = '\0';

        /* Trim leading spaces */
        char *p = name_buf;
        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p == '\0') {
            /* empty line → re-ask for name */
            continue;
        }

        /* Check for "done" (case-insensitive) */
        char lower[256];
        size_t len = strlen(p);
        for (size_t i = 0; i < len && i < sizeof(lower) - 1; i++) {
            lower[i] = (char)tolower((unsigned char)p[i]);
        }
        lower[len] = '\0';
        if (strcmp(lower, "done") == 0) {
            break;
        }

        /* Ask for ID exactly like the prof: */
        int hunter_id = 0;
        while (1) {
            printf("Enter hunter ID (integer): ");
            if (!fgets(line, sizeof(line), stdin)) {
                break;
            }

            char *endptr = NULL;
            hunter_id = (int)strtol(line, &endptr, 10);
            if (endptr == line) {
                /* no digits parsed → prompt again */
                continue;
            }
            break;
        }

        struct Hunter *h = house_add_hunter(&house, p, hunter_id);
        if (!h) {
            fprintf(stderr, "Failed to add hunter.\n");
            break;
        }

        printf("\n");
    }

    if (house.hunter_count == 0) {
        printf("\nNo hunters entered; ending investigation.\n");
        house_cleanup(&house);
        return 0;
    }

    /* Start ghost + hunter threads AFTER all hunters are created */
    if (pthread_create(&house.ghost.thread, NULL, ghost_thread_func, &house.ghost) != 0) {
        fprintf(stderr, "Failed to start ghost thread.\n");
        house_cleanup(&house);
        return 1;
    }

    for (int i = 0; i < house.hunter_count; i++) {
        if (pthread_create(&house.hunters[i].thread, NULL,
                           hunter_thread_func, &house.hunters[i]) != 0) {
            fprintf(stderr, "Failed to start hunter thread %d.\n",
                    house.hunters[i].id);
            house.hunters[i].exited = true;
        }
    }

    /* Join threads */
    pthread_join(house.ghost.thread, NULL);
    for (int i = 0; i < house.hunter_count; i++) {
        pthread_join(house.hunters[i].thread, NULL);
    }

    /* ======================= FINAL SUMMARY ======================= */

    printf("=====================================================\n");
    printf("Investigation Results:\n");
    printf("=====================================================\n");

    /* Per-hunter exit summary */
    for (int i = 0; i < house.hunter_count; i++) {
        struct Hunter *h = &house.hunters[i];
        const char *reason = exit_reason_to_string(h->exit_reason);
        printf("[x] Hunter %s (ID %d) exited because of [%s] "
               "(bored=%d fear=%d).\n",
               h->name, h->id, reason, h->boredom, h->fear);
    }
    printf("\n");

    /* Shared Case File Checklist block */
    EvidenceByte collected;
    sem_wait(&house.case_file.mutex);
    collected = house.case_file.collected;
    sem_post(&house.case_file.mutex);

    printf("Shared Case File Checklist:\n");

    const enum EvidenceType *ev_list = NULL;
    int ev_count = get_all_evidence_types(&ev_list);
    for (int i = 0; i < ev_count; i++) {
        EvidenceByte bit = (EvidenceByte)ev_list[i];
        int has_it = (collected & bit) != 0;
        const char *mark = has_it ? "[✓]" : "[ ]";
        printf(" - %s %s\n", mark, evidence_to_string(ev_list[i]));
    }
    printf("\n");

    /* Victory Results block */
    int evidence_hunters = 0;
    for (int i = 0; i < house.hunter_count; i++) {
        if (house.hunters[i].exit_reason == LR_EVIDENCE) {
            evidence_hunters++;
        }
    }

    int guess_valid = 0;
    enum GhostType guessed_type = guess_ghost_from_evidence(collected, &guess_valid);
    const char *guess_name = "N/A";
    if (guess_valid) {
        guess_name = ghost_to_string(guessed_type);
    }

    printf("Victory Results:\n");
    printf("-----------------------------------------------------\n");
    printf("- Hunters exited after identifying the ghost: %d/3\n",
           evidence_hunters);
    printf("- Ghost Guess: %s\n", guess_name);
    printf("- Actual Ghost Type: %s\n\n", ghost_to_string(house.ghost.type));

    int hunters_win = (guess_valid &&
                       guessed_type == house.ghost.type &&
                       evidence_hunters > 0);

    printf("Overall Result: %s\n",
           hunters_win ? "Hunters Win!" : "Ghost Wins!");

    house_cleanup(&house);
    return 0;
}
