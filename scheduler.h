#ifndef SCHEDULER_H_
#define SCHEDULER_H_

typedef enum TaskState {
  TaskState_Runnable = 0,
  TaskState_Blocked = 1,
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


#endif // SCHEDULER_H_
