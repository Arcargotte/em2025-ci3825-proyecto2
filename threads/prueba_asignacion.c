#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <math.h>


int main(void){
    
    int threads = 30;
    int drones = 30;
    int array_of_drones_for_threads[threads];

    float drone_per_thread = (float)drones/threads;

    printf("Division: %f\n", drone_per_thread);

    int drone_int = (int) drone_per_thread;
    
    printf("Parte entera: %d\n", drone_int);

    for(int i = 0; i < threads; i++){
        array_of_drones_for_threads[i] = drone_int;
    }

    // I just need the decimal part
    drone_per_thread = drone_per_thread - drone_int;

    printf("Parte decimal: %f\n", drone_per_thread);

    // I just need the decimal part
    int dif = roundf(drone_per_thread * threads);

    printf("Resto: %d\n", dif);
    
    int i = 0;
    while(dif > 0){
        array_of_drones_for_threads[i]++;
        dif--;
        i++; 
    }

    printf("[");
    for(int i = 0; i < threads; i++){
        if(i != threads - 1){
            printf("%d, ", array_of_drones_for_threads[i]);
        } else{
            printf("%d", array_of_drones_for_threads[i]);
        }
        
    }
    printf("]\n");


    return 0;
}