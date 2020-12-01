#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <string.h>
#include <dirent.h>
#include <sys/inotify.h>

#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_EVENT_MONITOR 2048
#define NAME_LEN 32
#define MONITOR_EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_LEN MAX_EVENT_MONITOR*(MONITOR_EVENT_SIZE+NAME_LEN)
#define PATH "/home/fernando/Documents/ProyectoFinal/test/"

FILE *fp;

void enviar(char mensaje[100], int fd){
    fp = fopen("Modificaciones.txt", "a+");
    fprintf(fp,"LLAMADA AL SERVIDOR:%s\n", mensaje);
    fclose(fp);
    send(fd, mensaje, strlen(mensaje), 0);
    sleep(10);
}

void listFiles(const char *path, int fdc){
    struct dirent *dp;
    DIR *dir = opendir(path);
    int len = 0;
    char new_path[100];

    if(!dir){
        return;
    }
    while ((dp = readdir(dir)) != NULL)
    {
        memset(new_path, '\0', sizeof new_path);
        len = strlen(dp->d_name);
        if(strlen(dp->d_name) > 4){
            if(dp->d_name[len - 1] == 'q' && dp->d_name[len - 2] == 'e' && dp->d_name[len - 3] == 's' && dp->d_name[len - 4] == '.'){
                printf("%s%s\n", path, dp->d_name);
                strcat(new_path, path);
                strcat(new_path, dp->d_name);
                fp = fopen("Modificaciones.txt", "a+");
                fprintf(fp,"%s\n", new_path);
                fclose(fp);
                enviar(new_path, fdc);
                memset(new_path, '\0', sizeof new_path);
            }
        }
    }
    closedir(dir);
}

static void daemonize(){
    pid_t pid;

    pid = fork();

    if (pid < 0){
        exit(EXIT_FAILURE);
    }

    if (pid > 0){
        exit(EXIT_SUCCESS);
    }

    if (setsid() < 0){
        exit(EXIT_FAILURE);
    }

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0){
        exit(EXIT_FAILURE);
    }

    if (pid > 0){
        exit(EXIT_SUCCESS);
    }

    umask(027);

    chdir("/home/fernando/Documents/ProyectoFinal");

    int x;

    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--){
        close(x);
    }

    openlog("firstdaemon", LOG_PID, LOG_DAEMON);
}

int main(){
    printf("Starting daemonize\n");
    daemonize();

    //Creaciond el Cliente para poder enviar mensajes al servidor
    struct sockaddr_in serv;
    int fdc;
    int conn;
    char message[100] = "";

    fdc = socket(AF_INET, SOCK_STREAM, 0);
    serv.sin_family = AF_INET;
    serv.sin_port = htons(8096);

    inet_pton(AF_INET, "127.0.0.1", &serv.sin_addr);

    connect(fdc, (struct sockaddr *)&serv, sizeof(serv));


    // Inicia el monitores de la carpeta
    fp = fopen("Modificaciones.txt", "a+");
    fprintf(fp,"Capturando Archivos\n");
    fclose(fp);
    int fd, watch_desc, tam;
    char buffer[BUFFER_LEN], new_path[100];
    fd = inotify_init();
    if (fd < 0){
        fp = fopen("Modificaciones.txt", "a+");
        fprintf(fp,"Notify No se inicializo\n");
        fclose(fp);
        perror("Notify no se incializo");
    }
    watch_desc = inotify_add_watch(fd, PATH, IN_MOVED_TO);
    printf("BUenas %d \n", watch_desc);
    if(watch_desc < 0){
        printf("No se puede monitorear: %s", "/home/fernando/Documents/ProyectoFinal/test");
        fp = fopen("Modificaciones.txt", "a+");
        fprintf(fp,"No se puede monitorear\n");
        fclose(fp);
    }
    else{
        printf("Monitoreando: %s", "/home/fernando/Documents/ProyectoFinal/test");
        fp = fopen("Modificaciones.txt", "a+");
        fprintf(fp,"Monitoreando\n");
        fclose(fp);
    }
    printf("\n");
    int len_temp = 0;
    int i = 0;
    tam = 1;
    while(1){
        i = 0;
        int total_read = read(fd, buffer, BUFFER_LEN);
        if (total_read < 0){
            perror("Read error");
        }
        while(i < total_read){
            struct inotify_event *event = (struct inotify_event*)&buffer[i];
            if(event->len){
                if(event->mask & IN_MOVED_TO){
                    printf("IN MOVED TO");
                    if(event->mask & IN_ISDIR){
                        strcat(new_path, PATH);
                        strcat(new_path, event->name);
                        strcat(new_path, "/");
                        listFiles(new_path, fdc);
                        memset(new_path, '\0', sizeof new_path);

                    }else{
                        len_temp = strlen(event->name);
                        if(event->name[len_temp-4] == '.' && event->name[len_temp-3] == 's' && event->name[len_temp-2] == 'e' && event->name[len_temp-1] == 'q'){
                            printf("File \"%s\" was created", event->name);
                            strcat(new_path, PATH);
                            strcat(new_path, event->name);
                            fp = fopen("Modificaciones.txt", "a+");
                            fprintf(fp,"%s\n", new_path);
                            fclose(fp);
                            enviar(new_path, fdc);
                            memset(new_path, '\0', sizeof new_path);
                        }
                    }
                }
                printf("\n");
                i += MONITOR_EVENT_SIZE + event->len;
            }
        }
    }
    inotify_rm_watch(fd, watch_desc);
    close(fd);
    fclose(fp);
    syslog(LOG_NOTICE, "First daemon terminated.");
    closelog();
    return EXIT_SUCCESS;
}