# Ghost Hunting Simulation

A multithreaded C simulation where a team of hunters explores a haunted house to identify a ghost by collecting evidence before fear or boredom forces them to leave.

## Overview

This project simulates a ghost-hunting scenario inside a house with multiple connected rooms. Hunters and the ghost run as separate threads, interacting with the environment at the same time.

Hunters must gather enough **unique evidence** and return safely to the van to correctly identify the ghost. If they fail, the ghost wins.

## Features

* Multithreaded simulation using `pthread`
* 1 ghost thread and up to 4 hunter threads
* Randomized ghost type, movement, and behaviour
* Different hunter devices for collecting evidence
* Shared case file used by all hunters
* Fear and boredom system affecting hunters
* CSV logging of hunter and ghost activity
* Final investigation summary with results

## How It Works

* Hunters start in the **Van** and move through connected rooms
* Each hunter has a device that collects specific evidence types
* The ghost moves, leaves evidence, or idles
* Hunters update a shared case file with collected evidence
* The simulation ends when:

  * Enough evidence is collected (hunters win), or
  * Hunters leave due to fear/boredom (ghost wins)

## Build & Run

```bash
make
./project
```

## Concepts Used

* Multithreading (`pthread`)
* Synchronization (semaphores / locks)
* Structs and enums in C
* Dynamic memory management
* Simulation design and state handling
