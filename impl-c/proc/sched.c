#include "proc/sched.h"
#include "proc/task.h"

#include "list.h"
#include "mm.h"
#include "uart.h"

// proc/sched.S
void switch_to(struct task_struct *prev, struct task_struct *next);

static void kill_zombies();

struct list_head run_queue;
struct list_head exited;

void scheduler_init() {
  list_init(&run_queue);
  list_init(&exited);
}

void task_schedule() {
  uart_println("Scheduler called");
  struct task_struct *cur = get_current();
  struct task_struct *next;

  struct task_entry *entry_next;
  if (!list_empty(&run_queue)) {
    while (1) {
      entry_next = (struct task_entry *)list_pop_front(&run_queue);
      next = entry_next->task;
      if (next->status == TASK_STATUS_ALIVE) {
        list_push(&entry_next->list, &run_queue);
        break;
      } else {
        list_push(&entry_next->list, &exited);
      }
    }
    uart_println("cur:%d next:%d", cur->id, next->id);
    switch_to(cur, next);
  }
}

void idle() {
  while (1) {
    uart_println("idle scheduling...");
    _wait();
    task_schedule();
    kill_zombies();
  }
}

void kill_zombies() {
  struct list_head *entry;
  struct task_struct *task;
  while (!list_empty(&exited)) {
    entry = list_pop(&exited);
    task = ((struct task_entry *)entry)->task;
    uart_println("recycle space for task:%d", task->id);
    list_del(entry);
    kfree(task);
    kfree(entry);
  }
}

void _dump_runq() {
  struct list_head *list_head = &run_queue;
  struct list_head *entry;
  uart_printf("Runqueue");
  int i = 0;
  for (entry = list_head->next; entry != list_head; entry = entry->next) {
    struct task_struct *task = ((struct task_entry *)entry)->task;
    uart_printf("->[%d, %x]", task->id, task);
    i++;
  }
  uart_println("");
}