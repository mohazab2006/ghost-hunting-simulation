# Ghost Hunting Simulation

A multithreaded **ghost-hunting simulation** written in **C** for **COMP2401 Systems Programming**.  
This final project models a haunted house investigation where multiple hunter threads explore rooms, collect evidence, and attempt to identify the ghost before fear or boredom forces them out.

## Overview

The program simulates a haunted house called **Willow House**, where a team of hunters enters from the van, moves through connected rooms, gathers evidence, and shares findings through a common case file. At the same time, a ghost moves independently through the house, leaving behind clues and reacting to hunter presence. The simulation ends when the hunters either successfully identify the ghost or fail to collect enough valid evidence in time. :contentReference[oaicite:0]{index=0}

This project was built as a **COMP2401 Systems Programming final project**, with a strong focus on:
- multithreading with `pthread`
- synchronization with semaphores
- dynamic memory management
- structs, enums, and modular C design
- shared-state simulation logic :contentReference[oaicite:1]{index=1}

## Features

- **Multithreaded simulation**
  - one thread for the ghost
  - one thread for each hunter :contentReference[oaicite:2]{index=2}

- **Shared case file**
  - all hunters contribute evidence to a common file
  - the case is considered solved when at least three unique evidence types are collected :contentReference[oaicite:3]{index=3}

- **Room-based house graph**
  - the haunted house is made of connected rooms
  - hunters and the ghost move through this graph during the simulation :contentReference[oaicite:4]{index=4}

- **Evidence-driven ghost identification**
  - evidence is represented with bitmasks
  - ghost types are defined as unique combinations of three evidence types :contentReference[oaicite:5]{index=5}

- **Concurrency protection**
  - semaphores are used to protect shared room state and the case file
  - helper functions support safe locking of multiple rooms at once :contentReference[oaicite:6]{index=6}

- **End-of-game summary**
  - displays hunter exit reasons
  - shows collected evidence checklist
  - compares the hunters’ ghost guess with the actual ghost type
  - prints the final result: **Hunters Win** or **Ghost Wins** :contentReference[oaicite:7]{index=7}

## How It Works

1. The house is initialized and populated with rooms.
2. The ghost is created and placed in the house.
3. The user enters hunter names and IDs.
4. Once setup is complete, the program starts:
   - one ghost thread
   - one thread per hunter
5. Hunters explore the house, collect evidence, and return results to the shared case file.
6. The ghost moves around independently and influences the simulation.
7. After all threads finish, the program prints a final investigation report. :contentReference[oaicite:8]{index=8}

## Evidence Types

The simulation includes the following evidence types:

- EMF
- Orbs
- Radio
- Temperature
- Fingerprints
- Writing
- Infrared :contentReference[oaicite:9]{index=9}

Each ghost type is represented as a unique combination of three evidence types, which allows the hunters to identify the correct ghost if they gather a valid set. :contentReference[oaicite:10]{index=10}

## Project Structure

```text
.
├── Makefile
├── defs.h
├── helpers.h
├── main.c
├── casefile.c
├── house.c
├── room.c
├── movement.c
├── hunter.c
├── ghost.c
├── stack.c
└── helpers.c
