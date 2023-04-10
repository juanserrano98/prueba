#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>//De aqui saco el exit_failure, devuelve distinto de 0 como se nos comentó
#include <string.h>
#include "cola.h"

// El contenido de este fichero implementa las funciones de la cola.
// Es prácticamente igual a la cola que tienes hecha de las prácticas
// de laboratorio pero adaptándola a la estructura de datos dato_cola
// usada en este ejercicio.
//
// Mira el fichero cola.h para ver las estructuras de datos a utilizar

void inicializar_cola(Cola *cola, int tam_cola)
{
    // A RELLENAR
     cola->datos = (dato_cola**) malloc(tam_cola*sizeof(dato_cola**));

   if (cola->datos == NULL) {
      perror("Inicialización fallida de la cola.\n");
      exit(EXIT_FAILURE);
   }

   cola->head = 0;

   //mutex
   if (pthread_mutex_init(&(cola->mutex_head), NULL) < 0) {
      perror("Inicialización fallida del mutex_head.\n");
      exit(EXIT_FAILURE);
   }
   if (pthread_mutex_init(&(cola->mutex_tail), NULL) < 0) {
      perror("Inicialización fallida del mutex_tail.\n");
      exit(EXIT_FAILURE);
   }

    //semaforos
   if(sem_init(&(cola->num_huecos), 0, tam_cola) < 0){
      perror("Inicialización fallida del semaforo num_huecos.\n");
      exit(EXIT_FAILURE);
   }
   if(sem_init(&(cola->num_ocupados), 0, 0) < 0){
      perror("Inicialización fallida del semaforo num_ocupados.\n");
      exit(EXIT_FAILURE);
   }

   cola->tail = 0;

   cola->tam_cola = tam_cola;
}


void destruir_cola(Cola *cola)
{
    // A RELLENAR
   int tam_cola = cola->tam_cola;

   free(cola->datos);
   
   //mutex
   if (pthread_mutex_destroy(&(cola->mutex_head)) < 0) {
      perror("Deestruccion fallida del mutex.\n");
      exit(EXIT_FAILURE);
   }
   if (pthread_mutex_destroy(&(cola->mutex_head)) < 0) {
      perror("Desstruccion fallida del mutex.\n");
      exit(EXIT_FAILURE);
   }

   //semaforos
   if(sem_destroy(&(cola->num_ocupados)) < 0){
      perror("Destruccion fallida del semaforo num_ocupados\n");
      exit(EXIT_FAILURE);
   }

   if(sem_destroy(&(cola->num_huecos)) < 0){
      perror("Destruccion fallida del semaforo num_huecos.\n");
      exit(EXIT_FAILURE);
   }
}

void insertar_dato_cola(Cola *cola, dato_cola * dato)
{
    // A RELLENAR
    if(pthread_mutex_lock(&(cola->mutex_head)) < 0){
      perror("error bloqueando el mutex_head\n");
      exit(EXIT_FAILURE);
   }

   if(pthread_mutex_lock(&(cola->mutex_tail)) < 0){
      perror("error bloqueando el mutex_tail\n");
      exit(EXIT_FAILURE);
   }

   if(sem_wait(&(cola->num_huecos)) < 0){
      perror("error en semaforo num_huecos\n");
      exit(EXIT_FAILURE);
   }
   

   (cola->datos)[cola->head] = dato;
   (cola->head) = ((cola->head) + 1) % (cola->tam_cola);

   if(sem_post(&(cola->num_ocupados)) < 0){
      perror("error en semáforo num_ocupados\n");
      exit(EXIT_FAILURE);
   
   if(pthread_mutex_unlock(&(cola->mutex_head)) < 0){
      perror("error desbloqueando el cerrojo\n");
      exit(EXIT_FAILURE);
   }
   
   }
}


dato_cola * obtener_dato_cola(Cola *cola)
{
    // A RELLENAR
     if(sem_wait(&(cola->num_ocupados)) < 0){
      perror("error en semaforo num_ocupados\n");
      exit(EXIT_FAILURE);
   
    if(pthread_mutex_lock(&(cola->mutex_tail)) < 0){
      perror("error bloqueando el cerrojo\n");
      exit(EXIT_FAILURE);
   }
}

   //dato_cola* dato = (cola->datos)[cola->tail];
   dato_cola *p = (cola->datos)[cola->tail];
   (cola->tail) = ((cola->tail) + 1) % (cola->tam_cola); 

   if(sem_post(&(cola->num_huecos)) < 0){
      perror("error en semaforo num_huecos\n");
      exit(EXIT_FAILURE);
   }
   
   if(pthread_mutex_unlock(&(cola->mutex_head)) < 0){
      perror("error desbloqueando el cerrojo cabecero\n");
      exit(EXIT_FAILURE);
   }

    return(p);
}
