/*
 * modulo_consola.h
 *
 *  Created on: 16 may. 2022
 *      Author: utnso
 */

#ifndef MODULO_CONSOLA_H_
#define MODULO_CONSOLA_H_


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <client/utils.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <commons/config.h>

t_log* logConsola;
int socketKernel;
t_config* configMemoria;


typedef struct{
  char* puerto_kernel;
  char* ip_kernel;

}confConsola;
confConsola info;


#endif /* MODULO_CONSOLA_H_ */
