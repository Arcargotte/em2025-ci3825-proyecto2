#include <stdio.h>
#include <pthread.h>

#define NUM_THREADS 3

void* tarea_intensiva(void* arg) {
    long id = (long)arg;
    printf("Hilo %ld iniciado\n", id);
    double x = 0.0;
    // Trabajo intensivo en CPU
    for (long i = 0; i < 1000000000; i++) {
        x += (double)i / (i + 1); // Cálculo simple pero pesado
    }
    printf("Hilo %ld terminado, resultado: %f\n", id, x);
    return NULL;
}

int main() {
    pthread_t hilos[NUM_THREADS];
    for (long i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&hilos[i], NULL, tarea_intensiva, (void*)i) != 0) {
            perror("Error al crear hilo");
            return 1;
        }
    }

    // Espera a que los hilos terminen
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(hilos[i], NULL);
    }
    printf("Todos los hilos han terminado\n");
    return 0;
}