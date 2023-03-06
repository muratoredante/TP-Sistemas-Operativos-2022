/*
 * transicion.h
 *
 *  Created on: 12 may. 2022
 *      Author: utnso
 */

#ifndef TRANSICION_H_
#define TRANSICION_H_

#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include <time.h>
#include <semaphore.h>
#include <server/comunicacion_Kernel_CPU.h>
#include "client/utils.h"
#include "modulo_kernel.h"





typedef struct{
	uint8_t codigo_operacion;
	t_buffer* buffer;
}t_paqueter;



t_queue* new;
t_list* ready;
t_list* exec;
t_list* blocked;
t_list* suspendedBlocked;
t_list* suspendedReady;
t_list* salida; //cambiar por exit
sem_t mutex_interrupt;
sem_t semMutexNew;
sem_t semMutexReady;
sem_t semMutexExec;
sem_t semMutexExit;
sem_t semMutexBlocked;
sem_t semMutexSuspendedBlocked;
sem_t semMutexSuspendedReady;
sem_t semPrueba;
sem_t semHandshake;
sem_t mutex_dispatch;
sem_t semHiloBlockedReady;
sem_t semHiloSuspendedBlockedSuspendedReady;
sem_t semHiloBlockedSuspendedBlocked;
sem_t semHiloNewReadySuspendedRadyReady;
sem_t semGradoDeMultiprogramacion;
sem_t semSincroPcbEnExec;
sem_t semSincroInterrupt;
sem_t semSincroProcesoEnIO;
sem_t semMutexIO;
sem_t semMutexContadorIO;
sem_t semColaIO;

sem_t semSincroMemoria;





t_pcb* kernel_new_ready();
t_pcb* kernel_blocked_suspended_blocked(t_pcb* proceso);
t_pcb* kernel_exec_blocked();
t_pcb* kernel_suspended_blocked_suspended_ready(t_pcb* proceso);
t_pcb* kernel_suspended_ready_ready(t_pcb* proceso);
t_pcb* kernel_blocked_ready();
void kernel_exec_exit();
t_pcb* kernel_ready_exec(t_pcb* proceso);
t_pcb* kernel_exec_ready(t_pcb* proceso);
void mostrarEstados();



#endif /* TRANSICION_H_ */
