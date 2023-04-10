#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "util.h"

// Función de utilidad que determina si los caracteres de una cadena son todos numericos
int valida_numero(char *str) {
    // A RELLENAR
        if(!isdigit(str)) {
            printf("El caracter ha de ser un numero");
            exit(1);
        }
}

// Función de utilidad que valida si una cadena de caracteres representa una IPv4 valida
int valida_ip(char *ip)
{
    // Comprueba si la cadena contiene una ip válida
    // A RELLENAR
    //Si todo va bien, devuelve 0, si no, 1
     int i, j, num, len;
    int nums[4];
    char *endptr;

    // Comprobamos si la cadena es nula o vacía
    if (!ip || ip[0] == '\0') {
        perror("La IP esta vacía");
        exit(1);
    }

    // Separo la cadena en puntos comprobando a la vez que tenga 4 elementos
    i = 0;
    j = 0;
    len = strlen(ip);
    while (j < len) {
        if (ip[j] == '.') {
            if (i == 3) {
                exit(1);
            }
            nums[i++] = num;
            num = 0;
            j++;
        } else if (isdigit(ip[j])) {
            num = num * 10 + (ip[j] - '0');
            j++;
        } else {
            exit(1);
        }
    }
    if (i != 3) {
        perror("Error en la IP");
        exit(1);
    }
    nums[i] = num;

    // Comprobamos que cada elemento esté entre 0 y 255
    for (i = 0; i < 4; i++) {
        if (nums[i] < 0 || nums[i] > 255) {
            perror("El numero esta fuera del rango entre 0 y 255");
            exit(1);
        }
    }

    // Comprobamos que la cadena completa se ha convertido correctamente a números
    num = strtol(ip, &endptr, 10);
    if (num < 0 || num > 255 || *endptr != '\0') {
        exit(1);
    }

    return 1;
}

// Función de utilidad, para generar los tiempos aleatorios entre un
// min y un max
double randRange(double min, double max)
{
  return min + (rand() / (double) RAND_MAX * (max - min + 1));
}


// Función de utilidad para depuración. Emite por pantalla el mensaje
// que se le pasa como parámetro, pero pone delante del mensaje un
// timestamp, para poder ordenar la salida por si saliera desordenada
//
// Ejemplo de uso:
//
//  log_debug("Mensaje a mostrar por pantalla")
//
void log_debug(char *msg){
  struct timespec t;
  clock_gettime(_POSIX_MONOTONIC_CLOCK, &t);
  printf("[%ld.%09ld] %s", t.tv_sec, t.tv_nsec, msg);
}
