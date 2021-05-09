#include "shell.h"
#include "pf_alloc.h"
#include "thread.h"
#include "system_call.h"
#include "kernel.h"
#include "base_ops.h"
#include "vfs.h"
#include "tmpfs.h"

#include "io.h"

extern struct filesystem tmpfs;

void idle()
{
    while ( 1 ) {
        printf("idle\n");
        sys_schedule();
        wait(1000000);
    }
}

void kernel_main()
{
    // initialization
    init_page_frame();
    init_thread_pool();

    // mount root file system
    register_filesystem(&tmpfs);
    init_root("tmpfs");

    // default pseudo thread to set first tpidr
    struct Thread t;
    t.tid = 0;
    t.pid = 0;
    t.kernel_sp = KERNEL_STACK_TOP;

    set_current_thread(&t);
    
    // create shell and idle threads
    create_thread(shell);
    create_thread(idle);

    // start scheduling
    sys_schedule();
}