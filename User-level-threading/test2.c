
#include<stdio.h>

#include<stdlib.h>
#include<unistd.h>

int main(int argc,char** argv){


for(int i=0;i<100;i++){
    //    system("cd build && gdb test_b -ex run -ex quit");
           system("cd build && ./test_b ");
        //    printf("next\n");
        //    fflush(stdout);

        //    usleep(5000);
}
for(int i=0;i<100;i++){
    //    system("cd build && gdb test_b -ex run -ex quit");
           system("cd build && ./test_a ");
        //    printf("next\n");
        //    fflush(stdout);

        //    usleep(5000);
}
for(int i=0;i<100;i++){
    //    system("cd build && gdb test_b -ex run -ex quit");
           system("cd build && ./test_c ");
        //    printf("next\n");
        //    fflush(stdout);

        //    usleep(5000);

    //    >res.txt 2>&1
}
for(int i=0;i<100;i++){
    //    system("cd build && gdb test_b -ex run -ex quit");
           system("cd build && ./test_d ");
        //    printf("next\n");
        //    fflush(stdout);

        //    usleep(5000);

    //    >res.txt 2>&1
}

return 0;

}