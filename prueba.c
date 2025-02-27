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
        target->health = target->health - drone.damage;
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

void create_threads (pthread_t * array_of_threads, pthread_attr_t * thread_drone_attr, drone * array_of_drones, thread_args_drone ** arr_of_args_drone, target * array_of_targets, thread_args_target * arr_of_args_target, int num_of_drones, int num_of_targets){
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

        printf("%d\n", arg->drone.id);

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

int main(int argc, char *argv[]){
    
    int num_of_drones = 3;
    int num_of_targets = 4;

    pthread_t array_of_threads[3];
    thread_args_target arr_of_args_target[num_of_targets];

    drone array_of_drones[num_of_drones];
    target array_of_targets[num_of_targets];

    drone dron1 = {1,2,2,1, 1};
    array_of_drones[0] = dron1;

    drone dron2 = {1,2,2,3, 2};
    array_of_drones[1] = dron2;

    drone dron3 = {1,2,2,3, 3};
    array_of_drones[2] = dron3;

    target om1 = {6,8,5, 1};
    array_of_targets[0] = om1;
    target om2 = {2,0,1,2};
    array_of_targets[1] = om2;

    target oc1 = {7,7,5,3};
    array_of_targets[2] = oc1;

    target oc2 = {1,3,3,4};
    array_of_targets[3] = oc2;

    pthread_t tid_drone1;
    array_of_threads[0] = tid_drone1;

    pthread_t tid_drone2;
    array_of_threads[1] = tid_drone2;

    pthread_t tid_drone3;
    array_of_threads[2] = tid_drone3;

    pthread_attr_t thread_drone_attr;

    pthread_attr_init(&thread_drone_attr);

    create_threads(array_of_threads, &thread_drone_attr, array_of_drones, arr_of_args_drone, array_of_targets, arr_of_args_target, num_of_drones, num_of_targets);
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