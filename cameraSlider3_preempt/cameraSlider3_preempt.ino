
#include <krnl.h>

// A small krnl program with two independent tasks
// High priority task (t1) acts like a realtime controller with
// fixed samplings frequency.
// A low priority task(t2) do non real time actions (just some printing)

struct k_t *pt1, // pointer to hold reference
*pt2,          // to taskdescriptor for t1 and t2  
*samplingsSem;
struct *sem1; // semaphore for external interrupt

char s1[200]; // stak for task t1
char s2[200]; // stak for task t2
char s3[200]; // stak for task t2


void xxx()
{

  // no local vars ?!?  ! I think
  PUSHREGS();
  if (!k_running)
    goto exitt ;

  icnt++;
  ki_signal(sem1); 
  K_CHG_STAK();
exitt:

  POPREGS();
  RETI();
}

void initISR()
{
  
  attachInterrupt(digitalPinToInterrupt(2),xxx,FALLING);
  digitalWrite(2,HIGH);
}

void toggleLED13()
{
  static char flag=0;
  if (flag) {
    flag = 0;
    digitalWrite(13,HIGH);
  }
  else {
    flag = 1;
    digitalWrite(13,LOW);
  }
}

void t1(void)
{

  k_set_sem_timer(samplingsSem,100); // krnl will signal sem every 10 tick

  while (1) {  
    // wait until next kick from timer
    // In this way we will run realtime with a fixed frequencey
    // but beware of tasks with highe priority bq they can spoil your
    // realtime bq you will be ready but not running until they leave the
    // active queue
    
    k_wait(samplingsSem,0); 
    toggleLED13(); // chk frequency with an oscilloscope 

    // AD(); 
    // pid();
    // DA();
    // hope you get the plot :-)
    
  }         
}

void t3(void) {

  while (1) {
//  int i;

    k_wait(sem1,0); // wait forever
    doBlink();
  }

}

void t2(void)
{
  // and task body for task 2
  // runs independent of task t1
  while (1) {
    Serial.println("tik"); // just do something non realtime :-)
    k_sleep(1000);
   }
}

void setup()
{
  Serial.begin(9600);  // for output from task 1
  pinMode(13,OUTPUT);  // for blink on LED from task 2

  // init krnl so you can create 2 tasks, 1 semaphore and no message queues
  k_init(2,1,0); 
  
  sem1 = k_crt_sem(0,10);

  // two task are created
  //                |------------ function used for body code for task
  //                |  |--------- priority (lower number= higher prio
  //                |  | |------- array used for stak for task 
  //                |  | |   |--- staksize for array s1
  pt1=k_crt_task(t1,10,s1,200);  // highest prio
  pt2=k_crt_task(t2,11,s2,200);  // lowest - 10 < 11 as well as 5 < 100 :-)
  pt3=k_crt_task(t3,11,s3,200);  // lowest - 10 < 11 as well as 5 < 100 :-)

  //                         |--- startvalue (can be 0,1,2, not negative 
  //                          |-- max value before maxing out
  samplingsSem = k_crt_sem(0,10);

  // Init Rotary encoder reading
  initISR();

  // NB-1 remember an Arduino has only 2-8 kByte RAM
  // NB-2 remember that stak is used in function calls for
  //  - return address
  //  - registers stakked
  //  - local variabels in a function
  //  So having 200 Bytes of stak excludes a local variable like ...
  //    int arr[400];  
  // krnl call k_unused_stak returns size of unused stak
  // Both task has same priority so krnl will shift between the
  // tasks every 10 milli second (speed set in k_start)

  k_start(10); // start kernel with tick speed 10 milli seconds
}

void loop(){ /* loop will never be called */
}



#include "krnl.h"

// External triggered ISR
// An Interrupt Service Routine is attached to pin 2
// So when pin2 is drived to ground (by a wire) an interrupt is generated.
// The ISR increment a counter and send it to a message Q
// naming ki_send .... "i" indicates it can be used in an ISR and demand interrupt to be disabled prio to call
// and that no task shift takes place in the call
// demonstrates ISR with  message Q and preemption(task shift) in the ISR
// NB Take a look on the ISR. For 1280 and 2560 it is INT4 but for 168,328,.. it's INTO
// It is taken care by a compile flag
// (c) beerware license JDN 2013
// AS USUAL Serial.print is not krnl safe !!! cant break your program

struct k_t * p_t1,*p_t2,*sem1;

#define STK_SIZE 100

char s1[STK_SIZE]; // stak for t1 ... and t2
char s2[STK_SIZE]; // stak for t1 ... and t2
char s3[STK_SIZE]; // stak for t1 ... and t2

volatile int icnt=0;

void doBlink(void) {
  static char flag = 0;
  flag = ! flag;
  digitalWrite(13,flag);
}


void t2()
{
  while (1) {
    // doBlink();
   // k_sleep(100);
  }
}

void t1(void) {

  while (1) {
//  int i;

    k_wait(sem1,0); // wait forever
    doBlink();
  }

}



void setup() {

  Serial.begin(115200);
  pinMode(13,OUTPUT);

  k_init(2,1,0); // from now you can crt task,sem etc

  sem1 = k_crt_sem(0,10); // 

  p_t1 = k_crt_task(t1, 10, s1, STK_SIZE);
  p_t2 = k_crt_task(t2, 10, s2, STK_SIZE);
  p_t3 = k_crt_task(t3, 10, s3, STK_SIZE);

  initISR();

  Serial.println("just before");
  k_start(10); // now we are runnning   with timer 10 msev
  //Serial.println("oev");

  // main will not come back and will sleep rest of life
}

void loop(void) {
  // just for compilation - will never be called
}

/* QED :-) */




