#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdbool.h>

long long n, m, num_of_processes, num_of_drones, num_of_targets, work_if_matrix = 0;

typedef struct drone{
    long long x;
    long long y;
    long long radius;
    long long damage;
    long long id;
} drone;

typedef struct target{
    long long x;
    long long y;
    long long health;
    long long resistance;
    long long id;
    bool destroyed;
    int type;
} target;

drone * array_of_drones;
target * array_of_targets;
target *** land;
pthread_mutex_t * available;

/**
 * @brief Used to receive the input with a specific format and to initialize all global variables used in the code.
 * 
 * @param file_name name or path to the file in format .txt used as input.
 * @return bool, true in case everything goes well, otherwise false.
 */
bool parse_input(char * file_name){
    
    FILE *txt_file;
    txt_file = fopen(file_name, "r");
    if (txt_file == NULL) {
        perror("Error: Couldn't open text file!\n");
        exit(EXIT_FAILURE);
    }
    
    // Receives rows and colums
    if ( fscanf(txt_file, "%lld %lld", &n, &m) != 2 ) {
        perror("Error: Wrong format!\n");
        exit(EXIT_FAILURE);
    }

    // Assign memory for matrix land
    land = (target ***)malloc(n * sizeof(target **));
    if (land == NULL) {
        perror("Error assigning memory!\n");
        exit(EXIT_FAILURE);
    }

    for (long long i = 0; i < n; i++) {
        // Assign memory to row
        land[i] = (target **)malloc(m * sizeof(target *)); 
        if (land[i] == NULL) {
            perror("Error assigning memory!\n");
            exit(EXIT_FAILURE);
        }

        for (long long j = 0; j < m; j++) {
            land[i][j] = NULL;  // Initialize pointers in NULL
        }
    }

    // Receives number of targets

    if ( fscanf(txt_file, "%lld", &num_of_targets) != 1 ) {
        perror("Error: Wrong format!\n");
        exit(EXIT_FAILURE);
    }

    array_of_targets = (target *) mmap(NULL, num_of_targets * sizeof(target),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    if (array_of_targets == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    for(long long i = 1; i <= num_of_targets; i++){
        long long coord_x, coord_y, resistance;

        // Receives attributes for each target
        if (fscanf(txt_file, "%lld %lld %lld", &coord_x, &coord_y, &resistance) != 3) {
            perror("Error: Wrong format!\n");
            exit(EXIT_FAILURE);
        }

        array_of_targets[i - 1].x = coord_x;
        array_of_targets[i - 1].y = coord_y;
        array_of_targets[i - 1].health = resistance;
        array_of_targets[i - 1].resistance = resistance;
        array_of_targets[i - 1].id = i;
        array_of_targets[i - 1].type = (resistance < 0) ? 0 : 1; 
        array_of_targets[i - 1].destroyed = false;

        // land is designed to point to targets in array of targets
        land[coord_x][coord_y] = &array_of_targets[i - 1];
    }

    // Receives number of drones
    if ( fscanf(txt_file, "%lld", &num_of_drones) != 1 ) {
        perror("Error: Wrong format!\n");
        exit(EXIT_FAILURE);
    }

    array_of_drones = (drone *) malloc (num_of_drones * sizeof(drone));

    for(long long i = 1; i <= num_of_drones; i++){
        long long coord_x, coord_y, radius, power;

        if ( fscanf(txt_file, "%lld %lld %lld %lld", &coord_x, &coord_y, 
            &radius, &power) != 4 ) {
            perror("Error: Wrong format!\n");
            exit(EXIT_FAILURE);
        }

        // This is a variable used to determine if it's convenient to use the matrix strategy
        work_if_matrix += (2*radius + 1)*(2*radius + 1);

        array_of_drones[i-1].x = coord_x;
        array_of_drones[i-1].y = coord_y;
        array_of_drones[i-1].radius = radius;
        array_of_drones[i-1].damage = power;
        array_of_drones[i-1].id = i;
    }

    fclose(txt_file);
    return true;
}

/**
 * @brief Gives a balanced amount of drones to each thread. 
 * Example: Suppose you have 5 drones and 3 threads, then
 * thread 1 -> 2 drones
 * thread 2 -> 2 drones
 * thread 3 -> 1 drone
 * 
 * @param array_of_drones_for_threads array empty to be filled.
 * @return void, it doesn't return; but fills the array of drones per thread with a balanced amount of drones.
 */
void calculate_drone_per_thread( long long * array_of_drones_for_threads ){

    float drone_per_thread = (float)num_of_drones/num_of_processes;
    long long drone_int = (long long) drone_per_thread, dif, i;

    memset(array_of_drones_for_threads, 0, num_of_processes * sizeof(long long));

    for(long long i = 0; i < num_of_processes; i++){
        array_of_drones_for_threads[i] = drone_int;
    }

    drone_per_thread = drone_per_thread - drone_int;

    dif = (long long) round((double) drone_per_thread * num_of_processes);

    i = 0;
    while(dif > 0){
        array_of_drones_for_threads[i]++;
        dif--;
        i++; 
    }

}

/**
 * @brief Computes how much damage a drone makes over a given target
 * 
 * @param drone Drone that could or not attack the target
 * @param target Target attacked or not by the drone
 * @return void
 */
void computes_damage (drone drone, target * target){

    bool hits = false;

    // First Quadrant
    if( drone.x >= target->x && drone.y >= target->y && 
        drone.x - drone.radius <= target->x && 
        drone.y - drone.radius <= target->y ){
        
        hits = true;
        
    }
    //Second Quadrant
    else if( drone.x < target->x && drone.y > target->y && 
             drone.x + drone.radius >= target->x && 
             drone.y - drone.radius <= target->y ){

        hits = true;
        
    }
    //Third Quadrant
    else if( drone.x > target->x && drone.y < target->y && 
             drone.x - drone.radius <= target->x && 
             drone.y + drone.radius >= target->y ){
        
        hits = true;
        
    }
    //Fourth Quadrant
    else if( drone.x <= target->x && drone.y <= target->y && 
             drone.x + drone.radius >= target->x && 
             drone.y + drone.radius >= target->y ){
        
        hits = true;
        
    }

    if ( hits ){
        pthread_mutex_lock(available);
        if(target->type == 0){

            target->health = target->health + drone.damage;

            if( target->health >= 0 ){
                target->destroyed = true;
            }

        } else{

            target->health = target->health - drone.damage;

            if( target->health <= 0 ){
                target->destroyed = true;
            }

        }
        pthread_mutex_unlock(available);
    }
}

/**
 * @brief Computes how much targets a drone reaches in the matrix
 * 
 * @param drone Drone that explodes in a given position and reaches or not some targets
 * @return void.
 */
void computes_damage_in_matrix(drone drone){

    long long x0 = drone.x - drone.radius, y0 = drone.y - drone.radius, i = x0, j;
    
    while(i < x0 + (2*drone.radius + 1)){

        j = y0;

        if( i >= 0 && i < n ){

            while(j < y0 + (2*drone.radius + 1)){

                if( j >= 0 && j < m ){

                    if( land != NULL && land[i][j] != NULL && land[i][j]->id > 0 && 
                        land[i][j]->destroyed == false && 
                        land[i][j]->type == 0 ){
                        
                        // Blocking others threads to access to the critical section
                        pthread_mutex_lock(available);
                        land[i][j]->health += drone.damage; 
                        
                        if( land[i][j]->health >= 0 ){
                            land[i][j]->destroyed = true;
                        }
                        pthread_mutex_unlock(available);
                        // Unblocking others threads to access to the critical section

                    } else if( land != NULL && land[i][j] != NULL && land[i][j]->id > 0 && 
                               land[i][j]->destroyed == false && 
                               land[i][j]->type == 1 ){
                        
                        // Blocking others threads to access to the critical section
                        pthread_mutex_lock(available);
                        land[i][j]->health -= drone.damage;
                        
                        if( land[i][j]->health <= 0 ){
                            land[i][j]->destroyed = true;
                        }
                        pthread_mutex_unlock(available);
                        // Unblocking others threads to access to the critical section

                    }
                    
                }
                j++;
            }
        }
        i++;
    }

}

/**
 * @brief Decides strategy used to process drones. 
 * 
 * @return int: If it's more convenient to work with no matrix, then it will return 1. If it's more convenient to work with the matrix, then it will return 2.
 */
int strategy_decider(){

    long long work_if_no_matrix = (long long)(num_of_drones * num_of_targets);
    return (work_if_no_matrix <= work_if_matrix) ? 1 : 2;

}

/**
 * @brief Used to print ouput required.
 * 
 * @return void, it just prints the output in a specific formats.
 */
void print_output(){

    long long om_destroyed_targets = 0, om_parcially_destroyed_targets = 0, 
        ic_destroyed_targets = 0, ic_parcially_destroyed_targets = 0, 
        om_intact_targets = 0, ic_intact_targets = 0;
    
    for (long long i = 0; i < num_of_targets; i++){
        if( array_of_targets[i].type == 0 && !array_of_targets[i].destroyed && 
            array_of_targets[i].resistance == array_of_targets[i].health ){

            om_intact_targets++;

        } else if( array_of_targets[i].type == 0 && !array_of_targets[i].destroyed && 
                   array_of_targets[i].resistance != array_of_targets[i].health ){
            
            om_parcially_destroyed_targets++;
            
        } else if( array_of_targets[i].type == 0 && array_of_targets[i].destroyed ){

            om_destroyed_targets++;

        } else if( array_of_targets[i].type == 1 && !array_of_targets[i].destroyed && 
                   array_of_targets[i].resistance == array_of_targets[i].health ){
            
            ic_intact_targets++;

        } else if( array_of_targets[i].type == 1 && !array_of_targets[i].destroyed && 
                   array_of_targets[i].resistance != array_of_targets[i].health ){

            ic_parcially_destroyed_targets++;

        } else if( array_of_targets[i].type == 1 && array_of_targets[i].destroyed ){

            ic_destroyed_targets++;

        }
    }

    printf("OM sin destruir: %lld\n", om_intact_targets);
    printf("OM parcialmente destruidos: %lld\n", om_parcially_destroyed_targets);
    printf("OM totalmente destruido: %lld\n", om_destroyed_targets);

    printf("IC sin destruir: %lld\n", ic_intact_targets);
    printf("IC parcialmente destruidos: %lld\n", ic_parcially_destroyed_targets);
    printf("IC totalmente destruido: %lld\n", ic_destroyed_targets);
}

long long min(long long a, long long b){
    return (a < b) ? a : b;
}

int main (int argc, char *argv[]){
    
    long long process_iter;

    if(argc != 3){
        printf("Error: You should send exactly 2 arguments!\n");
        return 1;
    }

    num_of_processes = atoll(argv[1]);
    long long array_of_drones_per_process[num_of_processes];

    if(!parse_input(argv[2])){
        return 1;
    }

    // If num_of_processes exceeds the limit, then we limit it
    long long maximum_processes = min(n * m, num_of_drones);
    if(num_of_processes > maximum_processes){
        num_of_processes = maximum_processes;
    }

    available = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (available == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    if (pthread_mutex_init(available, &attr) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }

    calculate_drone_per_thread(array_of_drones_per_process);
    
    pid_t id_process = 1;

    process_iter = -1;
    
    for (long long k = 0; k < num_of_processes; k++){
        process_iter++;
        id_process = fork();

        if ((long long)id_process < 0){
            perror("Error forking process\n");
            return 1;
        }

        if (id_process == 0) {
            
            int strategy = strategy_decider();
            long long initial_drone = 0;

            switch(strategy){
                case 1:
    
                    for(long long k = 0; k < process_iter; k++){
                        initial_drone += array_of_drones_per_process[k];
                    }

                    // Iterates from initial drone until it reaches the amount of drones given
                    for(long long i = initial_drone; 
                        i < initial_drone + array_of_drones_per_process[process_iter]; 
                        i++){
                        for (long long j = 0; j < num_of_targets; j++){
                            if(!array_of_targets[j].destroyed){
                                computes_damage( array_of_drones[i], 
                                                 &array_of_targets[j] );
                            }
                        }
                    }

                    break;
                    
                case 2:

                    for(long long k = 0; k < process_iter; k++){
                        initial_drone += array_of_drones_per_process[k];
                    }

                    // Iterates from initial drone until it reaches the amount of drones given
                    for(long long i = initial_drone; 
                        i < initial_drone + array_of_drones_per_process[process_iter]; 
                        i++){
                        computes_damage_in_matrix(array_of_drones[i]);
                    }

                    break;
            }

            // Free Section
            if (land != NULL) {
                for(long long i = 0; i < n; i++){
                    free(land[i]);
                }
            }
            free(land);
            free(array_of_drones);
            fflush(stdout);
            exit(0); 
        }
    }

    // Parent process waits for childs to finish
    while (wait(NULL) > 0);

    print_output();

    // Free Section

    if (land != NULL) {
        for(long long i = 0; i < n; i++){
            free(land[i]);
        }
    }
    
    free(land);
    
    munmap(array_of_targets, num_of_targets * sizeof(target));
    munmap(available, sizeof(pthread_mutex_t));
    free(array_of_drones);

    return 0;
}