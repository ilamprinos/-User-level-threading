
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
// #include "../src/threads.h"
#include "threads.h"
// #include "../src/types.h"






void func(){

    //for alignment do SIGSTKSZ-1 and a NOT gate, or make the (2^y) y last bits 0, also ALIGNMENT = SIGSTKSZ 
    // void* pointer= !(SIGSTKSZ-1);
    
    // int temp;


    struct rlimit lim;
    // char ee[16384];
    // printf("INSIDE FUNC\n");
    getrlimit(RLIMIT_STACK, &lim);
    // printf("CURR %d\n",lim.rlim_cur);
    // printf("MAX %d\n",lim.rlim_max);
    thread_t* temp_desc;
    temp_desc= thread_self();
    printf("IN FUNC %d\n",temp_desc->id);
    printf("LEAVING FUNC\n");
    thread_exit();

    
    // printf("KALISPERA APO FUNC %d\n",SIGSTKSZ);
    // printf("%p\n",&temp);

    
    // unsigned long p,p_shifted;
    // p= &temp;

    // p_shifted = p>>12;
    // p = p_shifted<<12;
    
    // thread_t* ptr = *(unsigned long*)p; 

    // printf("POINTER %ld\n",p);
    // printf("ACTUAL POINTER %p",ptr);
    // printf("ID %d\n",ptr->id);

}


int main(int argc,char** argv){

    // thread_li
    // thread_t e;
    // thread_t
    // thread_t k;
    thread_t* tr;
    struct rlimit lim;
    getrlimit(RLIMIT_STACK, &lim);
    printf("CURR %ld\n",lim.rlim_cur);
    printf("MAX %ld\n",lim.rlim_max);
    printf("MAIN\n");

    thread_lib_init(2);

    // thread_create(tr,(void*)func,NULL,0,NULL);
    thread_t** tpt=THREAD_LIST(thread_self());
    printf("eew %d \n",tpt[0]->id);


  


    thread_create(&tr, func, NULL , 0, THREAD_LIST(thread_self()));
    thread_inc_dependency(1);
    // thread_self();
    thread_yield();
    // printf("MAIN EXITING\n");
    thread_lib_exit();
    return 0;
}
