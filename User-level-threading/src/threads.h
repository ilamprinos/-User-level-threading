#ifndef THREADS_H
#define THREADS_H
// #include<ucontext.h>
#include<pthread.h>
#include<unistd.h>
#include <stdlib.h>
#include "deque.h"
#include<string.h>
#include<ucontext.h>

#include "atomic.h"

#include "sem.h"

#ifdef USEDEBUG
#define Debug(f_,...) printf((f_),##__VA_ARGS__);\
fflush(stdout)
#else
#define Debug(...) (void)0
#endif 


// #define USE_LOCAL_QUEUES
// #define RECYCLE_THREADS
// #define METRICS


#ifdef METRICS
#include <time.h>
static clock_t timer;
static atomic_t threads_created;
static atomic_t stacks_recycled;
// static atomic_t mallocs;
// static atomic_t frees;
#endif



#define SIGSTKSZ  16384*2
#define ALIGNMENT 16384*2

#define STACK_MULTIPLICATOR 8

enum state{RUNNING,EXIT,YIELDED};

typedef struct stack_strct{
    struct  stack_strct *next,*prev;
    char* stack;    
    int id;
}stack_strct;

typedef struct thread_desc {
    struct thread_desc *next,*prev;
    atomic_t id; //my id
    atomic_t deps;
    int num_successors;
    
    enum state st;
    char *stack;
    ucontext_t my_context; /* see man getcontext */
    ucontext_t* scheduler_context;
    struct thread_desc* callee;
    struct thread_desc **successors;
    
}thread_t;

// typedef struct thread_desc thread_t;

typedef struct native_thread{
    ucontext_t my_context;
    pthread_t pid;
	char* stack;
    size_t first_native;
    t_deque* local_queue;

}native_th;




native_th* native_list;
int native_list_size;
static ucontext_t uctx_main;
thread_t* main_thread;

t_deque* ready_queue;
t_deque* free_list;
spin_t sched_lock;
atomic_t ids;
volatile int rr_counter;

spin_t thr_exit_lock;

int mask_size;
//This variable holds the main thread's shifted stack 
static unsigned long first_thread_stack_start= -1;


//metric variables



#define THREAD_LIST_NONE	(thread_t *[]){ NULL }
#define THREAD_LIST(...)	(thread_t *[]){ __VA_ARGS__, NULL }


int context_create_th(thread_t* desc,void (wrapper)(void*),void (func)(void*),void*arg,int stack_size);
int simple_context_swap(ucontext_t* from, ucontext_t* to);


/*
Check thread state (RUNNING,YIELDED,EXITED)

@param desc, thread descriptor to check
@param local_queue, native thread's local queue (Scheduler is responsible to move a thread to its local queue) 

*/
void check_thread(thread_t* desc,t_deque* local_queue);


// void copy_thread_t(thread_t* from, thread_t* to);

/*
    Scheduler running on every native thread
*/

void scheduler();

/*
Wrapper is responsible to run the user-level thread and call thread exit at the end 
*/

void wrapper(void*(*body)(void*),void*arg);


int copy_struct_to_stack(thread_t* strct,char* stack);




/*
Environment initialization
@param native_threads number of threads to create

*/
int thread_lib_init(int native_threads);


/*
Return thread's id
*/

thread_t* thread_self();

/*
Return thread's id as an integer

*/
int thread_getid();


/*
Create a thread
@param thread_descriptor, The thread new thread's handle
@param body, function which will be executed by thread
@param arg, arguments of the function
@param deps, number of dependency threads
@param successors threads that depend on this one
*/

int thread_create(thread_t *thread_descriptor ,void(body)(void*),void *arg,int deps,thread_t **successors );

/*
Voluntarily give up the processor

*/
int thread_yield();

/*
Increase number of dependencies to this thread

@param num_deps number of dependencies
*/
int thread_inc_dependency(int num_deps);

/*
Terminate thread
*/
void thread_exit();

/*
Terminate environment
*/
int thread_lib_exit();

#endif
