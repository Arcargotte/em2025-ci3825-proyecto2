#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

bool generateInstanceProblem(char *fileName){

    FILE *txt_file;
    // Opens file with read function
    txt_file = fopen(fileName, "r");

    // In case the file couldn't open
    if (txt_file == NULL) {
        return false;
    }

    // Reads the content of the file line by line
    int line_counter = 1;
    char linea[999];
    while (fgets(linea, sizeof(linea), txt_file) != NULL) {
        
        // Get the memory space needed for path
        int q = 0;
        while(linea[q] != '\t'){
            q++;
        }

        // Allocating memory for path
        char *path = (char *)malloc(q + 1);

        // Saving first occurrence of tab '\t'
        int first_tab_occurr = q;

        // Iterating through a variable number of tabs until it finds the file type
        while(linea[first_tab_occurr] == '\t'){
            first_tab_occurr++;
        }
        char fileType = linea[first_tab_occurr];

        // 
        int j = 0;
        while(j < q){
            path[j] = linea[j];
            j++;
        }
        path[j] = '\0';

        if(fileType != 'D' && fileType != 'F'){
            printf("\x1b[31mError: in line \x1b[37m%d\x1b[31m: \x1b[37mNot valid file type!\n", line_counter);
            return 1;
        }  

        // Verify that first directory is correct
        if(strcmp(path, "/") != 0 && line_counter == 1){
            printf("\x1b[31mError: in line \x1b[37m1\x1b[31m: \x1b[37m Not valid first directory!\n");
            return false;
        }

        free(path);
        line_counter++;
    }   

    // Close file
    fclose(txt_file);

    return true;
}