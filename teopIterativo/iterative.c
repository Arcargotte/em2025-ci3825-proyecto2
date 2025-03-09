#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

int n;
int m;
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
    }
}

drone * array_of_drones;
target * array_of_targets;
// Matrix with targets and empty spaces
target *** land = NULL;

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
                        land[i][j]->health += drone.damage; 
                        if(land[i][j]->health >= 0){
                            land[i][j]->destroyed = true;
                        }
                    } else if(land[i][j] != NULL && land[i][j]->id > 0 && land[i][j]->destroyed == false && land[i][j]->type == 1){
                        land[i][j]->health -= drone.damage;
                        if(land[i][j]->health <= 0){
                            land[i][j]->destroyed = true;
                        }
                    }
                }
                j++;
            }
        }
        i++;
    }

}

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
    
    if(!parse_input(argv[1])){
        return 1;
    }

    long long work_no_matrix = (long long)(num_of_drones * num_of_targets);

    printf("Work if Matrix: %d \nWork if not matrix: %lld\n", work_if_matrix, work_no_matrix);

    if(work_no_matrix <= work_if_matrix){

        printf("Es mejor no hacer matriz\n");

        for(int i = 0; i < num_of_drones; i++){
            for(int j = 0; j < num_of_targets; j++){
                if(!array_of_targets[j].destroyed){
                    computes_damage(array_of_drones[i], &array_of_targets[j]);
                }
            }
        }

    } else {

        printf("Es mejor hacer la matriz\n");

        for(int i = 0; i < num_of_drones; i++){
            computes_damage_in_matrix(array_of_drones[i]);
        }

    }

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
                printf("Estoy intacto y soy target (%d, %d, %d)\n", array_of_targets[i].id, array_of_targets[i].x, array_of_targets[i].y);
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
    

    for (int i = 0; i < n; i++) {
        free(land[i]);
    }

    free(array_of_targets);
    free(array_of_drones);
    free(land);

    return 0;
}