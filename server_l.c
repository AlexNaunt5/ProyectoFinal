#include <stdio.h>
#include <string.h>
#include <stdlib.h> 
#include <omp.h>

#include <sys/socket.h>
#include <netinet/in.h>


//Se obtiene la cadena original para revisar
char cadena[12000000];

/**
 * @brief Revisa las posiciones en donde se conuentra cada una
 * de las lineas de adn en el orginal
 * 
 * @param secuencia 
 * @return int 
 */
int findPosicion(char secuencia[12000000]){
    int tamse = 0, tamca = 0, k, h;
    tamse = strlen(secuencia);
    tamca = strlen(cadena);
    for(int i = 0; i < tamse; i++){
        for(int j = 0; j < tamca; j++){
            if(secuencia[i] == cadena[j]){
                k=i;
                h=j;
                while((secuencia[k] == cadena[h]) && (k<tamse)){
                    h++;
                    k++;
                }
                if(k == tamse){
                    return j;
                }
            }
        }
        return -1;
    }
}

void Lector(char *path)
{
    //declaracion de todas las variables
    char *filename;
    int n, i, j;

    int contador_archivo = 0;
    char ch;

    char lineas[505][13000];
    char cadenas[505][13000];
    double cantsecuencia = 0.0, refrencia = 0.0, cantoriginal = 0.0;
    FILE *file;
    // //revisa y pone en un arreglo la informacion del archivo
    // //de las cadenas de adn
    // filename = "cadena2.txt";
    // file = fopen(filename, "r");
    i = 0;
    // while (fgets(linea2, 256, file))
    // {
    //     strcat(cadena, linea2);
    //     i++;
    // }
    // while((ch = fgetc(file)) != EOF)
    // {
    //     if(ch == '\n' || ch == '\t' || ch == '\r'){

    //     }else{
    //         cadena[contador_archivo] = ch;
    //         contador_archivo++;
    //     }

    // }
    cantoriginal = (strlen(cadena) - i);
    // fclose(file);

    //Se agrega el nombre del archivo por medio de linea de commando
    filename = path;
    n = 0;
    i = 0;
    file = fopen(filename, "r");
    // while (fgets(lineas[i], 256, file))
    // {
    //     lineas[i][strlen(lineas[i]) - 1] = '\0';
    //     i++;
    // }
    contador_archivo = 0;
    while((ch = fgetc(file)) != EOF){
        if(ch == ' '){

        }
        else if(ch == '\n'){
            i++;
            contador_archivo = 0;
        }else{
            lineas[i][contador_archivo] = ch;
            contador_archivo++;
        }
    }
    printf("I: %i\n", i);
    printf("TAM: %li\n", strlen(lineas[i]));
    if(strlen(lineas[i]) == 0){
        n = i;
        printf("S: %s\n", lineas[n-1]);
        lineas[n-1][strlen(lineas[n-1]) - 1] = '\0';
    }else
    {
        n = i + 1;
        lineas[i][strlen(lineas[n-1])] = '\0';
    }
    printf("N: %i\n", n);
    printf("\n\n");
    fclose(file);
    //Empieza programa en paralelo para leer y revisar las lineas
    for(i = 0; i < n-1; i++){
        lineas[i][strlen(lineas[i]) - 1] = '\0';
        // printf("Linea %i: %s\n", i, lineas[i]);
    }
    cadena[strlen(cadena)] = '\0';
    // printf("C: %c %li\n", cadena[367], strlen(cadena));
    
    for(i = 0; i < n; i++){
        printf("Linea %i: %li\n", i, strlen(lineas[i]));
    }
    
    int posiciones[n];
    // printf("%s\n\n", cadena);
    #pragma omp parallel shared(n)
        {
    #pragma omp for
            for (i = 0; i < n; i++){
                char *sequence = strchr(lineas[i], '\n');
                if(sequence){
                    *sequence = '\0';
                }
                // printf("Linea %i\n", i);
                // printf("Linea %s\n", lineas[i]);
                sequence = strstr(cadena, lineas[i]);
                // printf("S: %s", sequence);
                if(sequence != NULL){
                    //Se busca las posiones de los que se econtraron en la cadena original
                    posiciones[i] = findPosicion(lineas[i]);
                } else {
                    posiciones[i] = -1;
                }
            }
        } /*-- End of parallel region --*/

    //Se imprime las posiones de las cadenas que se encontraron
    int mapeadas = 0;
    int no_mapeadas = 0;
    for(i = 0, j = 0; i < n; i++){
        printf("\n");
        if(posiciones[i] != -1){
            j++;
            cantsecuencia += strlen(lineas[i]);
            printf("%s a partir del carcater %d", lineas[i], posiciones[i]);
            mapeadas++;
        }
        else{
            printf("%s no se econtro", lineas[i]);
            no_mapeadas++;
        }
    }
    
    //Se obtiene el porcentaje de refrencia
    // printf("SEC: %f ORG: %f\n", cantsecuencia, cantoriginal);
    refrencia = (cantsecuencia / cantoriginal) * 100;

    //Se manda los ultimos mensajes
    printf("\n El archivo cubre el %f del genoma de referencia", refrencia);

    printf("\n %d secuencias mapeadas", mapeadas);
    printf("\n %d secuencias no mapeadas\n", no_mapeadas);
    memset(lineas, 0, 505*13000*sizeof lineas[0][0]);
    
}

int main(){

    FILE *file;
    int contador_archivo = 0;
    char ch;
    //revisa y pone en un arreglo la informacion del archivo
    //de las cadenas de adn
    file = fopen("S._cerevisiae_processed.txt", "r");

    while((ch = fgetc(file)) != EOF)
    {
        if(ch == '\n' || ch == '\t' || ch == '\r'){

        }else{
            cadena[contador_archivo] = ch;
            contador_archivo++;
        }

    }

    fclose(file);
    printf("Cadena Cargada: %li\n", strlen(cadena));



    struct sockaddr_in serv;
    int fd;
    int conn;
    char message[100] = "";

    serv.sin_family = AF_INET;
    serv.sin_port = htons(8096);
    serv.sin_addr.s_addr = INADDR_ANY;

    fd = socket(AF_INET, SOCK_STREAM, 0);

    if(fd == -1){
        printf("Error al crear el socket");
        return(EXIT_FAILURE);
    }
    bind(fd, (struct sockaddr *)&serv, sizeof(serv));
    listen(fd, 5);

    while (conn = accept(fd, (struct sockaddr *)NULL, NULL))
    {
        int pid;
        if((pid = fork()) == 0){
            while (recv(conn, message, 100, 0) > 0) 
            {
                printf("Message Received: %s\n", message);
                Lector(message);
                memset(message, '\0', sizeof message);
            }
            exit(0);
            
        }
    }
    
}