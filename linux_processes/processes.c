#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main (void){
    
    unsigned int num_of_processes = 6;
    
    pid_t array_of_processes[num_of_processes];
    
    for (int i = 0; i < num_of_processes; i++){
        array_of_processes[i] = fork();
    }

    for (int j = 0; j < num_of_processes; j++){
        if (array_of_processes[j] == 0){
            printf("This is the parent process with id %d\n", (int) getpid());
        } else{
            printf("This is the child process with id %d\n", (int) getpid());
        }
    }
    
    return 0;
}