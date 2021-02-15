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

void add_task(TaskDescriptor* task, void* stack, void* func) {

  // Push initial values onto the stack.
  uint64_t* u64_sp = (uint64_t*) stack;

  // First return address
  *(--u64_sp) = (uint64_t) scheduler_start_task;
  // Zero for all initial registers.
  for (int i = 0; i < 15; i++) {
    uint64_t val;
    if (i == 2) { // rcx
      val = (uint64_t) func;
    } else {
      val = 0;
    }
    *(--u64_sp) = val;
  }

  task->stack_pointer = u64_sp;

  // Add the task and mark as runnable.
  task->state = TaskState_Runnable;
  task->next = current_task->next;
  current_task->next = task;
}



void init_scheduler() {
  // Initialize root_task
  root_task.stack_pointer = 0;
  root_task.next = &root_task;
  root_task.state = TaskState_Runnable;

  // Initialize the current task as the root_task;
  current_task = &root_task;
}

void run_scheduler_loop() {
  while (1) {
    disable_interrupts();
    while (yield(TaskState_Runnable)) {}
    enable_interrupts_and_halt();
  }
}

