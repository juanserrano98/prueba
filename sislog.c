#include <stdio.h>
#include <string.h>
#include <stdlib.h>//De aqui saco el exit_failure, devuelve distinto de 0 como se nos comentó
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <sched.h>

// Archivos de cabecera para manipulación de sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <time.h>

#include "cola.h"
#include "util.h"

#define CIERTO           1
#define FALSO            0

#define NUMFACILITIES   10
#define NUMLEVELS        8

// Estructura de datos para pasar los parametros a los hilos de atencion
struct param_hilo_aten{
    int num_hilo;
    int s;
};

typedef struct param_hilo_aten param_hilo_aten;

// ====================================================================
// PROTOTIPOS FUNCIONES
// ====================================================================
static void handler(int signum); // Manejador de señal SIGINT

// ====================================================================
// VARIABLES GLOBALES
// ====================================================================

// Cola para sincronizar los hilos de atencion de peticiones con los
// hilos trabajadores
Cola cola_eventos;

// Puerto en el que esperamos los mensajes
int puerto;

// Variable booleana que indica si el socket es orientado a conexión o no
unsigned char es_stream=CIERTO;

// Variable que almacena el numero de hilos de atencion de peticiones
int num_hilos_aten;

// Variable que almacena el numero de hilos trabajadores
int num_hilos_work;

// Puntero a la dirección de comienzo del array de datos de hilo
// de los hilos de atencion de peticiones
pthread_t *hilos_aten;

// Puntero a la dirección de comienzo del array de datos de hilo
// de los hilos trabajadores
pthread_t *hilos_work;

// Arrays para la traducción de nombres de niveles y de facilities
// y para obtener los nombres de los ficheros de registro
char * facilities_names[NUMFACILITIES]={
    "kern",
    "user",
    "mail",
    "daemon",
    "auth",
    "syslog",
    "lpr",
    "news",
    "uucp",
    "cron"};

char * level_names[NUMLEVELS]={
    "emerg",
    "alert",
    "crit",
    "err",
    "warning",
    "notice",
    "info",
    "debug"};

char * facilities_file_names[NUMFACILITIES]={
    "fac00.dat",
    "fac01.dat",
    "fac02.dat",
    "fac03.dat",
    "fac04.dat",
    "fac05.dat",
    "fac06.dat",
    "fac07.dat",
    "fac08.dat",
    "fac09.dat"};

// mutex de exclusion a los ficheros de registro
pthread_mutex_t mfp[NUMFACILITIES];


// Tamaño de la cola circular
int tam_cola;

// ====================================================================
// FUNCION handler de las señales recibidas por el hilo buque
// ====================================================================
static void handler(int signum)
{
    register int i;

    switch (signum){
        case SIGINT:
            // Si se recibe esta señal, hay que terminar "bien"
            // liberando todos los recursos en uso antes de finalizar
            // A RELLENAR
            //Destruir cola:
            destruir_cola(&cola_eventos);
            //Destruir hilos
            free(hilos_aten);
            free(hilos_work);
            //Destruir mutex
            if (pthread_mutex_destroy(&mfp[NUMFACILITIES]) < 0)
        {
            perror("Error en la destruccion del mutex mfp\n");
            exit(EXIT_FAILURE);
        }

            printf("ERROR: programa terminado");
            exit(0);
        default:
            pthread_exit(NULL);
    }
}

void procesa_argumentos(int argc,char *argv[])
{
    // A RELLENAR
     if (argc < 6)
    {
        fprintf(stderr,"Forma de uso: %s num_puerto t|u tam_cola num_hilos_aten num_hilos_work\n", argv[0]);
        exit(1);
    }
    puerto = atoi(argv[1]);
//    es_stream = *argv[2];
    tam_cola = atoi(argv[3]);
    num_hilos_aten = atoi(argv[4]);
    num_hilos_work = atoi(argv[5]);


    //valido numero puerto
    if ((puerto < 1024) || (puerto > 65535))
    {
        fprintf(stderr, "Error: El puerto debe ser mayor o igual a 1024 y menor o igual que 65535\n");
        exit(2);
    }
    if (*argv[2] == 'u')
    {
    es_stream = FALSO;
    }
    else if (*argv[2] == 't')
    {
    es_stream = CIERTO;
    }
    else
    {
        fprintf(stderr,"Error, es_stream solo puede valer u o t");
        exit(3);
    }

    if(tam_cola < 1)
    {
        fprintf(stderr,"Error: el tamaño de la cola ha de ser superior a 0\n");
        exit(4);
    }
    if(num_hilos_aten < 1)
    {
        fprintf(stderr,"Error: se necesita al menos un hilo para atender peticiones\n");
        exit(5);
    }
    if(num_hilos_work < 1)
    {
        fprintf(stderr,"Error: se necesita al menos un hilo que procese las peticiones\n");
        exit(6);
    }
}

// ====================================================================
// Implementación de los hilos
// ====================================================================

void *Worker(int *id)
{
    int id_worker;
    FILE *fp;
    dato_cola *evt;
    char msg[2048];
    char *fechahora;
    time_t timeraw;

    // Hacemos copia del parámetro recibido
    id_worker=*id;

    // y liberamos la memoria reservada para él
    free(id);

    // Mostramos información de depuración por pantalla
    sprintf(msg, "Comienza el Worker %d\n", id_worker);
    log_debug(msg);

    // Codigo del worker. Espera datos de la cola de sincronización,
    // genera en base a ellos la línea a escribir, y la escribe
    // en el fichero que corresponda. Mira "cola.h"
    // para recordar la estructura dato_cola que recibe de la cola
    while (1)
    {
        // A RELLENAR
        // Obtener un dato de la cola de sincronización
        evt = obtener_dato_cola(&cola_eventos);

        // Obtener la fecha y hora actual
        timeraw = time(NULL);
        fechahora = ctime(&timeraw);
        fechahora[strlen(fechahora) - 1] = '\0'; // Asi quito el salto de línea

        // Generar la línea a escribir en el archivo
        sprintf(msg, "%s:%s:%s:%s", facilities_names[evt->facilidad], level_names[evt->nivel],fechahora, evt->msg);

        pthread_mutex_lock(&mfp[evt->facilidad]);
        // Escribir la línea en el archivo correspondiente
        fp = fopen(facilities_file_names[evt->facilidad],"a");
        fprintf(fp,"%s",msg);
        fclose(fp);
        pthread_mutex_unlock(&mfp[evt->facilidad]);
                
        fflush(fp);

    }
}

void * AtencionPeticiones (param_hilo_aten *q)
{
   int sock_dat, recibidos;
   struct sockaddr_in d_cliente;
   socklen_t l_dir=sizeof(d_cliente);
   char msg[100];
   char buffer[TAMMSG];
   char * token;
   char * loc;
   dato_cola *p;
   int s;  // Variable local para almacenar el socket que se recibe como parámetro
   int num_hilo; // Variable local para almacenar el numero de hilo que se recibe como parámetro
                 // (no usada, pero puedes usarla para imprimir mensajes de depuración)

   // Información de depuración
   sprintf(msg, "Comienza el Hilo de Atencion de Peticiones %d\n", q->num_hilo);
   log_debug(msg);

   // Hacemos copia de los parámetros recibidos
   s = q->s;
   num_hilo = q->num_hilo;

   // y liberamos la memoria reservada para el parámetro
   free(q);

   while (1) // Bucle infinito de atencion de mensajes
   {
        // Primero, se recibe el mensaje del cliente. Cómo se haga depende
        // de si el socket es orientado a conexión o no
        if (es_stream) // TCP
        {
            // Aceptar el cliente, leer su mensaje hasta recibirlo entero, y cerrar la conexión
            // A RELLENAR
            listen(s,SOMAXCONN);
            sock_dat=accept(s, (struct sockaddr *) &d_cliente, &l_dir);//llamada al sistema que acepta la conexion
            if (sock_dat==-1) 
            {
                perror("Eror al aceptar el cliente");
                exit(1);
            }
            recibidos = recv(sock_dat, buffer,TAMMSG,0);
            if (s == -1) 
            {
                perror("Error al leer el mensaje del cliente");
                exit(1);
            }
            close(recibidos);
        }        
        else // UDP
        {
            // Recibir el mensaje del datagrama
            // A RELLENAR
            if ((recibidos = recvfrom(s, buffer, TAMMSG, 0,
        (struct sockaddr *) &d_cliente, &l_dir)) < 0)
            {
            perror("Error esperando el mensaje\n");
            exit(EXIT_FAILURE);
            }
        }
        // Una vez recibido el mensaje, es necesario separar sus partes,
        // guardarlos en la estructura adecuada, y poner esa estructura en la cola
        // de sincronización.
        // A RELLENAR
        //Primero tokenizo los mensajes para separarlos:
        // Tokenizar mensaje
        char* saveptr = NULL;
        int tokfacilidad, toknivel;
        char* tokmsg;
        char* token = strtok_r(buffer, ":", &saveptr);
        tokfacilidad = atoi(token);
        if(token == NULL) {
            perror("Error en el token facilidad\n");
            exit(EXIT_FAILURE);
        }      

        token = strtok_r(NULL, ":", &saveptr);
        if(token == NULL) {
            perror("Error en el token nivel\n");
            exit(EXIT_FAILURE);
        }
        toknivel = atoi(token);

        token = strtok_r(NULL, "", &saveptr);
        if(token == NULL) {
           perror("Error en al tokenizar el mensaje\n");
            exit(EXIT_FAILURE);
        }
        tokmsg = token;

        // Añadir mensaje a cola circular
        p = (dato_cola*)malloc(sizeof(dato_cola));
        p->facilidad = tokfacilidad;
        p->nivel = toknivel;
        strcpy(p->msg, tokmsg);
        insertar_dato_cola(&cola_eventos,p);        
       
   }
}


// ====================================================================
// PROGRAMA PRINCIPAL
// ====================================================================

// Su misión es crear e inicializar los recursos de sincronización globales,
// lanzar todos los hilos y esperar a que finalicen

int main(int argc,char *argv[])
{
    register int i;   // Indice para bucles
    int *id;          // Para pasar el identificador a cada hilo trabajador
    int sock_pasivo;
    struct sockaddr_in d_local;
    param_hilo_aten *q;

    procesa_argumentos(argc,argv);

    setbuf(stdout,NULL); // quitamos el buffer de la salida estandar
    signal(SIGINT, handler); // establecemos el comportamiento ante la llegada asíncrona de la señal

    // Datos para asignar puerto al socket
    d_local.sin_family= AF_INET;
    d_local.sin_addr.s_addr=htonl(INADDR_ANY);
    d_local.sin_port=htons(puerto);

    if (es_stream) // Preparar socket TCP
    {
        // A RELLENAR
        sock_pasivo = socket(PF_INET, SOCK_STREAM, 0);
        if(sock_pasivo == -1) 
        {
            perror("Error al crear socket TCP");
            exit(1);
        }
        sock_pasivo = listen(sock_pasivo,SOMAXCONN);
        if (sock_pasivo == -1)
        {
            perror("Error al poner en modo escucha el socket TCP");
            exit(1);
        }
        printf("Creé el socket! TCP\n");
    }
    else // Preparar socket UDP
    {
        // A RELLENAR
        sock_pasivo = socket(PF_INET, SOCK_DGRAM, 0);
        if(sock_pasivo == -1) 
        {
            perror("Error al crear socket UDP");
            exit(1);
        }
        printf("Creé el socket! UDP\n");
    }

    // Asignamos el puerto al socket
    // A RELLENAR
    sock_pasivo = bind(sock_pasivo, (struct sockaddr *) &d_local, sizeof(d_local));
        if (sock_pasivo == -1)
        {
            perror("Error al asignar puerto al socket");
            exit(1);
        }

    // creamos el espacio para los objetos de datos de hilo
    hilos_aten=(pthread_t *)malloc(num_hilos_aten*sizeof(pthread_t));

    // Inicializamos los mutex de exclusión a los ficheros de log
    // en que escribirán los workers
    // A RELLENAR
    if(pthread_mutex_init(&mfp[NUMFACILITIES],NULL)!=0){
    perror("Error al inicializar el mutex.\n");
  }

    // Reservamos espacio para los objetos de datos de hilo de los hilos trabajadores
    hilos_work=(pthread_t *)malloc(num_hilos_work*sizeof(pthread_t));

    // Inicializamos la cola
    // A RELLENAR
    inicializar_cola(&cola_eventos,tam_cola);


    // Creamos cada uno de los hilos de atención de peticiones
    for (i=0;i<num_hilos_aten;i++)
    {
        // A RELLENAR
        id = (int *) malloc (sizeof(int));
        *id = i;
        if (pthread_create(&hilos_work[i],NULL,(void*) Worker, (void *) id) < 0)
        {
            fprintf(stderr, "Error creando el hilo de atención de peticiones\n");
            exit(EXIT_FAILURE);
        }
    }
    // Y creamos cada uno de los hilos trabajadores
    for (i=0;i<num_hilos_work;i++)
    {
        // A RELLENAR
        id = (int *) malloc (sizeof(int));
        *id = i;
        if (pthread_create(&hilos_work[i],NULL,(void*) Worker, (void *) id) < 0)
        {
            fprintf(stderr, "Error creando el hilo trabajador\n");
            exit(EXIT_FAILURE);
        }
    }
    printf("Cree todo!\n");
    // Esperamos a que terminen todos los hilos
    for (i=0;i<num_hilos_aten;i++)
    {
        pthread_join(hilos_aten[i],NULL);
    }
    for (i=0;i<num_hilos_work;i++)
    {
        pthread_join(hilos_work[i],NULL);
    }
}
