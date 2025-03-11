#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

// Global variables
long long n, m, num_of_drones, num_of_targets, work_if_matrix = 0;

typedef struct drone {
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

// Global arrays
drone * array_of_drones;
target * array_of_targets;
target *** land = NULL;

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

    if( hits ){
        if( target->type == 0 ){

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
    }
}

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

                        land[i][j]->health += drone.damage; 

                        if( land[i][j]->health >= 0 ){
                            land[i][j]->destroyed = true;
                        }

                    } else if( land != NULL && land[i][j] != NULL && land[i][j]->id > 0 && 
                               land[i][j]->destroyed == false && 
                               land[i][j]->type == 1 ){

                        land[i][j]->health -= drone.damage;

                        if( land[i][j]->health <= 0 ){
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

    for (int i = 0; i < n; i++) {
        // Assign memory to row
        land[i] = (target **)malloc(m * sizeof(target *)); 
        if (land[i] == NULL) {
            perror("Error assigning memory!\n");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < m; j++) {
            land[i][j] = NULL;  // Initialize pointers in NULL
        }
    }

    // Receives number of targets

    if ( fscanf(txt_file, "%lld", &num_of_targets) != 1 ) {
        perror("Error: Wrong format!\n");
        exit(EXIT_FAILURE);
    }

    array_of_targets = (target *) malloc (num_of_targets * sizeof(target));

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

int strategy_decider(){

    long long work_if_no_matrix = (long long)(num_of_drones * num_of_targets);
    return (work_if_no_matrix <= work_if_matrix) ? 1 : 2;

}

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

int main(int argc, char *argv[]){

    int strategy;

    if(argc != 2){
        printf("Error: You should send exactly 1 argument!");
        return 1;
    }
    
    if(!parse_input(argv[1])){
        return 1;
    }

    strategy = strategy_decider();

    switch (strategy){
        case 1:

            for(long long i = 0; i < num_of_drones; i++){
                for(long long j = 0; j < num_of_targets; j++){
                    if(!array_of_targets[j].destroyed){
                        computes_damage(array_of_drones[i], &array_of_targets[j]);
                    }
                }
            }
            break;

        case 2:

            for(long long i = 0; i < num_of_drones; i++){
                computes_damage_in_matrix(array_of_drones[i]);
            }

            break;

    }

    print_output();

    if (land != NULL) {
        for(long long i = 0; i < n; i++){
            free(land[i]);
        }
    }
    free(land);
    free(array_of_targets);
    free(array_of_drones);

    return 0;
}