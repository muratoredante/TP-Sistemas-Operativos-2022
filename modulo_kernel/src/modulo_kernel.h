/*
 * modulo_kernel.h
 *
 *  Created on: 9 may. 2022
 *      Author: utnso
 */

#ifndef MODULO_KERNEL_H_
#define MODULO_KERNEL_H_

#include <client/client.h>
#include <server/utils.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include <commons/temporal.h>
#include "transicion.h"
#include<commons/collections/list.h>
#include<commons/config.h>
#include <commons/string.h>





int id;

char* kernelIpMemoria;
char* kernelPuertoMemoria;
char* kernelIpCpu;
char* kernelPuertoCpuDispatch;
char* kernelPuertoCpuInterrupt;
char* kernelPuertoEscucha;
char* kernelAlgoritmoPlanificacion;
int kernelEstimacionInicial;
double kernelAlfa;
int kernelGradoMultiprogramacion;
int kernelTiempoMaximoBloqueado;
int client_interrupt;
int socket_memoria;

t_list *lista_sockets_consolas;
typedef struct{
	int conexion;
}arg_struct;
pthread_mutex_t mutex_id_pcb;
sem_t nuevo_new;




void iniciarConfig(char* config);
t_config *configKernel;
t_log *logKernel;

t_pcb* kernel_crear_proceso(int id,int tamanio, char* listaDeInstruccionesAEjecutar);
void kernel_inicializar_estructuras_administrativas(char* config);
int sizeOfPcb();
void planificadorLargoPlazo(t_pcb* proceso);
int gradoDeMultiprogramacionActual(); //Que retorne la cantidad de procesos actuales que hay en: ready, exec y blocked
int dispatch_open();
void notificar_interrupt();
void * hilo_dispatch();
void atender_consola(arg_struct* argumentos);
void respuesta_consola_fin_proceso(int pid);
void *hilo_new_ready_suspendedReady_ready();
void * hilo_blocked_suspendedBlocked();
void * hilo_blocked_suspended_ready();
t_pcb*  seleccionarProximoProcesoAEjecutar();
double diferenciaDeTiempoEnMsg(time_t unaHora, time_t otraHora);
t_pcb* atender_IO(t_pcb* unProceso);
t_pcb* nueva_IO(t_pcb* unProceso);
t_pcb* calculoSRT(t_pcb* unProceso,bool isInterrupt);
void* menorSRT(t_pcb* unProceso, t_pcb* otroProceso);
int notificar_memoria(tipoDePeticionMemoria msj,t_pcb* proceso);
void suspenderProceso(t_pcb* pcb);
void atiende_IO(t_pcb* pcb);



#endif /* MODULO_KERNEL_H_ */
