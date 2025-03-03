#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/wait.h>

#define SIZE 5
#define NUM_CHILDREN 3  // Número de hijos

// Definir estructura compartida
typedef struct {
    int id;
    int value;
} Data;

int main() {
    // Crear memoria compartida para un arreglo de structs
    Data * shared_array = (Data *)mmap(NULL, SIZE * sizeof(Data),
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_array == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Inicializar el arreglo en memoria compartida
    for (int i = 0; i < SIZE; i++) {
        shared_array[i].id = i;
        shared_array[i].value = i * 10;
    }

    // Crear múltiples procesos hijos
    for (int i = 0; i < NUM_CHILDREN; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        if (pid == 0) {  // Código del hijo
            printf("Hijo %d modificando la estructura...\n", i);
            for (int j = i; j < SIZE; j += NUM_CHILDREN) {  // Cada hijo modifica ciertas posiciones
                shared_array[j].value += (i + 1) * 5;
                printf("Hijo %d: shared_array[%d].value = %d\n", i, j, shared_array[j].value);
            }
            fflush(stdout);  // Forzar salida inmediata
            exit(0);
        }
    }

    // Padre espera a TODOS los hijos
    while (wait(NULL) > 0);

    // Mostrar datos finales
    printf("\nPadre: Arreglo final después de todos los hijos:\n");
    for (int i = 0; i < SIZE; i++) {
        printf("shared_array[%d] = { id: %d, value: %d }\n", i, shared_array[i].id, shared_array[i].value);
    }

    // Liberar memoria compartida
    munmap(shared_array, SIZE * sizeof(Data));

    return 0;
}