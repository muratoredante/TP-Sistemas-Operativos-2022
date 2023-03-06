/*
 * pruebaMemoria.h
 *
 *  Created on: 2 may. 2022
 *      Author: utnso
 */

#ifndef PRUEBAMEMORIA_H_
#define PRUEBAMEMORIA_H_

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <client/utils.h>
#include <server/utils.h>

typedef struct{
	int nroEntrada;
	int nroFrame;
	bool uso;
	bool presencia;
	bool modificado;
}t_pagina;

typedef struct{
	t_list* entradasTabla;
	int Pid;
}t_tablaPrimerNivel;

void pruebaNuevoProcesoReady(int servidor);
void pruebaSuspenderProceso(int servidor);
void mostrarTablaDePagina(int pid);
int handshake();


#endif /* PRUEBAMEMORIA_H_ */
