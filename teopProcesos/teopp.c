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

int n = 10;
int m = 10;

int num_of_processes = 3;

long long num_of_drones;
long long num_of_targets;
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
target *** land;

pthread_mutex_t * available;

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
    
    array_of_targets = (target *) mmap(NULL, num_of_targets * sizeof(target),
                             PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    if (array_of_targets == MAP_FAILED) {
        perror("mmap failed");
        exit(EXIT_FAILURE);
    }

    for(int i = 1; i <= num_of_targets; i++){
        
        int coord_x;  
        int coord_y;
        int resistance;

        fscanf(txt_file, "%d %d %d", &coord_x, &coord_y, &resistance);

        array_of_targets[i - 1].x = coord_x;
        array_of_targets[i - 1].y = coord_y;
        array_of_targets[i - 1].health = resistance;
        array_of_targets[i - 1].resistance = resistance;
        array_of_targets[i - 1].id = i;
        if(resistance < 0){
            array_of_targets[i - 1].type = 0;
        } else {
            array_of_targets[i - 1].type = 1;
        }
        array_of_targets[i - 1].destroyed = false;

        land[coord_x][coord_y] = &array_of_targets[i - 1];

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
    //sssprintf("Entra dron %d en (%d,%d) y target en (%d,%d)\n", drone.id,drone.x,drone.y, target->x,target->y);
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
                        pthread_mutex_lock(available);
                        land[i][j]->health += drone.damage; 
                        if(land[i][j]->health >= 0){
                            land[i][j]->destroyed = true;
                        }
                        pthread_mutex_unlock(available);
                    } else if(land[i][j] != NULL && land[i][j]->id > 0 && land[i][j]->destroyed == false && land[i][j]->type == 1){
                        pthread_mutex_lock(available);
                        land[i][j]->health -= drone.damage;
                        if(land[i][j]->health <= 0){
                            land[i][j]->destroyed = true;
                        }
                        pthread_mutex_unlock(available);
                    }
                }
                j++;
            }
        }
        i++;
    }

}


int main (int argc, char *argv[]){
    
    if(argc < 1 || argc > 3){
        return 1;
    }

    if(!parse_input(argv[1])){
        return 1;
    }

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

    int array_of_drones_per_process[num_of_processes];

    calculate_drone_per_thread(array_of_drones_per_process);
    
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
            
            if(num_of_drones * num_of_targets <= work_if_matrix){
                printf("Es mejor sin matriz.\n");
                for(int i = 0; i < array_of_drones_per_process[process_iter]; i++){
                    for (int j = 0; j < num_of_targets; j++){
                        if(!array_of_targets[j].destroyed){
                            computes_damage(array_of_drones[i + process_iter * array_of_drones_per_process[process_iter]], &array_of_targets[j]);
                        }
                    }
                }
                

            } else {
                printf("Es mejor con matriz.\n");
                for(int i = 0; i < array_of_drones_per_process[process_iter]; i++){
                    computes_damage_in_matrix(array_of_drones[i + process_iter * array_of_drones_per_process[process_iter]]);
                }
            }

            for (int i = 0; i < n; i++) {
                free(land[i]);
            }
            free(land);
            free(array_of_drones);
            fflush(stdout);  // Forzar salida inmediata
            exit(0); 
        }

        array_of_processes[k] = id_process;
    }

    while (wait(NULL) > 0);

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
    
    printf("El primer elemento es: %d\n", array_of_processes[0]);

    for (int i = 0; i < n; i++) {
        free(land[i]);
    }
    free(land);
    
    munmap(array_of_targets, num_of_targets * sizeof(target));
    munmap(available, sizeof(pthread_mutex_t));
    free(array_of_drones);
    

    return 0;
}