/*
 * transicion.c
 *
 *  Created on: 12 may. 2022
 *      Author: utnso
 */
#include "transicion.h"



void mostrarLista(t_list* lista){


	void forEachElement(t_pcb* proceso){


		printf("Id: %i \n",proceso->id);
	}
	list_iterate(lista,(void *)forEachElement);



	return;
}

//lo comento porq a veces le pasan t_list y a veces t_queue
void mostrarCola(t_list* cola){

/*	t_list* listaAux = list_create();
	t_pcb* proceso;

	while(!queue_is_empty(cola)){

		proceso = queue_pop(cola);
		list_add(listaAux,proceso);

	}

	mostrarLista(listaAux);

	int size = list_size(listaAux);

	for(int i = size - 1; i>=0 ; i--){

		proceso = list_get(listaAux,i);
		queue_push(cola,proceso);
	}



	return;*/
}


void mostrarEstados(){

	printf("Estado NEW: \n");
	sem_wait(&semMutexNew);
	//mostrarCola(new);
	sem_post(&semMutexNew);
	printf("Estado READY: \n");
	sem_wait(&semMutexReady);
	mostrarLista(ready);
	sem_post(&semMutexReady);
	printf("Estado EXEC: \n");
	sem_wait(&semMutexExec);
	mostrarLista(exec);
	sem_post(&semMutexExec);
	printf("Estado BLOCKED: \n");
	sem_wait(&semMutexBlocked);
	mostrarCola(blocked);
	sem_post(&semMutexBlocked);
	printf("Estado SUSPENDED BLOCKED: \n");
	sem_wait(&semMutexSuspendedBlocked);
	mostrarLista(suspendedBlocked);
	sem_post(&semMutexSuspendedBlocked);
	printf("Estado SUSPENDED READY: \n");
	sem_wait(&semMutexSuspendedReady);
	mostrarLista(suspendedReady);
	sem_post(&semMutexSuspendedReady);
	printf("Estado SALIDA: \n\n\n\n");
	sem_wait(&semMutexExit);
	mostrarLista(salida);
	sem_post(&semMutexExit);

	return;
}


int sizeOfPcb(){
	int size = sizeof(int) * 4 + 1 * sizeof(char) * 4 * sizeof(time_t)*3;
	return size;
}


t_pcb* kernel_new_ready(){

	sem_wait(&semMutexNew);
	t_pcb* procesoDeNewAReady = queue_pop(new);
	sem_post(&semMutexNew);
	sem_wait(&semMutexReady);
	list_add(ready,procesoDeNewAReady);
	sem_post(&semMutexReady);
	sem_post(&mutex_dispatch);
	//kernel_new_ready_avisar_a_memoria() TODO
	return procesoDeNewAReady;

}

void kernel_exec_exit(){


		sem_wait(&semMutexExec);
		t_pcb* procesoACambiar = list_remove(exec,0);
		sem_post(&semMutexExec);
//		free(procesoACambiar->estimacionRafagaActual);
		//free(procesoACambiar->estimacionRafagaAnterior);
//		free(procesoACambiar->horaDeEntradaABlocked);
//		free(procesoACambiar->horaDeEntradaAExec);
//		free(procesoACambiar->horaDeSalidaDeExec);
//		free(procesoACambiar->id);
//		free(procesoACambiar->lista_instrucciones);
//		free(procesoACambiar->programCounter);
//		free(procesoACambiar->tabla_paginas);
//		free(procesoACambiar->tamanio);
//		free(procesoACambiar->timepoEnBlocked);
		liberar_pcb(procesoACambiar);

		return;
}

t_pcb* kernel_ready_exec(t_pcb* proceso){

	bool busquedaPorId(t_pcb* unProceso) {

		return proceso->id == unProceso->id;
	}
	sem_wait(&semMutexReady);////////////////////////////////////////////////////////////////////////

	list_remove_by_condition(ready,(void*)busquedaPorId);
	sem_post(&semMutexReady);
	sem_wait(&semMutexExec);
	proceso->horaDeEntradaAExec = time(NULL);
	list_add(exec,proceso);
	sem_post(&semMutexExec);

	//TODO Revisar creo que faltan semáforos

	return proceso;
}

//el proceso que se pasa como parametro es irrelevante ya que en exec solo puede haber 1 solo
t_pcb* kernel_exec_ready(t_pcb* proceso){

	sem_wait(&semMutexExec);
	t_pcb* procesoACambiar = list_remove(exec,0);
	sem_post(&semMutexExec);
	sem_wait(&semMutexReady);
	procesoACambiar->horaDeSalidaDeExec = time(NULL);
	list_add(ready,procesoACambiar);
	sem_post(&semMutexReady);

	//sem_wait(&semPrueba);
	sem_post(&mutex_dispatch);
	return procesoACambiar;
}

t_pcb* kernel_exec_blocked(){

	sem_wait(&semMutexExec);
	t_pcb* procesoAMoverABlocked = list_remove(exec,0);
	sem_post(&semMutexExec);

	procesoAMoverABlocked->horaDeEntradaABlocked = time(NULL);

	sem_wait(&semMutexBlocked);
	procesoAMoverABlocked->horaDeSalidaDeExec = time(NULL);
	list_add(blocked,procesoAMoverABlocked);
	sem_post(&semMutexBlocked);

	log_info(logKernel,"Se mueve proceso a Blocked: %d",procesoAMoverABlocked->id);
	return procesoAMoverABlocked;
}

t_pcb* kernel_blocked_ready(t_pcb* proceso) {

	bool busquedaPorId(t_pcb* unProceso) {

		return proceso->id == unProceso->id;
	}

	sem_wait(&semMutexBlocked);
	t_pcb* procesoACambiar = list_remove(blocked,0);
	sem_post(&semMutexBlocked);
	sem_wait(&semMutexReady);
	list_add(ready,proceso);
	sem_post(&semMutexReady);
	sem_post(&mutex_dispatch);
	log_info(logKernel,"Se mueve proceso de Blocked a Ready: %d",procesoACambiar->id);


	return procesoACambiar;
}


t_pcb* kernel_blocked_suspended_blocked(t_pcb* proceso){

	bool busquedaPorId(t_pcb* unProceso) {
			return proceso->id == unProceso->id;
		}

		sem_wait(&semMutexBlocked);
		t_pcb* procesosAPasarASuspendedBlocked = list_remove_by_condition(blocked,(void*)busquedaPorId);
		sem_post(&semMutexBlocked);

		if(procesosAPasarASuspendedBlocked!= NULL){
			sem_wait(&semMutexSuspendedBlocked);
			list_add(suspendedBlocked,procesosAPasarASuspendedBlocked);
			sem_post(&semMutexSuspendedBlocked);
		}



	return procesosAPasarASuspendedBlocked;
}


t_pcb* kernel_suspended_blocked_suspended_ready(t_pcb* proceso) {

	t_pcb* procesoAPasarDeEstado;

	bool busquedaPorId(t_pcb* unProceso) {

		return proceso->id == unProceso->id;
	}

	//Pcb* procesoAPasarDeEstado=procesoConIOFinalizada();

	sem_wait(&semMutexSuspendedBlocked);
	procesoAPasarDeEstado = list_remove_by_condition(suspendedBlocked,(void*)busquedaPorId);
	sem_post(&semMutexSuspendedBlocked);
	sem_wait(&semMutexSuspendedReady);
	log_info(logKernel,"Se pasa proceso de SuspendedBlocked a SuspendedReady, IDPROCESO: %d",proceso->id);
	list_add(suspendedReady, procesoAPasarDeEstado);
	sem_post(&semMutexSuspendedReady);

	sem_post(&semHiloNewReadySuspendedRadyReady);

	return procesoAPasarDeEstado;
}

t_pcb* kernel_suspended_ready_ready(t_pcb* proceso) {

	bool busquedaPorId(t_pcb* unProceso) {

		return proceso->id == unProceso->id;
	}

	sem_wait(&semMutexSuspendedReady);
	log_info(logKernel,"Transicion suspendedReady-Ready del IDPROCESO: %d",proceso->id);
	t_pcb* procesoAPasarDeEstado = list_remove_by_condition(suspendedReady, (void *)busquedaPorId);
	sem_post(&semMutexSuspendedReady);
	sem_wait(&semMutexSuspendedReady);
	list_add(ready, procesoAPasarDeEstado);
	sem_post(&semMutexSuspendedReady);
	sem_post(&mutex_dispatch);

	return procesoAPasarDeEstado;
}








