#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>

int n = 10;
int m = 10;
int num_of_drones;
int num_of_targets;

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
    drone drone;
    target * array_of_targets;
    int num_of_targets;
};


void computes_damage (drone drone, target * target){

    bool hits = false;
    // First Quadrant
    printf("Entra dron %d en (%d,%d) y target en (%d,%d)\n", drone.id,drone.x,drone.y, target->x,target->y);
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
        printf("It hits. Current health: %d. Damage done: %d \n", target->health,drone.damage);
        
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
    } else{
        printf("It doesn't hit.\n");

    }
}

void * drone_damage_targets (void * args){
    
    thread_args_drone * arguments = (thread_args_drone * ) args;

    for (int j = 0; j < arguments->num_of_targets; j++){
        computes_damage(arguments->drone, &arguments->array_of_targets[j]);
        printf("Health left: %d\n",arguments->array_of_targets[j].health);
    }

    return NULL;
}

drone * array_of_drones;
target * array_of_targets;

int main(void){
    
    double start = get_time();

    num_of_drones = 3;
    num_of_targets = 4;

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

            int rows = atoi(line_rows);  
            int columns = atoi(line_columns);

            printf("Rows: %d\n", rows);
            printf("Columns: %d\n", columns);

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

            printf("Targets: %d\n", num_of_targets);

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

    for(int i = 0; i < num_of_drones; i++){
        for(int j = 0; j < num_of_targets; j++){
            if(!array_of_targets[j].destroyed){
                computes_damage(array_of_drones[i], &array_of_targets[j]);
            }
        }
    }
    
    /*PRUEBA*/
    printf("\n");
    for (int i = 0; i < num_of_targets; i++){
        printf("target en posicion (%d,%d) tiene health %d \n",array_of_targets[i].x, array_of_targets[i].y, array_of_targets[i].health);
    }

    free(array_of_targets);
    free(array_of_drones);

    double end = get_time();
    printf("Tiempo de ejecución: %.6f segundos\n", end - start);

    return 0;
}