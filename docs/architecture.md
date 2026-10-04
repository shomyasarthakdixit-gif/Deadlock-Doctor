# DeadlockDoctor Architecture

## Project Overview

DeadlockDoctor is a user-space Operating Systems and System Programming
project implemented in C on Ubuntu Linux.

It models processes competing for shared resources and demonstrates:

- Process management
- Process lifecycle and state transitions
- User-level scheduling
- Resource allocation
- Resource requests and release
- Concurrency and synchronization
- Deadlock detection
- Deadlock diagnosis
- Deadlock recovery

The project does not replace or modify the Linux kernel.

## Core Workflow

Process
→ Resource Request
→ Resource Unavailable
→ Process Waiting
→ Circular Dependency
→ Deadlock
→ Detection
→ Diagnosis
→ Recovery
→ System Continues

## Modules

### Process Manager

Responsible for:

- Process representation
- Process table
- Process lifecycle
- Process states
- Process creation
- Process termination
- State transitions

Primary files:

- src/process_manager.c
- include/process_manager.h

### Scheduler

Responsible for:

- READY queue
- User-level scheduling
- FCFS
- Round Robin if implemented

Primary files:

- src/scheduler.c
- include/scheduler.h

### Resource Manager

Responsible for:

- Resource representation
- Resource instances
- Availability
- Allocation
- Requests
- Release
- Waiting relationships

Primary files:

- src/resource.c
- include/resource.h

### Deadlock Detector

Responsible for:

- Detecting circular resource dependencies
- Identifying involved processes
- Identifying involved resources
- Reporting deadlock diagnostics

Primary files:

- src/deadlock.c
- include/deadlock.h

### Synchronization

Responsible for:

- Shared data protection
- POSIX threads where required
- Mutexes
- Condition variables where appropriate
- Semaphores where appropriate
- Prevention of race conditions

Primary files:

- src/synchronization.c
- include/synchronization.h

### Recovery

Responsible for:

- Selecting a recovery action
- Terminating/aborting a suitable process
- Releasing resources
- Updating system state
- Re-running deadlock detection

Primary files:

- src/recovery.c
- include/recovery.h

### CLI

Responsible for:

- User interaction
- Commands/menu
- Displaying system state
- Invoking project operations

Primary files:

- src/cli.c
- main integration as required

## Process States

The project uses:

NEW
READY
RUNNING
WAITING
TERMINATED

Example lifecycle:

NEW
→ READY
→ RUNNING
→ WAITING
→ READY
→ RUNNING
→ TERMINATED

## Deadlock Example

P1 holds R1 and waits for R2.

P2 holds R2 and waits for R1.

Therefore:

P1 → R2 → P2 → R1 → P1

This represents a circular dependency and can result in deadlock.

## Course Alignment

### CO-2

- Process abstraction
- Process lifecycle
- State transitions
- Creation
- Execution
- Synchronization
- Termination
- User-level scheduling

### CO-6

- Processes and threads
- POSIX threads
- Shared data
- Race conditions
- Mutexes
- Condition variables
- Semaphores
- Deadlocks
- Concurrency hazards

Supporting concepts from CO-1, CO-3 and CO-5 may be used only when they naturally support the project.

## Project Boundary

The following are not core requirements:

- Virtual memory implementation
- Page-table simulation
- Custom filesystem
- ext4 implementation
- Networking
- Kernel module
- Database
- Complex GUI
- Distributed deadlock detection
- Banker's Algorithm

## Team Responsibilities

Member 1:
Process Management + Scheduling

Member 2:
Resource Management + Deadlock Detection

Member 3:
Concurrency + Synchronization + Recovery + Integration
