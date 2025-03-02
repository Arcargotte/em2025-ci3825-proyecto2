#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main (void){
    
    unsigned int num_of_processes = 4 ; //Creates 4 child processes.
    
    pid_t array_of_processes[num_of_processes];
    pid_t id_process = 1;

    for (int i = 0; i < num_of_processes; i++){
        if ((int) id_process != 0){
            id_process = fork();
        }else {
            break;
        }

        array_of_processes[i] = id_process;
    }

    if ((int) id_process != 0){
        printf("This is parent process with id\n");
        for (int j = 0; j < num_of_processes; j++){
            printf("Child process %d with id %d\n", j, array_of_processes[j]);
        }
        return 0;
    } else{
        printf("This is child process\n");
        return 0;
    }
}
