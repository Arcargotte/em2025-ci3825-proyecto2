#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>

int n = 10;
int m = 10;

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

typedef struct thread_args_target thread_args_target;
struct thread_args_target{
    target target;
    drone * array_of_drones;
    int num_of_drones;
};

thread_args_drone * arr_of_args_drone[3];

pthread_mutex_t available;

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
        pthread_mutex_lock(&available);
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
        pthread_mutex_unlock(&available);
    } else{
        printf("It doesn't hit.\n");

    }
}

void * drone_damage_targets (void * args){
    
    thread_args_drone * arguments = (thread_args_drone * ) args;

    for (int j = 0; j < arguments->num_of_targets; j++){
        if(!arguments->array_of_targets[j].destroyed){
            computes_damage(arguments->drone, &arguments->array_of_targets[j]);
        }
        printf("Health left: %d\n",arguments->array_of_targets[j].health);
    }

    return NULL;
}

void create_threads (pthread_t * array_of_threads, pthread_attr_t * thread_drone_attr, drone * array_of_drones, thread_args_drone ** arr_of_args_drone, target * array_of_targets, int num_of_drones, int num_of_targets){
    /**
     * Procesa input del usuario de la siguiente manera:
     * 1. Usuario introduce el número de objetivos del mapa, coordenadas y resistencia
     * 1.1. En un bucle interno, por línea de input del usuario es creado una estructura target e insertado en la lista de objetivos
     *  
     * 2. Usuario introduce el número de drones, coordenadas, radio de explosión y poder
     * 2.1 En un bucle interno, por línea de input del usuario es creada una estructura drone e insertado en el arreglo de drones
     * 
     * 3. Son comparados el número de drones y el número de objetivos
     * 3.1. Si n_drones es menor que n_objetivos, por cada drone en el arreglo de drones, crear una lista de thread_args de tipo drone; en caso contrario,
     * crear una lista de thread_args de tipo target
     * 
     * 4. Por cada elemento en la lista de thread_args, agregar el identificador de un hilo a un arreglo de hilos
     * 5. Por cada elemento en el arreglo de hilos, ejecutar la función create_thread
     */

    /**
      * CREA ARREGLO DE HILOS
      */

    //CREA HILO POR DRON
    for (int i = 0; i < num_of_drones; i++){
        /*ASIGNA MEMORIA DINÁMICAMENTE*/

        thread_args_drone * arg = malloc(sizeof(thread_args_drone));

        if (arg == NULL){
            printf("Failed allocating dynamic memory!");
            return;
        }

        arg->drone=array_of_drones[i];
        arg->array_of_targets = array_of_targets;
        arg->num_of_targets = num_of_targets;

        arr_of_args_drone[i] = arg;

        pthread_create(&array_of_threads[i], thread_drone_attr, drone_damage_targets, arr_of_args_drone[i]);
    }
    
}

void join_threads (pthread_t * array_of_threads, int num_of_threads){
    /**
     * Por cada uno de los elementos en un arreglo de hilos, ejecutar la función pthread_join() para que el proceso espere a que los hilos terminen ejecución.
     */
    for (int i = 0; i < num_of_threads; i++){
        pthread_join(array_of_threads[i], NULL);
    }
}

void kill_threads(){
    /**
     * Elimina el bloque de atributos de los hilos del arreglo de hilos ejecutando la función pthread_attr_destroy.
     */
}

int main(void){
    
    int num_of_drones;
    int num_of_targets;

    drone array_of_drones[num_of_drones];
    target array_of_targets[num_of_targets];
    
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

            printf("x: %d\n",  coord_x);
            printf("Y: %d\n", coord_y);
            printf("Resistance: %d\n", resistance);

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

            printf("Drones: %d\n", num_of_drones);

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

            printf("x: %d\n",  coord_x);
            printf("Y: %d\n", coord_y);
            printf("Radius: %d\n", radius);
            printf("Power: %d\n", power);

        }


        line_counter++;
    }   

    // Close file
    fclose(txt_file);

    if(pthread_mutex_init(&available, NULL) != 0){
        fprintf(stderr, "Couldn't initialize mutex\n");
        return 1;
    }

    pthread_t array_of_threads[3];

    drone dron1 = {1,2,2,1,1};
    array_of_drones[0] = dron1;

    drone dron2 = {1,2,2,3,2};
    array_of_drones[1] = dron2;

    drone dron3 = {1,2,2,3,3};
    array_of_drones[2] = dron3;

    target om1 = {6,8,5,1,false,0};
    array_of_targets[0] = om1;
    target om2 = {2,0,1,2,false,0};
    array_of_targets[1] = om2;

    target oc1 = {7,7,5,3,false,1};
    array_of_targets[2] = oc1;

    target oc2 = {1,3,3,4,false,1};
    array_of_targets[3] = oc2;

    pthread_attr_t thread_drone_attr;

    pthread_attr_init(&thread_drone_attr);

    create_threads(array_of_threads, &thread_drone_attr, array_of_drones, arr_of_args_drone, array_of_targets, num_of_drones, num_of_targets);
    join_threads(array_of_threads, num_of_drones);
    //FREE DYNAMICALLY ALLOCATED MEMORY FOR ARGUMENTS IN ARRAY OF ARGUMENTS (DRONES AND TARGETS)
    for (int i = 0; i < num_of_drones; i++){
        free(arr_of_args_drone[i]);
    }
    //FREE DYNAMICALLY ALLOCATED MEMORY FOR ARGUMENTS IN ARRAY OF ARGUMENTS (DRONES AND TARGETS)
    pthread_attr_destroy(&thread_drone_attr);

    /*PRUEBA*/
    printf("\n");
    for (int i = 0; i < num_of_targets; i++){
        printf("target en posicion (%d,%d) tiene health %d \n",array_of_targets[i].x, array_of_targets[i].y, array_of_targets[i].health);
    }

    return 0;
}