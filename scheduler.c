#include "scheduler.h"

#include "primitives.h"

// The root/idle task descriptor.
TaskDescriptor root_task;

// The currently running task.
TaskDescriptor* current_task = 0;

int yield(TaskState new_old_state) {
  TaskDescriptor* old_task = current_task;
  TaskDescriptor* new_task = old_task->next;

  while (new_task != old_task) {
    if (new_task->state == TaskState_Runnable) break;
    new_task = new_task->next;
  }

  int ret_val = 0;
  if (new_task != old_task) {
    ret_val = 1;
    old_task->state = new_old_state;
    current_task = new_task;
    switch_to_task(old_task, new_task);
  }

  return ret_val;
}

void scheduler_start_task(void (*func)()) {
  enable_interrupts();

  func();
}
