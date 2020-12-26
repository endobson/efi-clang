#ifndef SCHEDULER_H_
#define SCHEDULER_H_

typedef enum TaskState {
  TaskState_Runnable,
  TaskState_Blocked,
} TaskState;

typedef struct TaskDescriptor {
  void* stack_pointer;
  struct TaskDescriptor* next;
  TaskState state;
} __attribute__ ((packed)) TaskDescriptor;

// Yield to another task, and move to the new state.
//
// Returns true if another task was available to yield to.
int yield(TaskState new_old_state);


// Adds a new task to be run.
//
// Must be called with interrupts disabled.
void add_task(TaskDescriptor* task, void* stack, void* func);

// Set up the initial scheduler data structures.
void init_scheduler();

// Start the scheduler loop.
void run_scheduler_loop();

#endif // SCHEDULER_H_
