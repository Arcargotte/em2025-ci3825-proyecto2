#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int n = 10;
int m = 10;

typedef struct drone drone;
struct drone{
    int x;
    int y;
    int radius;
    int damage;
};

typedef struct target target;
struct target{
    int x;
    int y;
    int health;
};

void computes_damage (drone drone, target target){
    // First Quadrant
    if (drone.x >= target.x && drone.y >= target.y){
        printf("(%d,%d)", drone.x - drone.radius, drone.y - drone.radius);
        if (drone.x - drone.radius <= target.x && drone.y - drone.radius <= target.y){
            printf ("It hits\n");
        }
    }
    
    //Second Quadrant
    else if (drone.x < target.x && drone.y > target.y){
        printf("(%d,%d)", drone.x + drone.radius, drone.y - drone.radius);

        if(drone.x + drone.radius >= target.x && drone.y - drone.radius <= target.y){
            printf("It hits\n");
        }
    }
    //Third Quadrant
    else if (drone.x > target.x && drone.y < target.y){
        printf("(%d,%d)", drone.x - drone.radius, drone.y + drone.radius);

        if(drone.x - drone.radius <= target.x && drone.y + drone.radius >= target.y){


            printf("It hits\n");
        }
    }
    //Fourth Quadrant
    else if (drone.x <= target.x && drone.y <= target.y){
        printf("(%d,%d)", drone.x + drone.radius, drone.y + drone.radius);

        if(drone.x + drone.radius >= target.x && drone.y + drone.radius >= target.y){

            printf("It hits\n");
        }
    }
}


int main(){
    
    int chart [n][m];

    int i = 0;

    /* PRUEBA  */

    while (i < n){
        int j = 0;
        while (j < m){
            chart[i][j] = 0;
            j++;
        }
        i++;
    }

    chart[6][8] = 5;
    chart[2][0] = 1;
    chart[7][7] = 5;
    chart[1][3] = 3;

    i = 0;
    while (i < n){
        int j = 0;
        while (j < m){
            printf("%d ", chart[i][j]);
            j++;
        }
        printf("\n");
        i++;
    }

    /*PRUEBA*/

    drone array_of_drones[1];
    target array_of_targets[4];

    drone dron1 = {1,2,2,1};
    array_of_drones[0] = dron1;

    target om1 = {6,8,5};
    array_of_targets[0] = om1;
    target om2 = {2,0,1};
    array_of_targets[1] = om2;

    target oc1 = {7,7,5};
    array_of_targets[2] = oc1;

    target oc2 = {1,3,3};
    array_of_targets[3] = oc2;

    for (int i = 0; i < 1; i++){
        for (int j = 0; j < 4; j++){

            computes_damage(array_of_drones[i], array_of_targets[j]);
        }
    }

    return 0;
}