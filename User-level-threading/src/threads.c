#include "threads.h"
#include <stdio.h>
#include <math.h>


int context_create_th(thread_t *desc, void(wrapper)(void *), void(func)(void *), void *arg, int stack_size)
{

    if (getcontext(&(desc->my_context)))
    {
        perror("getcontext failed");
        return -1;
    }
    // INCREMENT STACK TO AVOID TOUCHING THREAD DESCRIPTOR IN THE BEGINNING
    desc->my_context.uc_stack.ss_sp = desc->stack +sizeof(long double);
    
    desc->my_context.uc_stack.ss_size = SIGSTKSZ-sizeof(long double);
    desc->my_context.uc_link = NULL;
    makecontext(&(desc->my_context),(void (*)(void))wrapper, 2,func,arg);

    return 0;
}

int simple_context_swap(ucontext_t *from, ucontext_t *to)
{

    if (swapcontext(from, to))
    {
        perror("cannot swap context");
        return 1;
    }

    return 0;
}

int copy_struct_to_stack(thread_t *strct, char *stack)
{

    ((thread_t **)stack)[0] = strct;
    return 0;

}

// void copy_thread_t(thread_t* from, thread_t* to){


//     if(from == NULL || to ==NULL)
//         return;

//     to->callee =NULL;
//     to->id= from->id;
//     to->deps = from->deps;
//     to->num_successors = from->num_successors;
//     to->st = from->st;
//     to->stack = from->stack;
//     to->my_context= from->my_context;
//     to->scheduler_context=from->scheduler_context;
//     to->successors = from->successors;

//     return;
// }


void wrapper(void *(*body)(void *), void *arg)
{
    

    body(arg);

    thread_exit();
}

void check_thread(thread_t* desc,t_deque* local_queue){

    if(desc==NULL)
        return;

    //USER-LEVEL THREAD FINISHED, RECYCLE ITS STACK
    if(desc->st==EXIT){
        spin_unlock(&thr_exit_lock);

        // stack_strct* temp = (stack_strct*)malloc(sizeof(stack_strct)); 
        stack_strct* temp;
        temp =(stack_strct *) ((stack_strct**)desc->stack)[0];
        temp->id= desc->id;
        temp->stack= desc->stack;
        pushBack(free_list,(t_deque_node*)temp);
        desc->my_context.uc_stack.ss_sp = NULL;
        desc->stack= NULL;

    }else if(desc->st==YIELDED){//THREAD REQUEST YIELD, PUT IN THE READY QUEUE OR LOCAL READY QUEUE
        if (desc->deps == 0)
        {
            desc->st=RUNNING;
#ifdef USE_LOCAL_QUEUES
            pushFront(local_queue,(t_deque_node*)desc);
#else
            pushFront(ready_queue,(t_deque_node*) desc);
#endif
            return;
        }
    }
    //SHOULD NEVER COME TO THIS PART
    if(desc->st==RUNNING){
         Debug("IN CHECK THREAD SHOULD NOT BE HERE\n");
        return;
    }
}

void scheduler(native_th *info)
{       
    //INCREASE NATIVE THREAD STACK SIZE TO ENSURE THAT IT IS ENOUGH
        // pthread_attr_t atr;
        // size_t stksize;
        // pthread_attr_getstacksize(&atr, &stksize);
        // pthread_attr_setstacksize(&atr,STACK_MULTIPLICATOR*stksize);

    //FIRST NATIVE THREAD WILL BEGIN EVERYONE ELSE WHEN USER YIELDS FOR THE FIRST TIME
    if (info->first_native == 0)
    {
        info->pid = pthread_self();
        spin_lock(&sched_lock);
        spin_unlock(&sched_lock);
    }
    else
    {
        spin_unlock(&sched_lock);
    }

    Debug("SCHEDULER CREATED\n");

    thread_t *thrd_to_exec = NULL;

    while (1)
    {
        //CHECK IF YOU RAN A THREAD AND IT HAS AN IMMIDIATE SUCCESSOR (ZERO DEPENDENCIES)
        if(thrd_to_exec!=NULL && thrd_to_exec->callee!=NULL){
            thrd_to_exec= thrd_to_exec->callee;

        }else{
                
#ifdef USE_LOCAL_QUEUES
                thrd_to_exec =(thread_t *)popBack(info->local_queue);
#else
                thrd_to_exec = (thread_t *)popBack(ready_queue);
               
#endif


        }


#ifdef USE_LOCAL_QUEUES
        if(thrd_to_exec == NULL){
            // GO SEARCH FROM OTHER NATIVE THREADS LOCAL QUEUES, (DECREASES CONTENTION ON LOCKS)
            for(int i=0; i<native_list_size; i++){
                thrd_to_exec = popBack(native_list[i].local_queue);
                if(thrd_to_exec != NULL){
                    break;
                }
            }
            
        }
#endif

        if (thrd_to_exec != NULL)
        {
    
            /// MAKE SURE FIRST USER LEVEL THREAD (MAIN() FUNCTION) IS RUN BY FIRST NATIVE THREAD
            if(thrd_to_exec==main_thread && info->pid!=native_list[0].pid){
#ifdef USE_LOCAL_QUEUES

                pushBack(native_list[0].local_queue,(t_deque_node*)thrd_to_exec);
#else
                pushBack(ready_queue,(t_deque_node*)thrd_to_exec);
#endif
                thrd_to_exec= NULL;
                // sleep(3);
                continue;
            }
    
            // NOTIFY THREAD TO SWAP TO THIS SCHEDULER AND SWAP TO ITS WRAPPER
            thrd_to_exec->scheduler_context = &(info->my_context);
            simple_context_swap(&(info->my_context), &(thrd_to_exec->my_context));

            // THREAD IS FINISHED OR YIELDED
            check_thread(thrd_to_exec,info->local_queue);
            // thrd_to_exec= NULL;
        }
        else if (get_queue_size(free_list) == ids - 1)
        {
            // FREE LIST HAS ALL STACKS OF ALL USER LEVEL THREADS (EXCEPT MAIN())
            Debug("No more threads exiting\n");
            //FIRST NATIVE MUST NOT EXIT, BUT RUN MAIN()
            if(info->pid==native_list[0].pid){
                continue;
            }


            break;
        }


    }

    Debug("SCHEDULER exiting\n");
}

thread_t *thread_self()
{
    
    // random variable to fetch its address
    unsigned long temp;
    // p_address, a decimal value of an address in this stack
    // shifted_address, temporary value to find beginning of stack
    unsigned long p_address, stack_start, shifted_address;

    p_address = (unsigned long)&temp;

    shifted_address = p_address >> mask_size;
    stack_start = shifted_address << mask_size;

    //GO TO THE BEGINNING OF THE PAGE AND CHECK 
    if (stack_start == first_thread_stack_start)
    {
        Debug("THREAD SELF FUNCTION, THIS IS MAIN THREAD\n");
        return main_thread;
    }

    // NON MAIN THREADS HAVE THEIR DESCRIPTOR IN THE BEGINNING OF THEIR STACK
    thread_t *desc =(thread_t*) ((thread_t**)((char *)stack_start))[0];

    return desc;
}

int thread_lib_init(int native_threads)
{

#ifdef METRICS
    atomic_set(0, &threads_created);
    atomic_set(0, &stacks_recycled);
#endif

    int temp;

    spin_lock_init(&sched_lock);
    spin_lock_init(&thr_exit_lock);

    atomic_set(0, &ids);
    spin_lock(&sched_lock);
 
    if (!pthread_setconcurrency(native_threads))
    {
        Debug("concurrency set to %d\n",native_threads);
    }
    // try to implement log2 without math library
    mask_size = log2(SIGSTKSZ);
  
    Debug("THREAD_LIB_INIT_ \n");
    // POTENTIALLY THIS SHOULD BE INITIALISED IN THREAD_SELF ONCE, BUT WORKS HERE TOO
    first_thread_stack_start = (unsigned long)(&temp) >> mask_size << mask_size;

#ifndef USE_LOCAL_QUEUES
    ready_queue = dequeInit();
#else
        // rr_counter = 0;
    
#endif
    free_list = dequeInit();
    if (native_threads == 0)
    {
        Debug("Cannot initialize with 0 threads");
        exit;
    }

    
    native_list_size = native_threads;

    native_list = (native_th *)malloc((sizeof(native_th) * (native_threads)));
   
    native_list[0].pid = pthread_self();
    // POSIX MEMALIGN IS USED SO THAT WE CAN FIND THE BEGINNING OF STACKS
    if( posix_memalign((void **)&(native_list[0].stack), ALIGNMENT, SIGSTKSZ))
    {
         
        perror("POSIX MEMALIGN ERROR\n");
        exit;
        
    }


    if (getcontext(&(native_list[0].my_context)))
    {
        perror("getcontext failed");
        return -1;
    }
    //FIRST NATIVE IS CREATED BY THE OPERATING SYSTEM SO WE CREATE A CONTEXT FOR THE SCHEDULER
    native_list[0].my_context.uc_stack.ss_sp = native_list[0].stack;
    native_list[0].my_context.uc_stack.ss_size = SIGSTKSZ;
    native_list[0].my_context.uc_link = NULL;
    native_list[0].first_native = 1;

#ifdef USE_LOCAL_QUEUES
    native_list[0].local_queue = dequeInit();
#endif

    makecontext(&(native_list[0].my_context), (void *)scheduler, 1, &native_list[0]);

//  CREATE MAIN THREAD, (IT COULD BE ON THE STACK)
    main_thread = (thread_t *)malloc(sizeof(thread_t));


    atomic_inc_and_test(&ids);
    atomic_set(ids, &(main_thread->id));

    atomic_set(0, &(main_thread->deps));
    
    main_thread->stack = NULL;
    main_thread->my_context = uctx_main;
    main_thread->scheduler_context = &(native_list[0].my_context);
    main_thread->num_successors = 0;


    for (size_t i = 1; i < native_list_size; i++)
    {
        native_list[i].first_native = 0;
#ifdef USE_LOCAL_QUEUES
        native_list[i].local_queue = dequeInit();
#endif
        //OTHER NATIVE THREADS ARE CREATED NORMALLY
        pthread_create(&(native_list[i].pid), NULL, (void*)scheduler, &native_list[i]);
    }

#ifdef METRICS
    timer = clock();
#endif
    return 0;
}

int thread_getid()
{
    thread_t *desc;
    desc = thread_self();
    return desc->id;
}

int thread_create(thread_t *thread_descriptor, void(body)(void *), void *arg, int deps, thread_t **successors)
{

#ifdef METRICS
    // threads_created++;
     atomic_inc_and_test(&threads_created);
#endif
    
    stack_strct* temp=NULL;

//OPTIMIZATION, RECYCLE STACKS 
#ifdef RECYCLE_THREADS
    temp = popBack(free_list);
#endif
    if (temp!=NULL)
    {
#ifdef METRICS
        // stacks_recycled++;
        atomic_inc_and_test(&stacks_recycled);
#endif
       
        // thread_descriptor->id = temp->id;
        thread_descriptor->stack = temp->stack;
        atomic_set((int)temp->id, &(thread_descriptor->id));

        // free(temp);

        

    }else
    {
        atomic_inc_and_test(&ids);
        atomic_set((int)ids, &(thread_descriptor->id));
      
        if (posix_memalign((void **)&(thread_descriptor->stack), ALIGNMENT, SIGSTKSZ))
        {
            perror("POSIX MEMALIGN ERROR\n");
            exit;
        }        
    }

    atomic_set(deps, &(thread_descriptor->deps));
    copy_struct_to_stack(thread_descriptor, thread_descriptor->stack);
    // thread_descriptor->deps = deps;
    thread_descriptor->successors = successors;
    thread_descriptor->num_successors = 0;
    thread_descriptor->callee = NULL;
    thread_descriptor->st =RUNNING;

    if (successors != NULL)
        for (int i = 0; successors[i] != NULL; i++)
        {
            thread_descriptor->num_successors++;
        }

    // CREATE CONTEXTS HERE, WRAPPER IS RESPONSIBLE TO RUN USER-LEVEL THREADS

    // context_create_th(thread_descriptor,(void (*)(void *)) wrapper, body, arg, (SIGSTKSZ -(sizeof(long double))));

     if (getcontext(&(thread_descriptor->my_context)))
    {
        perror("getcontext failed");
        return -1;
    }
    // INCREMENT STACK TO AVOID TOUCHING THREAD DESCRIPTOR IN THE BEGINNING
    thread_descriptor->my_context.uc_stack.ss_sp = thread_descriptor->stack +sizeof(long double);    
    thread_descriptor->my_context.uc_stack.ss_size = SIGSTKSZ-sizeof(long double);
    thread_descriptor->my_context.uc_link = NULL;
    makecontext(&(thread_descriptor->my_context),(void (*)(void))wrapper, 2,body,arg);



  
    if (!deps)
    {
        // THREADS WITH NO DEPENDENCIES ARE PLACED TO THE FIRST NATIVE THREAD local_queue, OPTIMIZATIONS COULD BE ADDED (ROUND ROBIN etc.)
#ifdef USE_LOCAL_QUEUES
        // spin_lock(&other_lock);
        // pushBack(native_list[rr_counter++].local_queue,thread_descriptor);
        // if(rr_counter == native_list_size){
        //     rr_counter = 0;
        // }
        // spin_unlock(&other_lock);
        

        pushBack(native_list[0].local_queue,(t_deque_node*)thread_descriptor);
#else
        pushBack(ready_queue, (t_deque_node*)thread_descriptor);
#endif

    }

    return 0;
}

int thread_yield()
{
    thread_t *desc;
    desc = thread_self();
    
    desc->st = YIELDED;
    simple_context_swap(&(desc->my_context), desc->scheduler_context);
    desc->st = RUNNING;

    return 0;
}

int thread_inc_dependency(int num_deps)
{
    thread_t *desc;

    desc = thread_self();
  
    atomic_add(num_deps, &(desc->deps));
    return 0;
}

void thread_exit()
{
    thread_t *desc;
    thread_t *pos;
    thread_t* next_thread = NULL;

    desc = thread_self();

    if (desc == main_thread)
    {
        Debug("MAIN thread exiting \n");
        return;
    }

// DECREASE DEPENDENCIES TO YOUR SUCCESSORS


    spin_lock(&thr_exit_lock);

    for (int i = 0; i < desc->num_successors; i++)
    {

        pos = (desc->successors[i]);
        if (pos == NULL)
        {
            continue;
        }
        atomic_sub_and_test(1, &(pos->deps));
        
        if (pos->deps == 0)
        {
            //OPTIMIZATION, IF SUCCESSOR IS READY TO RUN(0 DEPS), TAKE IT IMMEDIATELY INSTEAD OF PUTTING IT IN THE QUEUE
#ifdef HAND_TO_HAND
            if(next_thread!=NULL){
#endif

#ifdef USE_LOCAL_QUEUES
                pushBack(native_list[0].local_queue,(t_deque_node*)pos);
#else
                pushBack(ready_queue,(t_deque_node*) pos);
#endif

#ifdef HAND_TO_HAND
            }else{
                next_thread = pos;
            }   
#endif   
        }
    }

    desc->st = EXIT;
    if(next_thread !=NULL){
        //SCHEDULER WILL SWAP TO next_thread, on the next loop
        desc->callee= next_thread;
    }else{
        desc->callee = NULL;
    }
    simple_context_swap(&(desc->my_context), desc->scheduler_context);

    return;
}

int thread_lib_exit()
{
    Debug("THREAD LIB EXIT\n");
  
   
 
    for (size_t i = 0; i < native_list_size; i++)
    {
        Debug("waiting thread\n");
        pthread_join(native_list[i].pid, NULL);
        Debug("thread exited\n");


    }

#ifdef USE_LOCAL_QUEUES
    for (size_t i = 0; i < native_list_size; i++)
    {
        free(native_list[i].local_queue);
    }

#endif

#ifdef METRICS
    printf("time passed is %f\n",((double)(clock()-timer))/CLOCKS_PER_SEC);

    printf("used IDS %d threads created %d\n",ids,threads_created);
    

    printf("Stacks _recycled %d\n",stacks_recycled);
    fflush(stdout);
#endif

    free(native_list[0].stack);
    free(native_list);


    stack_strct *desc;
    desc = (stack_strct *)popBack(free_list);

    while (desc!=NULL)
    {
        
        free(desc->stack);
   
        // free(desc);
        desc = (stack_strct *)popBack(free_list);
    }
#ifndef USE_LOCAL_QUEUES
    free(ready_queue);
#endif
    free(free_list);
         

    free(main_thread);


    return 0;
}
