// Archivos de cabecera para manipulación de sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <pthread.h>
#include <errno.h>
#include "util.h"

#define TAMLINEA  1024
#define FALSO   0
#define CIERTO  1
#define SINASIGNAR -1

// tipo de datos que recibiran los hilos lectores
struct datos_hilo{
  FILE *fp;
  pthread_mutex_t *dserv;
  struct sockaddr_in *addr;
};

typedef struct datos_hilo datos_hilo;


//
// VARIABLES GLOBALES
//

//IP del proceso syslog
char *ip_syslog;

//Puerto en el que espera el proceso syslog los
int puerto_syslog;

//Numero de hilos lectores
int nhilos;

//Es o no orientado a conexion
unsigned char es_stream=CIERTO;

// nombre del fichero fuente de eventos
char *fich_eventos;

// handler de archivo
FILE * fp;

pthread_mutex_t mutex;

void procesa_argumentos(int argc,char *argv[])
{
     if (argc < 6)
    {
        fprintf(stderr,"Forma de uso: %s ip_syslog puerto_syslog es_stream nhilos fich_eventos", argv[0]);
        exit(1);
    }
    ip_syslog = argv[1];
    puerto_syslog = atoi(argv[2]);
    es_stream = (strcmp(argv[3], "t") == 0) ? CIERTO : FALSO;
    nhilos = atoi(argv
