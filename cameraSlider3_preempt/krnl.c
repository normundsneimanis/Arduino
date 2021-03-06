/*****************************************************
*                                                    *
*                                                    *
*  __  _    ___  ____   ____     ___  _              *
* |  |/ ]  /  _]|    \ |    \   /  _]| |             *
* |  ' /  /  [_ |  D  )|  _  | /  [_ | |             *
* |    \ |    _]|    / |  |  ||    _]| |___          *
* |     ||   [_ |    \ |  |  ||   [_ |     |         *
* |  .  ||     ||  .  \|  |  ||     ||     |         *
* |__|\_||_____||__|\_||__|__||_____||_____|         *
*                                                    *
*                                                    *
*     krnl.c  part of kernel KRNL                    *
*           based on "snot"                          *
*                                                    *
*  June,               2014                          *
*  Feb                 2015                          *
*                      2016                          *
*      Author: jdn                                   *
*                                                    *
******************************************************
*                                                    *
*            (simple not - not ?! :-) )              *
* my own small KeRNeL adapted for Arduino            *
*                                                    *
* this version adapted for Arduino                   *
*                                                    *
* (C) 2012,2013,2014,2015                            *
*                                                    *
* Jens Dalsgaard Nielsen <jdn@es.aau.dk>             *
* http://es.aau.dk/staff/jdn                         *
* Section of Automation & Control                    *
* Aalborg University,                                *
* Denmark                                            *
*                                                    *
* "THE BEER-WARE LICENSE" (frit efter PHK)           *
* <jdn@es.aau.dk> wrote this file. As long as you    *
* retain this notice you can do whatever you want    *
* with this stuff. If we meet some day, and you think*
* this stuff is worth it ...                         *
*  you can buy me a beer in return :-)               *
* or if you are real happy then ...                  *
* single malt will be well received :-)              *
*                                                    *
* Use it at your own risk - no warranty              *
*                                                    *
* tested with duemilanove w/328, uno R3,             *
* seeduino 1280 and mega2560                         *
*****************************************************/

//http://www.nongnu.org/avr-libc/user-manual/FAQ.html#faq_cplusplus nice info

#include "krnl.h"
 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>

// CPU frequency - for adjusting delays
#if (F_CPU == 16000000)
#pragma message ("krnl detected 16 MHz" )
#else
#pragma message ("krnl detected 8 MHz")
#endif

#if (KRNL_VRS != 2016056)
#error "KRNL VERSION NOT UPDATED in krnl.c "
#endif

/*
#ifdef __cplusplus
extern "C" {
#endif
*/

/* which timer to use for krnl heartbeat
* timer 0 ( 8 bit) is normally used by millis - avoid !
* timer 1 (16 bit)  DEFAULT
* timer 2 ( 8 bit)
* timer 3 (16 bit) 1280/1284P/2560 only (MEGA)
* timer 4 (16 bit) 1280/2560 only (MEGA)
* timer 5 (16 bit) 1280/2560 only (MEGA)
*/

#if (KRNLTMR == 0)

// normally not goood bq of arduino sys timer so you wil get a compile error
// 8 bit timer !!!
#define KRNLTMRVECTOR TIMER0_OVF_vect
#define TCNTx TCNT0
#define TCCRxA TCCR0A
#define TCCRxB TCCR0B
#define TCNTx TCNT0
#define OCRxA OCR0A
#define TIMSKx TIMSK0
#define TOIEx TOIE0
#define PRESCALE 0x07
#define COUNTMAX 255
#define DIVV 15.625
#define DIVV8 7.812

#elif (KRNLTMR == 1)

#define KRNLTMRVECTOR TIMER1_OVF_vect
#define TCNTx TCNT1
#define TCCRxA TCCR1A
#define TCCRxB TCCR1B
#define TCNTx TCNT1
#define OCRxA OCR1A
#define TIMSKx TIMSK1
#define TOIEx TOIE1
#define PRESCALE 0x03
#define COUNTMAX 0xffff
#define DIVV 250
#define DIVV8 (DIVV/2)

#elif (KRNLTMR == 2)

// 8 bit timer !!!
#define KRNLTMRVECTOR TIMER2_OVF_vect
#define TCNTx TCNT2
#define TCCRxA TCCR2A
#define TCCRxB TCCR2B
#define TCNTx TCNT2
#define OCRxA OCR2A
#define TIMSKx TIMSK2
#define TOIEx TOIE2
#define PRESCALE 0x07
#define COUNTMAX 255
#define DIVV 15.625
#define DIVV8 7.812

#elif (KRNLTMR == 3)

#define KRNLTMRVECTOR TIMER3_OVF_vect
#define TCNTx TCNT3
#define TCCRxA TCCR3A
#define TCCRxB TCCR3B
#define TCNTx TCNT3
#define OCRxA OCR3A
#define TIMSKx TIMSK3
#define TOIEx TOIE3
#define PRESCALE 0x03
#define COUNTMAX 0xffff
#define DIVV 250
#define DIVV8 (DIVV/2)

#elif (KRNLTMR == 4)

#define KRNLTMRVECTOR TIMER4_OVF_vect
#define TCNTx TCNT4
#define TCCRxA TCCR4A
#define TCCRxB TCCR4B
#define TCNTx TCNT4
#define OCRxA OCR4A
#define TIMSKx TIMSK4
#define TOIEx TOIE4
#define PRESCALE 0x03
#define COUNTMAX 0xffff
#define DIVV 250
#define DIVV8 (DIVV/2)

#elif (KRNLTMR == 5)

#define KRNLTMRVECTOR TIMER5_OVF_vect
#define TCNTx TCNT5
#define TCCRxA TCCR5A
#define TCCRxB TCCR5B
#define TCNTx TCNT5
#define OCRxA OCR5A
#define TIMSKx TIMSK5
#define TOIEx TOIE5
#define PRESCALE 0x03
#define COUNTMAX 0xffff
#define DIVV 250
#define DIVV8 (DIVV/2)

#else

#pragma err "KRNL: no valid tmr selected"

#endif

//----------------------------------------------------------------------------

struct k_t *task_pool,			// array of descriptors for tasks
        *sem_pool,						// .. for semaphores
        AQ,							// Q head for active Q
        *pmain_el,						// procesdecriptor for main
        *pAQ,							// head of activeQ (AQ)
        *pDmy,							// ref to dummy task
        *pRun,							// who is running ?
        *pSleepSem;						// one semaphor for all to sleep at

struct k_msg_t *send_pool;		// ptr to array for msg sem pool

int k_task, k_sem, k_msg;		// how many did you request in k_init of descriptors ?
char nr_task = 0, nr_sem = 0, nr_send = 0;	// counters for created KeRNeL items

volatile char krnl_preempt_flag = 1;	//1: preempt, 0 : non preempt
volatile char k_running = 0, k_err_cnt = 0;
volatile unsigned int tcntValue;	// counters for timer system
volatile int fakecnt,			// counters for letting timer ISR go multipla faster than krnl timer
         fakecnt_preset;				// ...


static volatile char stopp = 0;	// main will loop on stop as dummy task

unsigned long k_millis_counter = 0;
unsigned int k_tick_size;
 
int tmr_indx;					// for travelling Qs in tmr isr

/******/

void k_eat_time(unsigned int eatTime)
{
    while (eatTime > 10) { // _delay_us() performs busywait
        eatTime -= 10;
        _delay_us(10000);
    }

    while (eatTime > 0) {
        eatTime -= 1;
        _delay_us(1000);
    }
}

//---QOPS---------------------------------------------------------------------

void enQ(struct k_t *Q, struct k_t *el)
{
    el->next = Q;
    el->pred = Q->pred;
    Q->pred->next = el;
    Q->pred = el;
}

struct k_t *deQ(struct k_t *el)
{
    el->pred->next = el->next;
    el->next->pred = el->pred;

    return (el);
}

void prio_enQ(struct k_t *Q, struct k_t *el)
{
    char prio = el->prio;

    Q = Q->next;				// bq first elm is Q head itself

    while (Q->prio <= prio) {	// find place before next with lower prio
        Q = Q->next;
    }

    el->next = Q;
    el->pred = Q->pred;
    Q->pred->next = el;
    Q->pred = el;
}

//---HW timer IRS--timer section------------------------

/*
 * The KRNL Timer is driven by timer
 *
 * Install the Interrupt Service Routine (ISR) for Timer2 overflow.
 * This is normally done by writing the address of the ISR in the
 * interrupt vector table but conveniently done by using ISR()
 *
 * Timer2 reload value, globally available
 */
struct k_t *pE;

ISR(KRNLTMRVECTOR, ISR_NAKED)  // naked so we have to supply with prolog and epilog (push pop stack of regs)
{

    PUSHREGS();					// no local vars ! I think
  
    TCNTx = tcntValue;			// Reload the timer

    if (!k_running) {
        goto exitt;
    }

    fakecnt--;					// for very slow k_start values
    //bq timer cant run so slow (8 bit timers at least)
    if (0 < fakecnt) {		// how often shall we run KeRNeL timer code ?
        goto exitt;
    }

    fakecnt = fakecnt_preset;	// now it's time for doing RT stuff

    k_millis_counter += k_tick_size;	// my own millis counter

    // the following may look crazy: to go through all semaphores and tasks
    // but you may have 3-4 tasks and 3-6 semaphores in your code
    // so - seems to be efficient :-)
    // so - it's a good idea not to init krnl with more items
    // (tasks/Sem/msg descriptors than needed)

    // Semaphore timer - check timers on semaphores - they may be cyclic
    pE = sem_pool;

    for (tmr_indx = 0; tmr_indx < nr_sem; tmr_indx++) {
        if (0 < pE->cnt2) {		// timer on semaphore ?
            pE->cnt2--;			// yep  decrement it
            if (pE->cnt2 <= 0) {	// timeout  ?
                pE->cnt2 = pE->cnt3;	// preset again - if cnt3 == 0 and >= 0 the rep timer
                ki_signal(pE);	//issue a signal to the semaphore
            }
            pE++;
        }
    }

    // Chk timers on tasks - they may be one shoot waiting
    pE = task_pool;

    for (tmr_indx = 0; tmr_indx < nr_task; tmr_indx++) {
        if (0 < pE->cnt2) {		// timer active on task ?
            pE->cnt2--;			// yep so let us do one down count
            if (pE->cnt2 <= 0) {	// timeout ? ( == 0 )
                ((struct k_t *)(pE->cnt3))->cnt1++;	// leaving sem so adjust semcount on sem
                prio_enQ(pAQ, deQ(pE));	// and rip task of semQ and insert in activeQ
                pE->cnt2 = -1;	// indicate timeout in this semQ
            }
        }
        pE++;
    }


    if (krnl_preempt_flag) {
        prio_enQ(pAQ, deQ(pRun));	// round robbin
        K_CHG_STAK();
    }

exitt:
    POPREGS();
    RETI();
}

//----------------------------------------------------------------------------
// inspired from ...
// http://arduinomega.blogspot.dk/2011/05/timer2-and-overflow-interrupt-lets-get.html
// Inspiration from  http://popdevelop.com/2010/04/mastering-timer-interrupts-on-the-arduino/
// Inspiration from "Multitasking on an AVR" by Richard Barry, March 2004
// and  http://www.control.aau.dk/~jdn/kernels/krnl/
//----------------------------------------------------------------------------
// avrfreaks.net
// and my old kernel from last century
// and a lot other stuff
// basic concept from my own very old kernels dated back bef millenium

void __attribute__ ((naked, noinline)) ki_task_shift(void)
{
    PUSHREGS();					// push task regs on stak so we are rdy to task shift
    K_CHG_STAK();
    POPREGS();					// restore regs
    RETI();						// and do a reti NB this also enables interrupt !!!
}

struct k_t *k_crt_task(void (*pTask) (void), char prio, char *pStk, int stkSize)
{

    struct k_t *pT;
    int i;
    char *s;

    if ( (k_running) || ((prio <= 0) || (DMY_PRIO < prio))
            || (k_task <= nr_task)) {
        goto badexit;
    }

    pT = task_pool + nr_task;	// lets take a task descriptor
    pT->nr = nr_task;
    nr_task++;

    pT->cnt2 = 0;				// no time out running on you for the time being
    pT->cnt3 = 0;				// no time out semaphore

    pT->cnt1 = (int)(pStk);		// ref to my stack

    // stack paint :-)
    for (i = 0; i < stkSize; i++) {	// put hash code on stak to be used by k_unused_stak()
        pStk[i] = STAK_HASH;
    }

    s = pStk + stkSize - 1;		// now we point on top of stak
    *(s--) = 0x00;				// 1 byte safety distance :-)

    // an interrupt do only push PC on stack by HW - can be 2 or 3 bytes
    // depending of 368/.../1280/2560
    *(s--) = lo8(pTask);		//  so top now holds address of function
    *(s--) = hi8(pTask);		// which is code body for task

    // NB  NB 2560 use 3 byte for call/ret addresses the rest only 2
#if defined (__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
    *(s--) = EIND;				// best guess : 3 byte addresses !!! or just 0
#endif

    // r1 is the socalled zero value register
    // see https://gcc.gnu.org/wiki/avr-gcc
    // can tmp be non zero (multiplication etc)
    *(s--) = 0x00;				// r1
    *(s--) = 0x00;				// r0
    *(s--) = 0x00;				// sreg

    //1280 and 2560 need to save rampz reg just in case
#if defined (__AVR_ATmega2560__) || defined (__AVR_ATmega1280__) || defined (__AVR_ATmega1284P__) || defined(__AVR_ATmega2561__)
    *(s--) = RAMPZ;				// best guess  0x3b
    // obsolete JDN    *(s--) = EIND;             // best guess
#endif

#if defined (__AVR_ATmega2560__) || defined (__AVR_ATmega1280__) || defined(__AVR_ATmega2561__)
    *(s--) = EIND;				// best guess 0x3c
#endif

    for (i = 0; i < 30; i++) {	//r2-r31 = 30 regs
        *(s--) = 0x00;
    }

    pT->sp_lo = lo8(s);			// now we just need to save stakptr
    pT->sp_hi = hi8(s);			// in thread descriptor

    // HW DEPENDENT PART - ENDE

    pT->prio = prio;			// maxv for holding org prio for inheritance
    pT->maxv = (int)prio;
    prio_enQ(pAQ, pT);			// and put task in active Q

    return (pT);

badexit:
    k_err_cnt++;
    return (NULL);
}

int freeRam(void)
{
    extern int __heap_start, *__brkval;
    int v;

    // hw specific :-/
    return ((int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval));

    /* from http://www.noah.org/wiki/Arduino_notes#debug_memory_problems_.28out_of_RAM.29
    int freeRam () {
    // __brkval is the address of the top of the heap if memory has been allocated.
    // If __brkval is zero then it means malloc has not used any memory yet, so
    // we look at the address of __heap_start.
    extern int __heap_start
    extern int *__brkval; // address of the top of heap
    int stack_top;
    return (int)&stack_top - ((int)__brkval == 0 ? (int)&__heap_start : (int)__brkval);
    }
    */
}

int k_sleep(int time)
{
    return k_wait(pSleepSem, time);
}

int k_unused_stak(struct k_t *t)
{
    int i = 0;
    char *pstk;

    if (t) {					// another task or yourself - NO CHK of validity !!!!!
        pstk = (char *)(t->cnt1);
    } else {
        pstk = (char *)(pRun->cnt1);
    }

    DI();
    // look for stack paint
    while (*pstk == STAK_HASH) {
        pstk++;
        i++;
    }
    EI();

    return (i);
}

int k_set_prio(char prio)
{
    int i;

    if (!k_running) {
        return (-1);
    }

    DI();

    if ((prio <= 0) || (DMY_PRIO <= prio)) { // not legal value my friend
        EI();
        return (-2);
    }
    i = pRun->prio;

    pRun->prio = prio;
    prio_enQ(pAQ, deQ(pRun));
    ki_task_shift();

    EI();

    return (i);
}

struct k_t *k_crt_sem(char init_val, int maxvalue)
{
    struct k_t *sem;

    if (k_running) {
        return (NULL);
    }

    if ((init_val < 0) || (32000 < init_val) || (maxvalue < -32000)
            || (32000 < maxvalue)) {
        goto badexit;
    }

    if (k_sem <= nr_sem) {
        goto badexit;
    }

    sem = sem_pool + nr_sem;
    sem->nr = nr_sem;
    nr_sem++;

    sem->cnt2 = 0;				// no timer running
    sem->next = sem->pred = sem;
    sem->prio = QHD_PRIO;
    sem->cnt1 = init_val;

    sem->maxv = maxvalue;
    sem->clip = 0;

    return (sem);

badexit:
    k_err_cnt++;

    return (NULL);
}

int k_set_sem_timer(struct k_t *sem, int val)
{
    // there is no k_stop_sem_timer fct just call with val== 0

    if (val < 0) {
        return (-1);			// bad value
    }

    DI();
    sem->cnt2 = sem->cnt3 = val;	// if 0 then timer is not running -
    EI();

    return (0);
}

int ki_signal(struct k_t *sem)
{
    DI();						// just in case
    if (sem->cnt1 < sem->maxv) {
        sem->cnt1++;				// Salute to Dijkstra

        if (sem->cnt1 <= 0) {
            sem->next->cnt2 = 0;	// return code == ok
            prio_enQ(pAQ, deQ(sem->next));
            return (0);
        }
        return (1);					// just delivered a signal - no task was waitin
    }

    if (32000 > sem->clip) {
        sem->clip++;
    }
    // here we are on bad clip failure no signal takes place
    // signal is lost !!!
#ifdef KRNLBUG
    k_sem_clip(sem->nr, sem->clip);
#endif
    return (-1);
}

int k_signal(struct k_t *sem)
{
    int res;

    DI();

    res = ki_signal(sem);		// 1: ok no task to AQ, 0: ok task to AQ

    if (res == 0) {
        if (krnl_preempt_flag)
            ki_task_shift();		// bq maybe started task has higher prio than me
    }

    EI();

    return (res);
}

int ki_wait(struct k_t *sem, int timeout)
{
// used by msg system
    DI();

    if (0 < sem->cnt1) {
        sem->cnt1--;			// Salute to Dijkstra
        return (1);				// ok: 1 bq we are not suspended
    }

    if (timeout < 0) {		// no luck, dont want to wait so bye bye
        return (-1);
    }
    // from here we want to wait
    pRun->cnt2 = timeout;		//  0 == wait forever

    if (timeout) {
        pRun->cnt3 = (int)sem;	// nasty keep ref to semaphore
    }
    //  so we can be removed if timeout occurs

    sem->cnt1--;				// Salute to Dijkstra

    enQ(sem, deQ(pRun));
    ki_task_shift();

    // back again - have semaphore received signal or timeout ?
    pRun->cnt3 = 0;				// reset ref to timer semaphore

    return ((char)(pRun->cnt2));	// 0: ok, 1: ok - no suspend , -1: timeout
}

int k_wait(struct k_t *sem, int timeout)
{
    int retval;
    DI();
    retval = ki_wait(sem, timeout);
    EI();
    return retval;				// 0: ok, -1: timeout
}

int k_wait_lost(struct k_t *sem, int timeout, int *lost)
{
    DI();
    if (lost != NULL) {
        *lost = sem->clip;
        sem->clip = 0;
    }
    return k_wait(sem, timeout);
}

int k_sem_signals_lost(struct k_t *sem)
{
    int x;

    DI();
    x = sem->clip;
    sem->clip = 0;
    EI();
    return x;
}

int ki_semval(struct k_t *sem)
{
    DI();

    return (sem->cnt1);
}

struct k_msg_t *k_crt_send_Q(int nr_el, int el_size, void *pBuf)
{
    struct k_msg_t *pMsg;

    if (k_running) {
        return (NULL);
    }

    if (k_msg <= nr_send) {
        goto errexit;
    }

    if (k_sem <= nr_sem) {
        goto errexit;
    }

    pMsg = send_pool + nr_send;
    pMsg->nr = nr_send;			// I am element nr nr_send in msgQ pool
    nr_send++;

    pMsg->sem = k_crt_sem(0, nr_el);

    if (pMsg->sem == NULL) {
        goto errexit;
    }

    pMsg->pBuf = (char *)pBuf;
    pMsg->r = pMsg->w = -1;
    pMsg->el_size = el_size;
    pMsg->nr_el = nr_el;
    pMsg->lost_msg = 0;
    pMsg->cnt = 0;				// count nr elm in Q

    return (pMsg);

errexit:
    k_err_cnt++;
    return (NULL);
}

char ki_send(struct k_msg_t *pB, void *el)
{

    int i;
    char *pSrc, *pDst;

    if (pB->nr_el <= pB->cnt) {
        // nope - no room for a putting new msg in Q ?
        if (pB->lost_msg < 32000) {
            pB->lost_msg++;
        }
#ifdef KRNLBUG
        k_send_Q_clip(pB->nr, pB->lost_msg);
#endif
        return (-1);			// nope
    }

    pB->cnt++;

    pSrc = (char *)el;

    pB->w++;
    if (pB->nr_el <= pB->w) {	// simple wrap around
        pB->w = 0;
    }

    pDst = pB->pBuf + (pB->w * pB->el_size);	// calculate where we shall put msg in ringbuf

    for (i = 0; i < pB->el_size; i++) {
        // copy to Q
        *(pDst++) = *(pSrc++);
    }

    return (ki_signal(pB->sem));	// indicate a new msg is in Q

}

char k_send(struct k_msg_t *pB, void *el)
{
    char res;

    DI();

    res = ki_send(pB,el);
    if (res == 0) {				// if new task in AQ == someone was waiting for msg
        if (krnl_preempt_flag)
            ki_task_shift();
    }

    EI();
    return (res);
}

char ki_receive(struct k_msg_t *pB, void *el, int *lost_msg)
{
    int i;
    char r, *pSrc, *pDst;

    // can be called from ISR bq no blocking
    DI();						// just to be sure

    if ((r = ki_wait(pB->sem, -1)) >= 0) {

        pDst = (char *)el;
        pB->r++;
        pB->cnt--;				// got one

        if (pB->nr_el <= pB->r) {
            pB->r = 0;
        }

        pSrc = pB->pBuf + pB->r * pB->el_size;

        for (i = 0; i < pB->el_size; i++) {
            *(pDst++) = *(pSrc++);
        }
        if (lost_msg) {
            *lost_msg = pB->lost_msg;
            pB->lost_msg = 0;
        }
        return (r);				// yes
    }

    return (-1);				// nothing for you my friend
}

char k_receive(struct k_msg_t *pB, void *el, int timeout, int *lost_msg)
{

    int i;
    char r, *pSrc, *pDst;

    DI();

    if (0 <= (r = ki_wait(pB->sem, timeout))) {
        // ki_wait bq then intr is not enabled when coming back
        pDst = (char *)el;
        pB->r++;
        pB->cnt--;				// got one

        if (pB->nr_el <= pB->r) {
            pB->r = 0;
        }

        pSrc = pB->pBuf + pB->r * pB->el_size;

        for (i = 0; i < pB->el_size; i++) {
            *(pDst++) = *(pSrc++);
        }

        if (lost_msg) {
            *lost_msg = pB->lost_msg;
            pB->lost_msg = 0;
        }

        EI();
        return (r);				// 1 if no suspension bq msg was already present, 0: ok  if you have waited on msg
    }

    EI();
    return (-1);				// nothing for you my friend
}

void k_round_robbin(void)
{

    // reinsert running task in activeQ if round robbin is selected
    DI();

    prio_enQ(pAQ, deQ(pRun));
    ki_task_shift();

    EI();
}

/* NASTYvoid from vrs 2001 it is main itself can be changed back
dummy_task (void) {  while (1); }
*/

int k_init(int nrTask, int nrSem, int nrMsg)
{
    if (k_running) {			// are you a fool ???
        return 0;
    }

    k_task = nrTask + 1;		// +1 due to dummy
    k_sem = nrSem + nrMsg + 1;	// due to that every msgQ has a builtin semaphore
    k_msg = nrMsg + 1;			// to align so first user msgQ has index 1
    nr_send++;					// to align so we waste one but ... better equal access
    task_pool = (struct k_t *)malloc(k_task * sizeof(struct k_t));
    sem_pool = (struct k_t *)malloc(k_sem * sizeof(struct k_t));
    send_pool = (struct k_msg_t *)malloc(k_msg * sizeof(struct k_msg_t));

    // we dont accept any errors
    if ((task_pool == NULL) || (sem_pool == NULL) || (send_pool == NULL)) {
        k_err_cnt++;
        goto leave;
    }
    // init AQ as empty double chained list
    pAQ = &AQ;
    pAQ->next = pAQ->pred = pAQ;
    pAQ->prio = QHD_PRIO;

    // crt dummy
    // JDN pDmy = k_crt_task (dummy_task, DMY_PRIO, dmy_stk, DMY_STK_SZ);
    pmain_el = task_pool;
    pmain_el->nr = 0;
    nr_task++;
    pmain_el->prio = DMY_PRIO;	// main is dummy
    prio_enQ(pAQ, pmain_el);

    pSleepSem = k_crt_sem(0, 2000);

leave:
    return k_err_cnt;
}

int k_start(int tm)
{
    /*
       48,88,168,328, 1280,2560
       timer 0 and 2 has same prescaler config:

       0 0 0 No clock source (Timer/Counter stopped).
       0 0 1 clk T2S /(No prescaling)
       0 1 0 clk T2S /8 (From prescaler)      2000000 intr/sec at 1 downcount
       0 1 1 clk T2S /32 (From prescaler)      500000 intr/sec ...
       1 0 0 clk T2S /64 (From prescaler)      250000
       1 0 1 clk T2S /128 (From prescaler)     125000
       1 1 0 clk T 2 S /256 (From prescaler)    62500
       1 1 1 clk T 2 S /1024 (From prescaler)   15625  eq 15.625 count down for 1 millisec so 255 counts ~= 80.32 milli sec timer

       timer 1(328+megas), 3,4,5(megas only)
       1280, 2560,2561 has same prescaler config :
       FOR 16 bits !
       prescaler in cs2 cs1 cs0
       0   0   0   none
       0   0   1   /1 == none
       0   1   0   /8     2000000 intr/sec
       0   1   1   /64     250000 intr/sec
       1   0   0   /256     62500 intr/sec
       1   0   1   /1024    15625 intr/sec
       16MHz Arduino -> 16000000/1024 =  15625 intr/second at one count
       16MHz Arduino -> 16000000/256  =  62500 ticks/second
       -------------------------/64   = 250000 ticks/second !

       NB 16 bit counter so values >= 65535 is not working
       **************************************************************************************

     */

    // will not start if errors during initialization
    if (k_err_cnt) {
        return -k_err_cnt;
    }
    // boundary check
    if (tm <= 0) {
        return -555;
    } else if (10 >= tm) {
        fakecnt = fakecnt_preset = 0;	// on duty for every interrupt
    } else if ((tm <= 10000) && (10 * (tm / 10) == tm)) {	// 20,30,40,50,...,10000
        fakecnt_preset = fakecnt = tm / 10;
        tm = 10;
    } else {
        return -666;
    }

    DI();						// silencio
    k_tick_size = tm;

//  outdated ? JDN NASTY
#if defined(__AVR_ATmega32U4__)
    // 32u4 have no intern/extern clock source register
#else
    // should be default ASSR &= ~(1 << AS2);   // Select clock source: internal I/O clock 32u4 does not have this facility
#endif

    TCCRxA = 0;
    TCCRxB = PRESCALE;			// atm328s  2560,...

    if (F_CPU == 16000000L) {
        tcntValue = COUNTMAX - tm * DIVV;

    } else {
        tcntValue = COUNTMAX - tm * DIVV8;	// 8 Mhz wwe assume
    }

    tcntValue += 2;				// add magic water :-) dep on your xtal

    TCNTx = tcntValue;

    //  let us start the show
    TIMSKx |= (1 << TOIEx);		// enable interrupt

    pRun = pmain_el;			// just for ki_task_shift
    k_running = 1;

    DI();
    ki_task_shift();			// bye bye from here
    EI();

    // this while loop bq main are dummy
    while (!stopp) ;

    return (pmain_el->cnt1);	// haps from pocket from kstop
}

int k_stop(int exitVal)
{
// DANGEROUS - handle with care - no isr timer control etc etc
// I WILL NEVER USE IT
    DI();						// silencio
    if (!k_running) {
        EI();
        return -1;
    }

    pmain_el->cnt1 = exitVal;	// transfer in pocket
    //NASTY
    // stop tick timer isr
    TIMSKx &= ~(1 << TOIEx);

    stopp = 1;
    // back to main
    AQ.next = pmain_el;			// we will be the next BRUTAL WAY TO DO IT NASTY
    ki_task_shift();
    while (1) ;					// you will never come here
}
 

unsigned long k_millis(void)
{
    unsigned long l;

    DI();
    l = k_millis_counter;
    EI();
    return l;
}

int k_tmrInfo(void)
{
    return (KRNLTMR);
}

char k_set_preempt(char on)
{
    if (on == 0 || on == 1) {
        krnl_preempt_flag = on;
    }
    return krnl_preempt_flag;
}

char k_get_preempt(void)
{
    return krnl_preempt_flag;
}

#ifdef KRNLBUG

// defined as weak so compiler will take yours instead of mine
void __attribute__ ((weak)) k_breakout(void)
{
}

void __attribute__ ((weak)) k_sem_clip(unsigned char nr, int nrClip)
{
}

void __attribute__ ((weak)) k_send_Q_clip(unsigned char nr, int nrClip)
{
}
#endif

/* EOF - JDN */

/*
#ifdef __cplusplus
}
#endif
*/
