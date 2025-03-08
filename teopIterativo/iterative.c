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
int num_of_drones;
int num_of_targets;

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

bool parse_input(){

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

            land = (target ***)malloc(n * sizeof(target **));

            if (land == NULL) {
                perror("Error assigning memory!\n");
                return false;
            }

            for (int i = 0; i < n; i++) {
                land[i] = (target **)malloc(m * sizeof(target *));
                if (land[i] == NULL) {
                    perror("Error al asignar memoria");
                    return false;
                }

                for (int j = 0; j < m; j++) {
                    land[i][j] = NULL;  // Initialize pointers in NULL
                }
            }


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
            land[coord_x][coord_y] = &array_of_targets[line_counter - 3];

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


            // This is a variable used to determine if it's convenient to use the matrix
            work_if_matrix += (2*radius + 1)*(2*radius + 1);

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

    return true;
}

int main(void){

    if(!parse_input()){
        return 1;
    }

    printf("Work if Matrix: %d \nWork if not matrix: %d\n", work_if_matrix, num_of_drones * num_of_targets);

    if(num_of_drones * num_of_targets <= work_if_matrix){

        printf("Es mejor no hacer matriz\n");

        for(int i = 0; i < num_of_drones; i++){
            double x = 0.0;
            // Trabajo intensivo en CPU
            for (long j = 0; j < 10000; j++) {
                x += (double)j / (j + 1); // Cálculo simple pero pesado
            }
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