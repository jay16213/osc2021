#ifndef _SCHED_H_
#define  _SCHED_H_

#include "list.h"
#include "type.h"

#define MAX_USTACK_NUM 20

typedef unsigned int pid_t;

typedef enum {
    running,
    ready,
    wait,
    dead,
    exit
} thread_state_t;

typedef struct {
    uint64_t x19;
    uint64_t x20;
    uint64_t x21;
    uint64_t x22;
    uint64_t x23;
    uint64_t x24;
    uint64_t x25;
    uint64_t x26;
    uint64_t x27;
    uint64_t x28;
    uint64_t fp;
    uint64_t lr;
    uint64_t sp;
} callee_reg_t;

typedef struct {
    callee_reg_t   reg;
    pid_t          pid;
    void           *kstack_addr;
    void           *ustack_addr[MAX_USTACK_NUM];
    uint16_t       ustack_num;
    uint16_t       wait_time;
    size_t         read_size;
    thread_state_t state;
    list_head_t    node;
} thread_t;

thread_t* thread_create(void (*task)(void));
void thread_exit(void);
void sched_init(void);
void schedule(void);
void idle(void);
void times_up(void);

#endif
