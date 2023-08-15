#include "ulock.h"
#include "x86.h"
#include "defs.h"

void init_lock(struct lock_t *s) {
    s->locked = 0;
}

void 
acquire_lock(struct lock_t *s) 
{
    /* Your Code Lab-9 */

    // cprintf("in acquire_lock\n");

    // from spinlock.c

    // https://stackoverflow.com/questions/60497547/why-do-we-need-pushcli-before-xchg
    // xchg ensures atomicity between threads, but interrupt handlers are still a problem.
    // ** see SO link -- e.g. if this thread has acquired lock (and not released),
    // and interrupt occurs, and interrupt handler wants this lock => it will be stuck in 
    // the while loop, and by definition an interrupt handler must be executed until 
    // its end before the CPU can continue doing anything else.
    // => need to restart system

    // but here, we're just using the locks in testcases -- so mp interrupt handler
    // won't try to acquire this lock...

    // pushcli(); // disable interrupts to avoid deadlock.          // panic: sched locks
                                                                    // grep -rnw 'sched locks' .
                                                                    // => sched()

    // The xchg is atomic.
    while(xchg(&(s->locked), 1) != 0);     // my: like in theory quiz-2,
                                        // returns prev value of s->locked
                                        // (and sets it to 1) -- so if it's 1 already,
                                        // sets it to 1_new(=1) and returns 1_prev
                                        // then we know that lock is already held...

/*  // NEEDED HERE?? (read the comment -- "past this point")
    // Tell the C compiler and the processor to not move loads or stores
    // past this point, to ensure that the critical section's memory
    // references happen after the lock is acquired.
    __sync_synchronize();
// */
}

void 
release_lock(struct lock_t *s) 
{
    /* Your Code Lab-9 */

    // cprintf("in release_lock\n");

    // Tell the C compiler and the processor to not move loads or stores
    // past this point, to ensure that all the stores in the critical
    // section are visible to other cores before the lock is released.
    // Both the C compiler and the hardware may re-order loads and
    // stores; __sync_synchronize() tells them both not to.

    // https://stackoverflow.com/questions/982129/what-does-sync-synchronize-do
    __sync_synchronize();

    // Release the lock, equivalent to lk->locked = 0.
    // This code can't use a C assignment, since it might
    // not be atomic. A real OS would use C atomics here.
    asm volatile("movl $0, %0" : "+m" (s->locked) : );

    // popcli();
}


void 
init_sem(struct sem_t *s, int initval)
{
    /* Your Code */

    // *** SEE https://www.youtube.com/watch?v=nc03pTD53Ro for sleeplock
    // sleeplock.c explanation: the spinlock sleeplock->lk is to 
    // protect the sleeplock variables!!
    // e.g., used below

    init_lock(&(s->lk));
    acquire_lock(&(s->lk));     // so that many threads don't do this
    s->val = initval;
    release_lock(&(s->lk));
}

void
up_sem(struct sem_t *s) 
{
    /* Your Code */

    acquire_lock(&s->lk);
    s->val++;           // do this BEFORE wakeup
    signal_sem(s);      // do this under lock √
    release_lock(&s->lk);

}

void 
down_sem(struct sem_t *s) 
{
    /* Your Code */

    // sleeplock.c

    acquire_lock(&s->lk);
    // while(s->val==0) {
    //     sleep(s, &s->lk);        // second arg must be a spinlock
                                    // hence, done in proc.c
                                    // block_sem uses ptable.lock as proxy spinlock
    // }
    // s->val--;
    while(s->val==0)
        block_sem(s);
    s->val--;
    release_lock(&s->lk);

}
