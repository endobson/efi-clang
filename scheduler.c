#include "scheduler.h"

#include "primitives.h"

void scheduler_start_task(void (*func)()) {
  enable_interrupts();

  func();
}
