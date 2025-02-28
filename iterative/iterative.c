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


int main(void){
    
    int num_of_drones = 3;
    int num_of_targets = 4;

    drone array_of_drones[num_of_drones];
    target array_of_targets[num_of_targets];

    drone dron1 = {1,2,2,1, 1};
    array_of_drones[0] = dron1;

    drone dron2 = {1,2,2,3, 2};
    array_of_drones[1] = dron2;

    drone dron3 = {1,2,2,3, 3};
    array_of_drones[2] = dron3;

    target om1 = {6,8,5,-1, false, 0};
    array_of_targets[0] = om1;
    target om2 = {2,0,1,-2, false, 0};
    array_of_targets[1] = om2;

    target oc1 = {7,7,5,3, false, 1};
    array_of_targets[2] = oc1;

    target oc2 = {1,3,3,4, false, 1};
    array_of_targets[3] = oc2;

    for(int i = 0; i < num_of_drones; i++){
        for(int j = 0; j < num_of_targets; j++){
            if(array_of_targets[j].destroyed == false){
                computes_damage(array_of_drones[i], &array_of_targets[j]);
            }
        }
    }
    
    /*PRUEBA*/
    printf("\n");
    for (int i = 0; i < num_of_targets; i++){
        printf("target en posicion (%d,%d) tiene health %d \n",array_of_targets[i].x, array_of_targets[i].y, array_of_targets[i].health);
    }

    return 0;
}