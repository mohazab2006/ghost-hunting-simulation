# Ghost Hunting Simulation

**Multithreaded C simulation — COMP2401 Systems Programming, Carleton University**

A concurrent simulation where a team of hunters explores a haunted house to identify a ghost by collecting evidence. Hunters and the ghost run as independent threads, interacting through shared state protected by synchronization primitives.

---

## Overview

| Field | Detail |
|---|---|
| **Course** | COMP2401 — Introduction to Systems Programming |
| **Institution** | Carleton University |
| **Language** | C (C99) |
| **Key concepts** | Multithreading, synchronization, dynamic memory, simulation design |

---

## How it works

Hunters start in the **Van** and move through randomly connected rooms collecting evidence. The ghost moves independently, leaving clues or idling. A shared case file accumulates evidence across all hunter threads.

**Hunters win** if they collect enough unique evidence and return safely to the Van. **Ghost wins** if all hunters flee due to fear or boredom.

---

## Concurrency Design

The core challenge is coordinating 4 hunter threads and 1 ghost thread across shared rooms, a shared case file, and a shared event log — without race conditions or deadlocks.

- **Mutexes** protect room state during movement and evidence collection
- **Semaphores** control access to the shared case file
- **Condition variables** coordinate multi-thread termination via shared exit flags
- **pthreads** manages thread lifecycle — creation, joining, and cleanup

All shared state access is guarded. The simulation is designed so threads can be added or removed without restructuring the synchronization model.

---

## Features

- 1 ghost thread + up to 4 concurrent hunter threads
- Randomized ghost type, starting room, and movement behavior each run
- Each hunter carries a different device detecting a specific evidence type
- Fear and boredom meters force hunters to flee if they stagnate
- CSV activity log captures every ghost and hunter action
- Final investigation summary printed on exit

---

## Project Structure

| File | Role |
|---|---|
| `main.c` | Entry point, house setup, thread launch |
| `defs.h` | All type definitions, constants, enums |
| `ghost.c` | Ghost thread logic |
| `hunter.c` | Hunter thread logic |
| `movement.c` | Room traversal for both agents |
| `house.c` | House and room initialization |
| `room.c` | Room data management |
| `casefile.c` | Shared case file — mutex-protected reads/writes |
| `stack.c` | Stack for evidence tracking |
| `helpers.c/h` | Utility functions |

---

## Build & Run

```
make
./project
```

Each run is non-deterministic — ghost type, room layout, and starting positions are randomized. Activity is logged to CSV for post-run analysis.
