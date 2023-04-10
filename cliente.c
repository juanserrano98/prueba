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
  struct sockaddr *dserv;
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

void procesa_argumentos(int argc,char *argv[])
{
    // A RELLENAR
     if (argc < 6)
    {
        fprintf(stderr,"Forma de uso: %s ip_syslog puerto_syslog es_stream nhilos fich_eventos", argv[0]);
        exit(1);
    }
    ip_syslog = argv[1];
    puerto_syslog = atoi(argv[2]);
    //es_stream = *argv[3];
    nhilos = atoi(argv[4]);
    fich_eventos = argv[5];


    //valido numero puerto
    if ((puerto_syslog < 1024) || (puerto_syslog > 65535))
    {
        fprintf(stderr, "Error: El puerto debe ser mayor o igual a 1024 y menor o igual que 65535\n");
        exit(2);
    }
    if (*argv[3] == 'u')
    {
    es_stream = FALSO;
    }
    else if (*argv[3] == 't')
    {
    es_stream = CIERTO;
    }
    if(nhilos < 1)
    {
        fprintf(stderr,"Error: el tamaño de la cola ha de ser superior a 0\n");
        exit(4);
    }
    if(ip_syslog == NULL)
    {
        fprintf(stderr,"Error: se necesita al menos un hilo para atender peticiones\n");
        exit(5);
    }
    if(fich_eventos == NULL)
    {
        fprintf(stderr,"Error: este fichero no puede ser vacío\n");
        exit(6);
    }
}

void salir_bien(int s)
{
  fclose(fp);
  exit(0);
}

void *hilo_lector(datos_hilo *p)
{
  int  enviados;
  char buffer[TAMLINEA];
  char *s;
  int sock_dat;

  do
  {
      bzero(buffer,TAMLINEA);
      // Leer la siguiente linea del fichero con fgets
      // (haciendo exclusión mutua con otros hilos)
      // El fichero (ya abierto por main) se recibe en uno de los parámetros
      // A RELLENAR -----------------
      pthread_mutex_t *c;
      pthread_mutex_lock(c);
      s = fgets(buffer, TAMLINEA, p->fp);
      pthread_mutex_unlock(c);

      if (s!=NULL)
      {
          // La IP y puerto del servidor están en una estructura sockaddr_in
          // que se recibe en uno de los parámetros
          if (es_stream)  // Enviar la línea por un socket TCP
          {
            // A RELLENAR
             sock_dat = socket(AF_INET, SOCK_STREAM, 0);
            if (sock_dat < 0) {
              perror("Error creando socket TCP");
              exit(EXIT_FAILURE);
            }

            if (connect(sock_dat, (struct sockaddr *)&p->fp, sizeof(p->fp)) < 0) {
              perror("Error conectando con el servidor");
              exit(EXIT_FAILURE);
            }

            enviados = send(sock_dat, buffer, strlen(buffer), 0);
            if (enviados < 0) {
              perror("Error enviando datos");
              exit(EXIT_FAILURE);
            }

          }
          else // Enviar la línea por un socket UDP
          {
            // A RELLENAR
             sock_dat = socket(AF_INET, SOCK_DGRAM, 0);
            if (sock_dat < 0) {
              perror("Error creando socket UDP");
              exit(EXIT_FAILURE);
            }

            enviados = sendto(sock_dat, buffer, strlen(buffer), 0,
              (struct sockaddr *)&p->fp, sizeof(p->fp));
            if (enviados < 0) {
              perror("Error enviando datos");
              exit(EXIT_FAILURE);
            }
          }
          close(sock_dat);
          printf("%s",s); // Para depuración, imprimios la línea que hemos enviado
      }
  }
  while(s); // Mientras no se llegue al final del fichero
}


void main(int argc, char * argv[])
{
    // La función main crea los hilos lector, pasándoles los parámetros necesarios,
    // y espera a que terminen

    register int i;

    pthread_t *th;
    datos_hilo q;

    int sock_dat, enviados;
    struct sockaddr_in d_serv;

    socklen_t ldir;
    char buffer[50];

    // Instalar la rutina de tratamiento de la señal SIGINT
    // A RELLENAR
    signal(SIGINT, salir_bien);

    // Procesar los argumentos de la línea de comandos
    procesa_argumentos(argc,argv);

    printf("IP servidor %s, es_stream=%d\n",ip_syslog,es_stream);
    if ((fp=fopen(fich_eventos,"r"))==NULL)
    {
      perror("Error al abrir el fichero de eventos");
      exit(6);
    }

    // creamos espacio para los objetos de datos de hilo
    // A RELLENAR
    th = (pthread_t *) malloc(nhilos*sizeof(pthread_t));
    if (th == NULL) {
        perror("Error al reservar espacio para los objetos de datos de hilo");
        exit(7);
    }

    // incicializamos los datos que le vamos a pasar como parámetro a los hilo_lector
    // (se pasa a todos el mismo parámetro)
    // A RELLENAR
    q.fp = fp;
    //q.dserv = d_serv;

    for (i=0;i<nhilos;i++)
    {
      // lanzamos el hilo lector
      // A RELLENAR
       if (pthread_create(&th[i], NULL, (void *) hilo_lector, (void *) &q) != 0)
      {
        perror("Error al crear el hilo lector");
        exit(EXIT_FAILURE);
      }
    }

    // Una vez lanzados todos, hacemos un join sobre cada uno de ellos
    for (i=0;i<nhilos;i++)
    {
      pthread_join(th[i],NULL);
    }
    // Al llegar aquí, todos los hilos han terminado
    fclose(fp);
}
