#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Global variables
int n;
int m;
long long num_of_drones;    
long long num_of_targets;
int num_of_threads;
int work_if_matrix = 0;
pthread_mutex_t available;

typedef struct drone drone;
struct drone{
    int x;
    int y;
    int radius;
    int damage;
    int id;
};

typedef struct target target;
struct target{
    int x;
    int y;
    int health;
    int resistance;
    int id;
    bool destroyed;
    int type;
};

typedef struct thread_args_drone{
    drone * array_of_drones;
    target * array_of_targets;
    int num_of_drones;
} thread_args_drone;

// Global arrays
drone * array_of_drones;
target * array_of_targets;
target *** land = NULL;

/* Esta linea está mal, es importante terminar de trabajar aqui. */
thread_args_drone * arr_of_args_drone[90000];

/**
 * @brief Computes how much damage a drone makes over a given target
 * 
 * @param drone Drone that could or not attack the target
 * @param target Target attacked or not by the drone
 * @return int that represent how much damage the drone makes over the given target
 */
int computes_damage (drone drone, target * target){

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
        return (target->type == 0) ? drone.damage : -1 * drone.damage;
    }

    return 0;
}

/**
 * @brief Computes how much targets a drone reaches in the matrix
 * 
 * @param drone Drone that explodes in a given position and reaches or not some targets
 * @return void.
 */
void computes_damage_in_matrix(drone drone){

    int x0 = drone.x - drone.radius, y0 = drone.y - drone.radius, i = x0, j;
    
    while(i < x0 + (2*drone.radius + 1)){

        j = y0;

        if( i >= 0 && i < n ){

            while(j < y0 + (2*drone.radius + 1)){

                if( j >= 0 && j < m ){

                    if( land[i][j] != NULL && land[i][j]->id > 0 && 
                        land[i][j]->destroyed == false && 
                        land[i][j]->type == 0 ){
                        
                        // Blocking others threads to access to the critical section
                        pthread_mutex_lock(&available);
                        land[i][j]->health += drone.damage; 
                        
                        if( land[i][j]->health >= 0 ){
                            land[i][j]->destroyed = true;
                        }
                        pthread_mutex_unlock(&available);
                        // Unblocking others threads to access to the critical section

                    } else if( land[i][j] != NULL && land[i][j]->id > 0 && 
                               land[i][j]->destroyed == false && 
                               land[i][j]->type == 1 ){
                        
                        // Blocking others threads to access to the critical section
                        pthread_mutex_lock(&available);
                        land[i][j]->health -= drone.damage;
                        
                        if( land[i][j]->health <= 0 ){
                            land[i][j]->destroyed = true;
                        }
                        pthread_mutex_unlock(&available);
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
 * @brief Iterates over the drones calculating the targets that the drone reaches.
 * 
 * @param args Struct that contains all the arguments for the function to work.
 * @return void *, this time is simply NULL.
 */
void * drone_damage_targets_matrix (void * args){
    
    thread_args_drone * arguments = (thread_args_drone * ) args;
    
    for(int i = 0; i < arguments->num_of_drones; i++){
        computes_damage_in_matrix(arguments->array_of_drones[i]);
    }

    pthread_exit(0);
    return NULL;
}

/**
 * @brief Iterates over the drones and calculates for each objective the damage made to it,
 * it saves the damage made in an local array and once it finishes calculating updates the health of each objective,
 * it uses a mutex to be sure of not getting involve in some race condition.
 * 
 * @param args Struct that contains all the arguments for the function to work.
 * @return void *, this time is simply NULL.
 */
void * drone_damage_targets (void * args){
    
    thread_args_drone * arguments = (thread_args_drone * ) args;

    // This is an array to save how much damage this drone has done to each target
    int damage_control_array[num_of_targets];
    
    // Initialize array with ceros
    for (int i = 0; i < num_of_targets; i++) {
        damage_control_array[i] = 0;
    }
    
    for(int i = 0; i < arguments->num_of_drones; i++){
        for(int j = 0; j < num_of_targets; j++){
            if( !arguments->array_of_targets[j].destroyed ){

                int damage = computes_damage(arguments->array_of_drones[i], 
                                            &arguments->array_of_targets[j]);
                
                damage_control_array[j] += damage;

                if( abs(damage_control_array[j]) >= abs(arguments->array_of_targets[j].resistance) ){
                    arguments->array_of_targets[j].destroyed = true;
                }
                
            }
        }
    }
    
    // Blocking others threads to access to the critical section
    pthread_mutex_lock(&available);
    for (int i = 0; i < num_of_targets; i++) {

        if( arguments->array_of_targets[i].type == 0 &&
            !arguments->array_of_targets[i].destroyed ){
            
            arguments->array_of_targets[i].health += damage_control_array[i];
            
            if( arguments->array_of_targets[i].health >= 0 ){
                arguments->array_of_targets[i].destroyed = true;
            }

        } else if( arguments->array_of_targets[i].type == 1 && 
                   !arguments->array_of_targets[i].destroyed ){

            arguments->array_of_targets[i].health += damage_control_array[i];
            
            if( arguments->array_of_targets[i].health <= 0 ){
                arguments->array_of_targets[i].destroyed = true;
            }

        }
    }
    pthread_mutex_unlock(&available);
    // Unblocking others threads to access to the critical section
    
    pthread_exit(0);
    return NULL;
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
void calculate_drone_per_thread( int * array_of_drones_for_threads ){

    float drone_per_thread = (float)num_of_drones/num_of_threads;
    int drone_int = (int) drone_per_thread, dif, i;

    for(int i = 0; i < num_of_threads; i++){
        array_of_drones_for_threads[i] = drone_int;
    }

    drone_per_thread = drone_per_thread - drone_int;

    dif = roundf(drone_per_thread * num_of_threads);

    i = 0;
    while(dif > 0){
        array_of_drones_for_threads[i]++;
        dif--;
        i++; 
    }

}

/**
 * @brief Decides strategy used to process drones. 
 * 
 * @return int: if it's more convenient to work with no matrix, then it will return 1. If it's more convenient to work with the matrix, then it will return 2.
 */
int strategy_decider(){

    long long work_if_no_matrix = (long long)(num_of_drones * num_of_targets);
    return (work_if_no_matrix <= work_if_matrix) ? 1 : 2;

}

/**
 * @brief Creates the threads, also decides what strategy will be used to solve the problem.
 * It decides between two strategies: 
 * 1.- Calculates damage of drones per objectives. (Arithmetic solution)
 * 2.- Calculates how much objectives are in the radius of each drone. (Matrix solution)
 * 
 * @param array_of_threads contains the array of threads
 * @param thread_drone_attr 
 * @param array_of_drones
 * @param arr_of_args_drone
 * @param array_of_targets
 * @return
 */
void create_threads ( pthread_t * array_of_threads, pthread_attr_t * thread_drone_attr, 
                      drone * array_of_drones, thread_args_drone ** arr_of_args_drone, 
                      target * array_of_targets, int strategy ){
    
    int j = 0, array_of_drones_for_threads[num_of_threads];

    calculate_drone_per_thread(array_of_drones_for_threads);

    for (int i = 0; i < num_of_threads; i++){
        
        thread_args_drone * arg = malloc(sizeof(thread_args_drone));
        if (arg == NULL){
            perror("ERROR: Failed allocating dynamic memory!");
            exit(EXIT_FAILURE);
        }

        arg->array_of_drones = (drone *)malloc(array_of_drones_for_threads[i] * sizeof(drone));
        if (!arg->array_of_drones) {
            perror("ERROR: Failed allocating dynamic memory!");
            exit(EXIT_FAILURE);
        }

        for(int k = 0; k < array_of_drones_for_threads[i]; k++){
            arg->array_of_drones[k] = array_of_drones[j];
            j++;
        }
        
        arg->array_of_targets = array_of_targets;
        arg->num_of_drones = array_of_drones_for_threads[i];

        arr_of_args_drone[i] = arg;

        switch (strategy){
            case 1:

                pthread_create( &array_of_threads[i], thread_drone_attr, 
                                drone_damage_targets, arr_of_args_drone[i] );
                break;

            case 2:

                pthread_create( &array_of_threads[i], thread_drone_attr, 
                                drone_damage_targets_matrix, arr_of_args_drone[i] );
                break;

        }

    }
}

/**
 * @brief Waits for threads to finish, starting for the first one, and then the others.
 * 
 * @param array_of_threads contains the array of id's threads so it could identify threads
 * @return void.
 */
void join_threads (pthread_t * array_of_threads){
    for (int i = 0; i < num_of_threads; i++){
        pthread_join(array_of_threads[i], NULL);
    }
}

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
        return false;
    }
    
    // Receives rows and colums
    fscanf(txt_file, "%d %d", &n, &m);

    // Assign memory for matrix land
    land = (target ***)malloc(n * sizeof(target **));
    if (land == NULL) {
        perror("Error assigning memory!\n");
        return false;
    }

    for (int i = 0; i < n; i++) {
        // Assign memory to row
        land[i] = (target **)malloc(m * sizeof(target *)); 
        if (land[i] == NULL) {
            perror("Error assigning memory!\n");
            return false;
        }

        for (int j = 0; j < m; j++) {
            land[i][j] = NULL;  // Initialize pointers in NULL
        }
    }

    // Receives number of targets
    fscanf(txt_file, "%lld", &num_of_targets);

    array_of_targets = (target *) malloc (num_of_targets * sizeof(target));

    for(int i = 1; i <= num_of_targets; i++){
        int coord_x, coord_y, resistance;

        // Receives attributes for each target
        fscanf(txt_file, "%d %d %d", &coord_x, &coord_y, &resistance);
    
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
    fscanf(txt_file, "%lld", &num_of_drones);

    array_of_drones = (drone *) malloc (num_of_drones * sizeof(drone));

    for(int i = 1; i <= num_of_drones; i++){
        int coord_x, coord_y, radius, power;

        fscanf(txt_file, "%d %d %d %d", &coord_x, &coord_y, &radius, &power);

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
 * @brief Used to print ouput required.
 * 
 * @return void, it just prints the output in a specific formats.
 */
void print_output(){

    int om_destroyed_targets = 0, om_parcially_destroyed_targets = 0, 
        ic_destroyed_targets = 0, ic_parcially_destroyed_targets = 0, 
        om_intact_targets = 0, ic_intact_targets = 0;
    
    for (int i = 0; i < num_of_targets; i++){
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

    printf("OM sin destruir: %d\n", om_intact_targets);
    printf("OM parcialmente destruidos: %d\n", om_parcially_destroyed_targets);
    printf("OM totalmente destruido: %d\n", om_destroyed_targets);

    printf("IC sin destruir: %d\n", ic_intact_targets);
    printf("IC parcialmente destruidos: %d\n", ic_parcially_destroyed_targets);
    printf("IC totalmente destruido: %d\n", ic_destroyed_targets);
}

int main(int argc, char *argv[]){

    if(argc != 3){
        printf("Error: You should send exactly 2 arguments!\n");
        return 1;
    }

    num_of_threads = atoi(argv[1]);

    // If an error occurs parsing the input, then return 1.
    if(!parse_input(argv[2])){
        return 1;
    }

    // Initialization of the mutex
    if(pthread_mutex_init(&available, NULL) != 0){
        fprintf(stderr, "Couldn't initialize mutex\n");
        return 1;
    }

    pthread_t array_of_threads[num_of_drones];
    pthread_attr_t thread_drone_attr;
    pthread_attr_init(&thread_drone_attr);

    // Decides the strategy to use in create_threads
    int strategy = strategy_decider();

    // Calls the function to initialize all threads
    create_threads( array_of_threads, &thread_drone_attr, array_of_drones, 
                    arr_of_args_drone, array_of_targets, strategy );
    
    // Waits for threads to finish
    join_threads(array_of_threads);

    print_output();

    // Free section
    for (int i = 0; i < num_of_threads; i++){
        free(arr_of_args_drone[i]->array_of_drones);
    }

    for (int i = 0; i < num_of_drones; i++){
        free(arr_of_args_drone[i]);
    }

    pthread_attr_destroy(&thread_drone_attr);

    for (int i = 0; i < n; i++) {
        free(land[i]);
    }
    free(array_of_targets);
    free(array_of_drones);
    free(land);

    pthread_mutex_destroy(&available);

    return 0;
}