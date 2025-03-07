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
#include <fcntl.h>
#include <stdbool.h>
#define SIZE 128


int n = 10;
int m = 10;

int num_of_processes = 2 ; //Creates 4 child processes.

int num_of_drones;
int num_of_targets;

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

pthread_mutex_t * available;

void parse_input(){
    FILE *txt_file;
    // Opens file with read function
    txt_file = fopen("archivo.txt", "r");

    // In case the file couldn't open
    if (txt_file == NULL) {
        printf("\x1b[31mError:\x1b[37m Couldn't open text file!\n");
        return;
    }

    // Reads the content of the file line by line
    int line_counter = 1;
    char line[100];
    while (fgets(line, sizeof(line), txt_file) != NULL) {
        
        if(line_counter == 1){
            // Get the memory space needed for
            int q = 0;
            while(line[q] != ' '){
                q++;
            }

            // Get the memory space needed for
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

            m = atoi(line_rows);  
            n = atoi(line_columns);

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
            while(line[k] != '\n'){
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
            while(line[j] != '\n'){
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
            while(line[j] != '\n'){
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
}

void calculate_drone_per_thread( int * array_of_drones_per_threads ){

    float drone_per_thread = (float)num_of_drones/num_of_processes;

    int drone_int = (int) drone_per_thread;

    for(int i = 0; i < num_of_processes; i++){
        array_of_drones_per_threads[i] = drone_int;
    }

    // I just need the decimal part
    drone_per_thread = drone_per_thread - drone_int;

    // I just need the decimal part
    int dif = roundf(drone_per_thread * num_of_processes);

    int i = 0;
    while(dif > 0){
        array_of_drones_per_threads[i]++;
        dif--;
        i++; 
    }

}

void computes_damage (drone drone, target * target){

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
        pthread_mutex_lock(available);
        if(target->type == 0){
            target->health = target->health + drone.damage;
            if(target->health >= 0){
                target->destroyed = true;
            }
        } else{
            target->health = target->health - drone.damage;
            if(target->health <= 0){
                target->destroyed = true;
            }
        }
        pthread_mutex_unlock(available);
    }
}

int main (void){
        
    // SHARED ANONYMOUS MEMORY SPACE

    available = (pthread_mutex_t *) mmap(NULL, sizeof(pthread_mutex_t),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (available == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Inicializar atributos del mutex
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    // Inicializar el mutex en la memoria compartida
    if (pthread_mutex_init(available, &attr) != 0) {
        perror("pthread_mutex_init");
        exit(EXIT_FAILURE);
    }

    pid_t array_of_processes[num_of_processes];

    target * shared_array_of_targets = (target *) mmap(NULL, 4 * sizeof(target),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (array_of_targets == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }    

    parse_input();

    int array_of_drones_per_process[num_of_processes];

    calculate_drone_per_thread(array_of_drones_per_process);

    for (int i = 0; i < num_of_targets; i++){
        shared_array_of_targets[i].id = array_of_targets[i].id;
        shared_array_of_targets[i].health = array_of_targets[i].health;
        shared_array_of_targets[i].resistance = array_of_targets[i].resistance;
        shared_array_of_targets[i].type = array_of_targets[i].type;
        shared_array_of_targets[i].destroyed = array_of_targets[i].destroyed;
        shared_array_of_targets[i].x = array_of_targets[i].x;
        shared_array_of_targets[i].y = array_of_targets[i].y;

    }
    
    pid_t id_process = 1;

    int process_iter = -1;

    for (int k = 0; k < num_of_processes; k++){
        process_iter++;
        id_process = fork();

        if ((int) id_process < 0){
            printf("Error forking process\n");
            return 1;
        }
        if (id_process == 0) {
            for(int i = 0; i < array_of_drones_per_process[process_iter]; i++){
                for (int j = 0; j < num_of_targets; j++){
                    if(!shared_array_of_targets[j].destroyed){
                        computes_damage(array_of_drones[i + process_iter * array_of_drones_per_process[process_iter]], &shared_array_of_targets[j]);
                    }
                }
            }
            fflush(stdout);  // Forzar salida inmediata
            exit(0);
        }

        array_of_processes[k] = id_process;
    }

    while (wait(NULL) > 0);


    free(array_of_targets);
    free(array_of_drones);

    int om_destroyed_targets = 0, om_parcially_destroyed_targets = 0, om_intact_targets = 0,
        ic_destroyed_targets = 0, ic_parcially_destroyed_targets = 0, ic_intact_targets = 0;

    for (int i = 0; i < num_of_targets; i++){
        if(shared_array_of_targets[i].type == 0 && !shared_array_of_targets[i].destroyed){
            if(shared_array_of_targets[i].resistance == shared_array_of_targets[i].health){
                om_intact_targets++;
            } else{
                om_parcially_destroyed_targets++;
            }
        } else if(shared_array_of_targets[i].type == 0 && shared_array_of_targets[i].destroyed){
            om_destroyed_targets++;
        } else if(shared_array_of_targets[i].type == 1 && !shared_array_of_targets[i].destroyed){
            if(shared_array_of_targets[i].resistance == shared_array_of_targets[i].health){
                ic_intact_targets++;
            } else{
                ic_parcially_destroyed_targets++;
            }
        } else if(shared_array_of_targets[i].type == 1 && shared_array_of_targets[i].destroyed){
            ic_destroyed_targets++;
        }
    }

    printf("OM sin destruir: %d \nOM parcialmente destruidos: %d \nOM totalmente destruido: %d\n", om_intact_targets, om_parcially_destroyed_targets, om_destroyed_targets);
    printf("IC sin destruir: %d \nIC parcialmente destruidos: %d \nIC totalmente destruido: %d\n", ic_intact_targets, ic_parcially_destroyed_targets, ic_destroyed_targets);
    
    printf("El primer elemento es: %d\n", array_of_processes[0]);

    return 0;
}