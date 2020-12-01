#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <string.h>

#define TAM 5


int main(){
    char palabras[TAM][100];
    char *sequence;
    char* palabra = "hola";
    char busqueda[100] = "Buscando la palabra hola";
    strcpy(palabras[0], "hola");
    strcpy(palabras[1], "hola");
    strcpy(palabras[2], "hola");
    strcpy(palabras[3], "hola");
    strcpy(palabras[4], "hola");

    printf("%s\n", palabras[0]);
    sequence = malloc(sizeof(char) * (strlen(palabras[0]) + 1));
    sequence = strstr(busqueda, palabras[0]);
    printf("%s\n", palabras[0]);
    if(sequence == NULL){

    }else
    {
        printf("Claro %s\n", sequence);
    }
    
    

    return 0;
}
