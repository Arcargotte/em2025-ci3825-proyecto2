#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>

int n;
int m;
long long num_of_drones;    
long long num_of_targets;
int num_of_threads = 3;
int work_if_matrix = 0;

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

drone * array_of_drones;
target * array_of_targets;
target *** land = NULL;

typedef struct thread_args_drone thread_args_drone;
struct thread_args_drone{
    drone * array_of_drones;
    target * array_of_targets;
    int num_of_drones;
};

typedef struct thread_args_target thread_args_target;
struct thread_args_target{
    target target;
    drone * array_of_drones;
    int num_of_drones;
};

/* Esta linea esta mal, es importante terminar de trabajar aqui. */
thread_args_drone * arr_of_args_drone[90000];

pthread_mutex_t available;

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
    //printf("Entra dron %d en (%d,%d) y target en (%d,%d)\n", drone.id,drone.x,drone.y, target->x,target->y);
    if (drone.x >= target->x && drone.y >= target->y){
        if (drone.x - drone.radius <= target->x && drone.y - drone.radius <= target->y){
            hits = true;
        }
    }
    //Second Quadrant
    else if (drone.x < target->x && drone.y > target->y){
        if(drone.x + drone.radius >= target->x && drone.y - drone.radius <= target->y){
            hits = true;
        }
    }
    //Third Quadrant
    else if (drone.x > target->x && drone.y < target->y){
        if(drone.x - drone.radius <= target->x && drone.y + drone.radius >= target->y){
            hits = true;
        }
    }
    //Fourth Quadrant
    else if (drone.x <= target->x && drone.y <= target->y){
        if(drone.x + drone.radius >= target->x && drone.y + drone.radius >= target->y){
            hits = true;
        }
    }

    if (hits){
        
        if(target->type == 0){
            return drone.damage;
        }
        return -1 * drone.damage;
        
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

    int x0 = drone.x - drone.radius;
    int y0 = drone.y - drone.radius;

    int i = x0;
    while(i < x0 + (2*drone.radius + 1)){
        int j = y0;
        if(i >= 0 && i < n){
            while(j < y0 + (2*drone.radius + 1)){
                if(j >= 0 && j < m){
                    if(land[i][j] != NULL && land[i][j]->id > 0 && land[i][j]->destroyed == false && land[i][j]->type == 0){
                        pthread_mutex_lock(&available);
                        land[i][j]->health += drone.damage; 
                        if(land[i][j]->health >= 0){
                            land[i][j]->destroyed = true;
                        }
                        pthread_mutex_unlock(&available);
                    } else if(land[i][j] != NULL && land[i][j]->id > 0 && land[i][j]->destroyed == false && land[i][j]->type == 1){
                        pthread_mutex_lock(&available);
                        land[i][j]->health -= drone.damage;
                        if(land[i][j]->health <= 0){
                            land[i][j]->destroyed = true;
                        }
                        pthread_mutex_unlock(&available);
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
        for (int j = 0; j < num_of_targets; j++){
            if(!arguments->array_of_targets[j].destroyed){
                int damage = computes_damage(arguments->array_of_drones[i], &arguments->array_of_targets[j]);
                damage_control_array[j] += damage;

                if((arguments->array_of_targets[j].type == 0 && abs(damage_control_array[j]) >= abs(arguments->array_of_targets[j].resistance)) || 
                    (arguments->array_of_targets[j].type == 1 && abs(damage_control_array[j]) >= abs(arguments->array_of_targets[j].resistance))){
                    arguments->array_of_targets[j].destroyed = true;
                }
            }
        }
    }
    
    //Blocking others threads to access to the critical section
    pthread_mutex_lock(&available);
    for (int i = 0; i < num_of_targets; i++) {

        if(arguments->array_of_targets[i].type == 0 && !arguments->array_of_targets[i].destroyed){
            
            arguments->array_of_targets[i].health += damage_control_array[i];
            if(arguments->array_of_targets[i].health >= 0){
                arguments->array_of_targets[i].destroyed = true;
            }
        } else if(arguments->array_of_targets[i].type == 1 && !arguments->array_of_targets[i].destroyed){
            arguments->array_of_targets[i].health += damage_control_array[i];
            
            if(arguments->array_of_targets[i].health <= 0){
                arguments->array_of_targets[i].destroyed = true;
            }
        }
    }
    pthread_mutex_unlock(&available);
    //Unblocking others threads to access to the critical section
    
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

    int drone_int = (int) drone_per_thread;

    for(int i = 0; i < num_of_threads; i++){
        array_of_drones_for_threads[i] = drone_int;
    }

    // I just need the decimal part
    drone_per_thread = drone_per_thread - drone_int;

    // I just need the decimal part
    int dif = roundf(drone_per_thread * num_of_threads);

    int i = 0;
    while(dif > 0){
        array_of_drones_for_threads[i]++;
        dif--;
        i++; 
    }

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
void create_threads (pthread_t * array_of_threads, pthread_attr_t * thread_drone_attr, drone * array_of_drones, thread_args_drone ** arr_of_args_drone, target * array_of_targets){
    //CREA HILO POR DRON
    int j = 0;
    int array_of_drones_for_threads[num_of_threads];

    calculate_drone_per_thread(array_of_drones_for_threads);

    for (int i = 0; i < num_of_threads; i++){
        /*ASIGNA MEMORIA DINÁMICAMENTE*/
        thread_args_drone * arg = malloc(sizeof(thread_args_drone));
        if (arg == NULL){
            printf("ERROR: Failed allocating dynamic memory!");
            return;
        }

        arg->array_of_drones = (drone *) malloc(array_of_drones_for_threads[i] * sizeof(drone));
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
        if(num_of_drones * num_of_targets <= work_if_matrix){
            pthread_create(&array_of_threads[i], thread_drone_attr, drone_damage_targets, arr_of_args_drone[i]);
        } else{
            pthread_create(&array_of_threads[i], thread_drone_attr, drone_damage_targets_matrix, arr_of_args_drone[i]);
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
    /*
        Por cada uno de los elementos en un arreglo de hilos, ejecutar la función pthread_join() para que el proceso espere a que los hilos terminen ejecución.
    */
    for (int i = 0; i < num_of_threads; i++){
        pthread_join(array_of_threads[i], NULL);
    }
}


/**
 * @brief Used to receive the input with a specific format and to initialize all global variables used in the code
 * 
 * @param file_name name or path to the file in format .txt used as input
 * @return bool, true in case everything goes well, otherwise false.
 */
bool parse_input(char * file_name){
    
    FILE *txt_file;
    // Opens file with read function
    txt_file = fopen(file_name, "r");

    // In case the file couldn't open
    if (txt_file == NULL) {
        printf("\x1b[31mError:\x1b[37m Couldn't open text file!\n");
        return false;
    }
    
    fscanf(txt_file, "%d %d", &n, &m);

    // Assign memory for matrix land
    land = (target ***)malloc(n * sizeof(target **));

    if (land == NULL) {
        perror("Error assigning memory!\n");
        return false;
    }

    for (int i = 0; i < n; i++) {
        land[i] = (target **)malloc(m * sizeof(target *));
        if (land[i] == NULL) {
            perror("Error assigning memory!\n");
            return false;
        }

        for (int j = 0; j < m; j++) {
            land[i][j] = NULL;  // Initialize pointers in NULL
        }
    }

    fscanf(txt_file, "%lld", &num_of_targets);
    //Assign memory for array of targets
    array_of_targets = (target *) malloc (num_of_targets * sizeof(target));
    for(int i = 1; i <= num_of_targets; i++){
        
        int coord_x;  
        int coord_y;
        int resistance;

        fscanf(txt_file, "%d %d %d", &coord_x, &coord_y, &resistance);

        target * new_target = (target *) malloc (sizeof(target));
        new_target->x = coord_x;
        new_target->y = coord_y;
        new_target->health = resistance;
        new_target->resistance = resistance;
        new_target->id = i;
        if(resistance < 0){
            new_target->type = 0;
        } else {
            new_target->type = 1;
        }
        new_target->destroyed = false;

        memcpy(&array_of_targets[i - 1], new_target, sizeof(target));
        land[coord_x][coord_y] = &array_of_targets[i - 1];

        free(new_target);

    }

    fscanf(txt_file, "%lld", &num_of_drones);

    array_of_drones = (drone *) malloc (num_of_drones * sizeof(drone));

    for(int i = 1; i <= num_of_drones; i++){
        
        int coord_x;  
        int coord_y;
        int radius;
        int power;

        fscanf(txt_file, "%d %d %d %d", &coord_x, &coord_y, &radius, &power);

        // This is a variable used to determine if it's convenient to use the matrix
        work_if_matrix += (2*radius + 1)*(2*radius + 1);

        drone * new_drone = (drone *) malloc (sizeof(drone));
        new_drone->x = coord_x;
        new_drone->y = coord_y;
        new_drone->radius = radius;
        new_drone->damage = power;
        new_drone->id = i;

        memcpy(&array_of_drones[i-1], new_drone, sizeof(drone));
        free(new_drone);

    }

    fclose(txt_file);
    return true;
}


int main(int argc, char *argv[]){

    if(argc < 1 || argc > 3){
        return 1;
    }

    // If an error occurs parsing the input, then return 1.
    if(!parse_input(argv[1])){
        return 1;
    }

    printf("Work if Matrix: %d \nWork if not matrix: %lld\n", work_if_matrix, num_of_drones * num_of_targets);

    // Initialization of the mutex
    if(pthread_mutex_init(&available, NULL) != 0){
        fprintf(stderr, "Couldn't initialize mutex\n");
        return 1;
    }

    // Array of ID's threads
    pthread_t array_of_threads[num_of_drones];

    pthread_attr_t thread_drone_attr;

    pthread_attr_init(&thread_drone_attr);

    // Calls the function to initialize all threads
    create_threads(array_of_threads, &thread_drone_attr, array_of_drones, arr_of_args_drone, array_of_targets);
    // Waits for threads to finish
    join_threads(array_of_threads);

    // Calculates the output and prints it in a specific format
    int om_destroyed_targets = 0, om_parcially_destroyed_targets = 0, om_intact_targets = 0,
        ic_destroyed_targets = 0, ic_parcially_destroyed_targets = 0, ic_intact_targets = 0;

    for (int i = 0; i < num_of_targets; i++){
        if(array_of_targets[i].type == 0 && !array_of_targets[i].destroyed){
            if(array_of_targets[i].resistance == array_of_targets[i].health){
                om_intact_targets++;
            } else{
                om_parcially_destroyed_targets++;
            }
        } else if(array_of_targets[i].type == 0 && array_of_targets[i].destroyed){
            om_destroyed_targets++;
        } else if(array_of_targets[i].type == 1 && !array_of_targets[i].destroyed){
            if(array_of_targets[i].resistance == array_of_targets[i].health){
                ic_intact_targets++;
            } else{
                ic_parcially_destroyed_targets++;
            }
        } else if(array_of_targets[i].type == 1 && array_of_targets[i].destroyed){
            ic_destroyed_targets++;
        }
    }

    printf("OM sin destruir: %d \nOM parcialmente destruidos: %d \nOM totalmente destruido: %d\n", om_intact_targets, om_parcially_destroyed_targets, om_destroyed_targets);
    printf("IC sin destruir: %d \nIC parcialmente destruidos: %d \nIC totalmente destruido: %d\n", ic_intact_targets, ic_parcially_destroyed_targets, ic_destroyed_targets);
    
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