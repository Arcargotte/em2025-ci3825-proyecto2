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
int num_of_drones;    
int num_of_targets;
int num_of_threads = 3;

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

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

typedef struct thread_args_drone thread_args_drone;
struct thread_args_drone{
    drone * array_of_drones;
    target * array_of_targets;
    int num_of_targets;
    int num_of_drones;
};

typedef struct thread_args_target thread_args_target;
struct thread_args_target{
    target target;
    drone * array_of_drones;
    int num_of_drones;
};


/* Esta linea esta mal, es importante terminar de trabajar aqui. */
thread_args_drone * arr_of_args_drone[31680];

pthread_mutex_t available;

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
    
    return NULL;
}

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
        arg->num_of_targets = num_of_targets;
        arg->num_of_drones = array_of_drones_for_threads[i];

        arr_of_args_drone[i] = arg;

        pthread_create(&array_of_threads[i], thread_drone_attr, drone_damage_targets, arr_of_args_drone[i]);
    }
}

void join_threads (pthread_t * array_of_threads){
    /**
     * Por cada uno de los elementos en un arreglo de hilos, ejecutar la función pthread_join() para que el proceso espere a que los hilos terminen ejecución.
    */
    for (int i = 0; i < num_of_threads; i++){
        pthread_join(array_of_threads[i], NULL);
    }
}

void kill_threads(){
    /*
        Elimina el bloque de atributos de los hilos del arreglo de hilos ejecutando la función pthread_attr_destroy.
    */
}

int main(void){

    double start = get_time();

    drone * array_of_drones;
    target * array_of_targets;

    // --------------------------------------------------------

    FILE *txt_file;
    // Opens file with read function
    txt_file = fopen("archivo.txt", "r");

    // In case the file couldn't open
    if (txt_file == NULL) {
        printf("\x1b[31mError:\x1b[37m Couldn't open text file!\n");
        return false;
    }

    // Reads the content of the file line by line
    int line_counter = 1;
    char line[100];
    while (fgets(line, sizeof(line), txt_file) != NULL) {
        
        if(line_counter == 1){
            // Get the memory space needed for the rows
            int q = 0;
            while(line[q] != ' '){
                q++;
            }

            // Get the memory space needed for the columns
            int j = q + 1;
            int i = 0;
            while(line[j] != '\n'){
                j++;
                i++;
            }

            const int rows_size = q; 
            const int columns_size = i; 

            char line_rows[rows_size + 1];
            char line_columns[columns_size + 1];

            q = 0;
            while(line[q] != ' '){
                line_rows[q] = line[q];
                q++;
            }
            line_rows[q] = '\0';

            j = q + 1;
            i = 0;
            while(line[j] != '\n'){
                line_columns[i] = line[j];
                j++;
                i++;
            }
            line_columns[i] = '\0';

            n = atoi(line_rows);  
            m = atoi(line_columns);

        } else if (line_counter == 2){
            int i = 0;
            while(line[i] != '\n'){
                i++;
            }
            const int chars_target = i;
            char line_num_of_targets[chars_target + 1];

            i = 0;
            while(line[i] != '\n'){
                line_num_of_targets[i] = line[i];
                i++;
            }
            line_num_of_targets[i] = '\0';

            num_of_targets = atoi(line_num_of_targets);

            array_of_targets = (target *) malloc (num_of_targets * sizeof(target));

        } else if( 2 + num_of_targets >= line_counter &&  line_counter > 2){
            
            // Get the memory space needed for
            int i = 0;
            while(line[i] != ' '){
                i++;
            }

            const int coord_x_size = i; 

            // Get the memory space needed for
            int j = i + 1;
            i = 0;
            while(line[j] != ' '){
                j++;
                i++;
            }

            const int coord_y_size = i;

            // Get the memory space needed for
            int k = j + 1;
            i = 0;
            while(line[k] != '\n' && line[k] != '\0'){
                k++;
                i++;
            }

            const int resistance_size = i; 

            char line_x[coord_x_size + 1];
            char line_y[coord_y_size + 1];
            char line_resistance[resistance_size + 1];

            // Get the memory space needed for
            i = 0;
            while(line[i] != ' '){
                line_x[i] = line[i];
                i++;
            }
            line_x[i] = '\0';

            // Get the memory space needed for
            j = i + 1;
            i = 0;
            while(line[j] != ' '){
                line_y[i] = line[j];
                j++;
                i++;
            }
            line_y[i] = '\0';

            // Get the memory space needed for
            k = j + 1;
            i = 0;
            while(line[k] != '\n'){
                line_resistance[i] = line[k];
                k++;
                i++;
            }
            line_resistance[i] = '\0';

            int coord_x = atoi(line_x);  
            int coord_y = atoi(line_y);
            int resistance = atoi(line_resistance);

            target * new_target = (target *) malloc (sizeof(target));
            new_target->x = coord_x;
            new_target->y = coord_y;
            new_target->health = resistance;
            new_target->resistance = resistance;
            new_target->id = line_counter - 2;
            if(resistance < 0){
                new_target->type = 0;
            } else {
                new_target->type = 1;
            }
            new_target->destroyed = false;

            memcpy(&array_of_targets[line_counter - 3], new_target, sizeof(target));
            free(new_target);

        } else if( 3 + num_of_targets == line_counter ){

            int i = 0;
            while(line[i] != '\n'){
                i++;
            }
            const int chars_drone = i;
            char line_num_of_drones[chars_drone + 1];

            i = 0;
            while(line[i] != '\n'){
                line_num_of_drones[i] = line[i];
                i++;
            }
            line_num_of_drones[i] = '\0';

            num_of_drones = atoi(line_num_of_drones);

            array_of_drones = (drone *) malloc (num_of_drones * sizeof(drone));

        } else if( 3 + num_of_targets + num_of_drones >= line_counter &&  line_counter > 3 + num_of_targets ){

            // Get the memory space needed for
            int i = 0;
            while(line[i] != ' '){
                i++;
            }

            const int coord_x_size = i; 

            // Get the memory space needed for
            int j = i + 1;
            i = 0;
            while(line[j] != ' '){
                j++;
                i++;
            }

            const int coord_y_size = i;

            // Get the memory space needed for
            int k = j + 1;
            i = 0;
            while(line[k] != ' '){
                k++;
                i++;
            }

            const int radius_size = i; 

            // Get the memory space needed for
            j = k + 1;
            i = 0;
            while(line[j] != '\n' && line[j] != '\0'){
                j++;
                i++;
            }

            const int power_size = i; 

            char line_x[coord_x_size + 1];
            char line_y[coord_y_size + 1];
            char line_radius[radius_size + 1];
            char line_power[power_size + 1];

            // Get the memory space needed for
            i = 0;
            while(line[i] != ' '){
                line_x[i] = line[i];
                i++;
            }
            line_x[i] = '\0';

            // Get the memory space needed for
            j = i + 1;
            i = 0;
            while(line[j] != ' '){
                line_y[i] = line[j];
                j++;
                i++;
            }
            line_y[i] = '\0';

            // Get the memory space needed for
            k = j + 1;
            i = 0;
            while(line[k] != ' '){
                line_radius[i] = line[k];
                k++;
                i++;
            }
            line_radius[i] = '\0';

            // Get the memory space needed for
            j = k + 1;
            i = 0;
            while(line[j] != '\n' && line[j] != '\0'){
                line_power[i] = line[j];
                j++;
                i++;
            }
            line_power[i] = '\0';

            int coord_x = atoi(line_x);  
            int coord_y = atoi(line_y);
            int radius = atoi(line_radius);
            int power = atoi(line_power);

            drone * new_drone = (drone *) malloc (sizeof(drone));
            new_drone->x = coord_x;
            new_drone->y = coord_y;
            new_drone->radius = radius;
            new_drone->damage = power;
            new_drone->id = line_counter - (3 + num_of_targets);

            memcpy(&array_of_drones[line_counter - (4 + num_of_targets)], new_drone, sizeof(drone));
            free(new_drone);
        }

        line_counter++;
    }  

    // Close file
    fclose(txt_file);

    if(pthread_mutex_init(&available, NULL) != 0){
        fprintf(stderr, "Couldn't initialize mutex\n");
        return 1;
    }

    pthread_t array_of_threads[num_of_drones];

    pthread_attr_t thread_drone_attr;

    pthread_attr_init(&thread_drone_attr);

    double start_threads = get_time();
    create_threads(array_of_threads, &thread_drone_attr, array_of_drones, arr_of_args_drone, array_of_targets);
    join_threads(array_of_threads);
    
    double end_threads = get_time();

    printf("Tiempo de ejecución de hilos: %.6f segundos\n", end_threads - start_threads);


    //FREE DINAMICALLY ALLOCATED MEMORY FOR THE ARRAY OF DRONES IN arr_of_args_drone
    for (int i = 0; i < num_of_threads; i++){
        free(arr_of_args_drone[i]->array_of_drones);
    }

    //FREE DYNAMICALLY ALLOCATED MEMORY FOR ARGUMENTS IN ARRAY OF ARGUMENTS (DRONES AND TARGETS)
    for (int i = 0; i < num_of_drones; i++){
        free(arr_of_args_drone[i]);
    }

    //FREE DYNAMICALLY ALLOCATED MEMORY FOR ARGUMENTS IN ARRAY OF ARGUMENTS (DRONES AND TARGETS)
    pthread_attr_destroy(&thread_drone_attr);


    int om_destroyed_targets = 0;
    int om_parcially_destroyed_targets = 0;
    int om_intact_targets = 0;
    int ic_destroyed_targets = 0;
    int ic_parcially_destroyed_targets = 0;
    int ic_intact_targets = 0;

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
    
    free(array_of_targets);
    free(array_of_drones);

    pthread_mutex_destroy(&available);

    double end = get_time();
    printf("Tiempo de ejecución: %.6f segundos\n", end - start);

    return 0;
}