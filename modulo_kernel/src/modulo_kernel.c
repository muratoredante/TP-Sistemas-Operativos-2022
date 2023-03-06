#include "modulo_kernel.h"




void kernel_inicializar_estructuras_administrativas(char* config){


	 id=0;
	 new = queue_create();
	 ready = list_create();
	 exec = list_create();
	 blocked = list_create();
	 suspendedBlocked = list_create();
	 suspendedReady = list_create();
	 salida = list_create(); //cambiar por exit
	 lista_sockets_consolas = list_create();
	 //lo agrego para que no me diga que hay leak
	 iniciarConfig(config);
	 return;

}



void iniciarConfig(char* config)
{
	configKernel = config_create(config);

	kernelIpMemoria = config_get_string_value(configKernel,"IP_MEMORIA");
	kernelPuertoMemoria = config_get_string_value(configKernel,"PUERTO_MEMORIA");
	kernelIpCpu = config_get_string_value(configKernel,"IP_CPU");
	kernelPuertoCpuDispatch = config_get_string_value(configKernel,"PUERTO_CPU_DISPATCH");
	kernelPuertoCpuInterrupt = config_get_string_value(configKernel,"PUERTO_CPU_INTERRUPT");
	kernelPuertoEscucha = config_get_string_value(configKernel,"PUERTO_ESCUCHA");
	kernelAlgoritmoPlanificacion = config_get_string_value(configKernel,"ALGORITMO_PLANIFICACION");
	kernelEstimacionInicial = config_get_int_value(configKernel,"ESTIMACION_INICIAL");
	kernelAlfa = config_get_double_value(configKernel,"ALFA");
	kernelGradoMultiprogramacion = config_get_int_value(configKernel,"GRADO_MULTIPROGRAMACION");
	kernelTiempoMaximoBloqueado = config_get_int_value(configKernel,"TIEMPO_MAXIMO_BLOQUEADO");
	logKernel = log_create("kernel.log", "kernel", 1, LOG_LEVEL_INFO);
	log_info(logKernel, "INICIANDO LOG kernel...");
}



void kernel_inicializar_semaforos(){


	 sem_init(&mutex_interrupt,0,0);
	 sem_init(&mutex_dispatch,0,0);
	 sem_init(&semMutexReady,0,1);
	 sem_init(&semMutexNew,0,1);
	 sem_init(&semMutexExec,0,1);
	 sem_init(&semMutexExit,0,1);
	 sem_init(&semMutexBlocked,0,1);
	 sem_init(&semMutexSuspendedBlocked,0,1);
	 sem_init(&semMutexSuspendedReady,0,1);
	 sem_init(&semPrueba,0,0);
	 sem_init(&semHandshake,0,0);
	 sem_init(&semGradoDeMultiprogramacion,0,kernelGradoMultiprogramacion); //Grado de multiprogramacion
	 sem_init(&nuevo_new,0,0);
	 sem_init(&semHiloBlockedReady,0,0);
	 sem_init(&semHiloSuspendedBlockedSuspendedReady,0,0);
	 sem_init(&semHiloBlockedSuspendedBlocked,0,0);
	 sem_init(&semSincroPcbEnExec,0,0);
	 sem_init(&semMutexIO,0,1);
	 sem_init(&semSincroProcesoEnIO,0,1);
	 sem_init(&semHiloNewReadySuspendedRadyReady,0,0);
	 sem_init(&semSincroMemoria,0,1);
	 sem_init(&semMutexContadorIO,0,1);
	 sem_init(&semColaIO,0,1);
	 log_info(logKernel,"Se Inicializan semaforos");
	 //TODO invento mio

	return;
}

///////////////////CONEXIONES//////////////////////////////////////////////////////////////////

t_list *parsear_instrucciones(char *instrucciones){
	char **nuevo_string = string_split(instrucciones,"\n");


	int i = 0;
	t_list *lista_instrucciones = list_create();
	while(nuevo_string[i] != NULL){
		char **divido_espacio = NULL;
		if(string_starts_with(nuevo_string[i],"N")){
			divido_espacio = string_split(nuevo_string[i]," ");
			int nro_instrucciones = atoi(divido_espacio[1]);
			for(int i=0;i<nro_instrucciones;i++){
				t_instruccion *instruccion = malloc(sizeof(t_instruccion));
				instruccion->tipo = NO_OP;
				instruccion->parametro_1 = 0;
				instruccion->parametro_2 = 0;
				list_add(lista_instrucciones,instruccion);
			}
		}else{
			t_instruccion *instruccion = malloc(sizeof(t_instruccion));
			if(string_starts_with(nuevo_string[i],"I")){
				divido_espacio = string_split(nuevo_string[i]," ");
				instruccion->tipo = IO;
				instruccion->parametro_1 = atoi(divido_espacio[1]);
				instruccion->parametro_2 = 0;
			}else if(string_starts_with(nuevo_string[i],"W")){
				divido_espacio = string_split(nuevo_string[i]," ");
				instruccion->tipo = WRITE;
				instruccion->parametro_1 = atoi(divido_espacio[1]);
				instruccion->parametro_2 = atoi(divido_espacio[2]);
			}else if(string_starts_with(nuevo_string[i],"C")){
				divido_espacio = string_split(nuevo_string[i]," ");
				instruccion->tipo = COPY;
				instruccion->parametro_1 = atoi(divido_espacio[1]);
				instruccion->parametro_2 = atoi(divido_espacio[2]);
			}else if(string_starts_with(nuevo_string[i],"R")){
				divido_espacio = string_split(nuevo_string[i]," ");
				instruccion->tipo = READ;
				instruccion->parametro_1 = atoi(divido_espacio[1]);
				instruccion->parametro_2 = 0;
			}else{
				instruccion->tipo = EXIT;
				instruccion->parametro_1 = 0;
				instruccion->parametro_2 = 0;
			}
			list_add(lista_instrucciones,instruccion);
		}
		i++;
		if(nuevo_string[i] != NULL){
			string_array_destroy(divido_espacio);
		}
	}
	string_array_destroy(nuevo_string);
	return lista_instrucciones;
}

void realizar_handshake(int socket, int valor_mensaje){
	uint32_t mensaje = valor_mensaje;
	send(socket,&mensaje,sizeof(uint32_t),0);
	log_info(logKernel,"Envio mensaje de handshake %d",mensaje);
	recv(socket,&mensaje,sizeof(uint32_t),0);
	log_info(logKernel,"Recibo mensaje de handshake %d",mensaje);
	//La respuesta esperada es el valor enviado + 1
}




void notificar_interrupt(){
	uint32_t mensaje = 123;
	send(client_interrupt,&mensaje,sizeof(uint32_t),0);
	log_info(logKernel,"Se manda INTERRUPT");
}

int notificar_memoria(tipoDePeticionMemoria msj,t_pcb* proceso){
	tipoDePeticionMemoria mensaje = msj;
	int pid;
	send(socket_memoria,&mensaje,sizeof(tipoDePeticionMemoria),0);
	send(socket_memoria,&proceso->id,sizeof(uint32_t),0);
	send(socket_memoria,&proceso->tamanio,sizeof(uint32_t),0);
	log_info(logKernel,"Se manda: %d a memoria",msj);
	recv(socket_memoria, &pid, sizeof(int),0);
	return pid;
}



void* hilo_dispatch(){
	t_pcb* procesoAEjecutar; //TODO CAPAZ MALLOC

	int conexion_dispatch = crear_conexion(kernelIpCpu,kernelPuertoCpuDispatch);
	//esperar handshake interrupt
	sem_wait(&semHandshake);
	realizar_handshake(conexion_dispatch,456);

	sem_wait(&mutex_dispatch);
	t_pcb* pcb = seleccionarProximoProcesoAEjecutar();

	mostrar_pcb(logKernel,pcb);
	enviar_PCB(pcb,conexion_dispatch);
	log_info(logKernel,"Se Envia un primer Pcb a CPU");

	while (1) {
		tipo_mensaje_CPU_Kernel mensaje_CPU;
		recv(conexion_dispatch, &mensaje_CPU, sizeof(tipo_mensaje_CPU_Kernel),0);
		pcb = recibir_PCB(conexion_dispatch);

		sem_wait(&semMutexExec);
		liberar_pcb(list_remove(exec,0));
		list_add(exec,pcb);
		sem_post(&semMutexExec);


		uint32_t duracion_bloqueo;

		switch (mensaje_CPU) {
		case MSG_END:
			log_info(logKernel,"Proceso %d EXIT",pcb->id);
			respuesta_consola_fin_proceso(pcb->id);
			sem_wait(&semSincroMemoria);
			notificar_memoria(PROCESOFINALIZADO,pcb);
			sem_post(&semSincroMemoria);
			kernel_exec_exit();


			sem_post(&semGradoDeMultiprogramacion);
			sem_wait(&mutex_dispatch);
			procesoAEjecutar = seleccionarProximoProcesoAEjecutar();



			enviar_PCB(procesoAEjecutar,conexion_dispatch);

			log_info(logKernel,"Se recibe END de CPU");
			break;

		case MSG_IO:
			//recibe el tiempo de bloqueo
			recv(conexion_dispatch, &duracion_bloqueo, sizeof(uint32_t), 0);
			//log_info(logKernel,"Proceso %d I/O Duracion %d",pcb->id,duracion_bloqueo);
			//log_info(logKernel,"Se quiere mover proceso a Blocked, cantidad de procesos en exec: %d", list_size(exec));
			pcb->timepoEnBlocked=duracion_bloqueo;

			sem_wait(&semMutexContadorIO);
			pcb->contadorIO++;
			sem_post(&semMutexContadorIO);

			log_info(logKernel,"Proceso %d IO, Duracion: %d",pcb->id,duracion_bloqueo);

			//TODO esto no hiciste ya antes del switch ?
			/*sem_wait(&semMutexExec);
			list_remove(exec,0);
			list_add(exec,pcb);
			sem_post(&semMutexExec);*/
			kernel_exec_blocked();

			pcb->contadorIO++;
			pthread_t suspendeProceso;
			pthread_create(&suspendeProceso, NULL, (void*) suspenderProceso, pcb);

			pthread_t atenderIO;
			pthread_create(&atenderIO, NULL, (void*) atiende_IO, pcb);



			sem_post(&semHiloBlockedReady);

			sem_wait(&mutex_dispatch);
			procesoAEjecutar = seleccionarProximoProcesoAEjecutar();

			enviar_PCB(procesoAEjecutar,conexion_dispatch);
			break;
		case MSG_INTERRUPT:;
			log_info(logKernel,"Proceso %d INTERRUMPIDO",pcb->id);
			//log_info(logKernel,"Proceso  INTERRUPT");

			time_t tiempoVerdaderoDeEntradaExec;

			sem_wait(&semMutexExec);
			pcb = list_get(exec,0);
			sem_post(&semMutexExec);
			tiempoVerdaderoDeEntradaExec = pcb->horaDeEntradaAExec;
			pcb = calculoSRT(pcb,true);


			kernel_exec_ready(pcb);

			sem_wait(&mutex_dispatch);
			procesoAEjecutar = seleccionarProximoProcesoAEjecutar();

			if(pcb->id == procesoAEjecutar->id){
				procesoAEjecutar->horaDeEntradaAExec = tiempoVerdaderoDeEntradaExec;
			}

			enviar_PCB(procesoAEjecutar,conexion_dispatch);
			log_info(logKernel,"Se recibe INTERRUPT de CPU");
			sem_post(&semSincroInterrupt);//TODO REVISAR?
			break;
		default:
			log_error(logKernel, "Mensaje de dispatch CPU incorrecto");
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void* hilo_new_ready_suspendedReady_ready(){

	//log_info(logKernel,"Hilo New-Ready iniciando");

	sem_wait(&semHiloNewReadySuspendedRadyReady);
	while(1){


	//	log_info(logKernel,"Hilo New-Ready corriendo");

		if(list_size(suspendedReady) > 0){



			sem_wait(&semMutexSuspendedReady);
			t_pcb* pcb = list_get(suspendedReady, 0);
			sem_post(&semMutexSuspendedReady);

			pcb = calculoSRT(pcb,false);
			sem_wait(&semGradoDeMultiprogramacion);
			pcb = kernel_suspended_ready_ready(pcb);

			//log_info(logKernel,"Grado de multiprogramacion: %d",semGradoDeMultiprogramacion);
			if(string_equals_ignore_case(kernelAlgoritmoPlanificacion,"SRT")){
				notificar_interrupt();
				sem_wait(&semSincroInterrupt);
			}

			sem_wait(&semHiloNewReadySuspendedRadyReady);


		}else{

			if (queue_size(new) > 0) {

				sem_wait(&semGradoDeMultiprogramacion);
				kernel_new_ready();


				if(string_equals_ignore_case(kernelAlgoritmoPlanificacion,"SRT")){
					notificar_interrupt();
					sem_wait(&semSincroInterrupt);
				}

				sem_wait(&semHiloNewReadySuspendedRadyReady);
			}

		}






	}


	return 0;
}


//void * hilo_blocked_suspendedBlocked(){
//
//	void* cumpleConTiempoMaximoDeBlocked(t_pcb* pcb){
//			if(diferenciaDeTiempoEnMsg(NULL,pcb->horaDeEntradaABlocked) > kernelTiempoMaximoBloqueado){
//
//				log_info(logKernel,"Se pasa de blocked a suspendedBlocked el proceso: %d",pcb->id);
//
//					//	log_info(logKernel,"Se pasa Proceso de blocked a suspendedBlocked");
//						sem_wait(&semSincroMemoria);
//						notificar_memoria(PROCESOSUSPENDIDO,pcb);
//						sem_post(&semSincroMemoria);
//						pcb = kernel_blocked_suspended_blocked(pcb);
//						sem_post(&semGradoDeMultiprogramacion);
//
//					//	log_info(logKernel,"Wait semBlocked");
////						sem_wait(&semBlocked);
//
//					}
//			return 0;
//		}
//
//
//	log_info(logKernel,"Hilo Blocked-SuspendedBlocked iniciando");
//	sem_wait(&semHiloBlockedSuspendedBlocked);
//
//	while(1){
//
//		list_iterate(blocked,(void*)cumpleConTiempoMaximoDeBlocked);
//
//		if(list_size(blocked)<=0){
//			sem_wait(&semHiloBlockedSuspendedBlocked);
//		}
//
//		if(list_size(suspendedBlocked)>0){
//			sem_post(&semHiloSuspendedBlockedSuspendedReady);
//		}
//	}
//
//	return 0;
//}


//Este hilo es espera activa :(
/*void* hilo_suspended_blocked_suspended_ready(){

	log_info(logKernel,"Hilo SuspendedBlocked-SuspendedReady iniciando");



	bool gradoDeMultiprogramacionLoPermite(t_pcb* unProceso){
		log_info(logKernel,"GRADO MULTIPROGRAMACION PID: %d", unProceso->id);
		if(gradoDeMultiprogramacionActual() < kernelGradoMultiprogramacion){

			kernel_suspended_blocked_suspended_ready(unProceso);
			sem_post(&semHiloNewReadySuspendedRadyReady);
//				sem_wait(&semSuspendedBlocked);
			return true;
		}
			return false;
	}

	while(1){
		//sem_wait(&semHiloSuspendedBlockedSuspendedReady);

		list_iterate(suspendedBlocked,(void*)gradoDeMultiprogramacionLoPermite);
		if(list_size(suspendedBlocked)<=0){
			sem_wait(&semHiloSuspendedBlockedSuspendedReady);
		}
	}



	return 0;
}*/


//void* hilo_blocked_ready(){
//	log_info(logKernel,"Hilo Blocked-Ready iniciando");
//
//	sem_wait(&semHiloBlockedReady);
//
//
//	while(1){
//
//
//		void* seHaceIO(t_pcb* pcbDePrueba) {
//
//			sem_post(&semHiloBlockedSuspendedBlocked);
//			pcbDePrueba = atender_IO(pcbDePrueba);
//
//
//			bool busquedaPorId(t_pcb* unProceso) {
//
//				return pcbDePrueba->id == unProceso->id;
//			}
//
//			sem_wait(&semMutexSuspendedBlocked);
//			bool estaSuspendido = list_any_satisfy(suspendedBlocked,(void*) busquedaPorId);
//			sem_post(&semMutexSuspendedBlocked);
//
//			if (estaSuspendido) {
//				//kernel_suspended_blocked_suspended_ready(pcbDePrueba); //GUARDAA
//				//sem_post(&semHiloSuspendedBlockedSuspendedReady);
//
//			} else {
//
//				pcbDePrueba = calculoSRT(pcbDePrueba,false);
//				kernel_blocked_ready(pcbDePrueba);
//				notificar_interrupt();
//				sem_wait(&semSincroInterrupt);
//
//
//
//			}
//
//			return pcbDePrueba;
//		}
//
//		if(list_size(blocked)>0){
//			list_iterate(blocked,(void*)seHaceIO);
//		}else{
//			sem_wait(&semHiloBlockedReady);
//		}
//
//		if(list_size(suspendedBlocked)>0){
//
//			list_iterate(suspendedBlocked,(void*)atender_IO);
//			sem_post(&semHiloSuspendedBlockedSuspendedReady);
//		}
//
//	}
//
//	return 0;
//}




t_pcb* seleccionarProximoProcesoAEjecutar(){

	void* menorSRT(t_pcb* unProceso, t_pcb* otroProceso){

		return unProceso->estimacionRafagaActual <= otroProceso->estimacionRafagaActual ? unProceso : otroProceso;
	}

	t_pcb* procesoAEjecutar;

	if(string_equals_ignore_case(kernelAlgoritmoPlanificacion,"SRT")){
		procesoAEjecutar = list_get_minimum(ready,(void*) menorSRT);
	}else{
		procesoAEjecutar = list_get(ready,0);
	}


	kernel_ready_exec(procesoAEjecutar);

	log_info(logKernel,"Se elige el proceso ID: %d para ejecutar",procesoAEjecutar->id);
	return procesoAEjecutar;

}



t_pcb* kernel_crear_proceso(int id, int tamanio, char* listaDeInstruccionesAEjecutar){

	log_info(logKernel,"Se crea proceso: %d",id);
	t_pcb* pcb = malloc(sizeof(t_pcb));
	pcb->id = id;
	pcb->tamanio = tamanio;
	pcb->lista_instrucciones = 	parsear_instrucciones(listaDeInstruccionesAEjecutar);
	pcb->programCounter = 0;
	pcb->estimacionRafagaActual = kernelEstimacionInicial;
	pcb->horaDeEntradaABlocked = 0;
	pcb->horaDeEntradaAExec = 0;
	pcb->horaDeSalidaDeExec = 0;
	pcb->estimacionRafagaAnterior=0;
	pcb->contadorIO = 0;
	sem_wait(&semSincroMemoria);
	pcb->tabla_paginas = notificar_memoria(NUEVOPROCESOREADY,pcb);
	sem_post(&semSincroMemoria);
	id++;
	return pcb;

}




void kernel_inicializar_hilos(){
	log_info(logKernel,"Se Inician Hilos");

	pthread_t hiloDispatch;
	pthread_t hiloNewReady;
//	pthread_t hiloBlockedSuspendedBlocked;
//	pthread_t hiloBlockedReady;
	pthread_t hiloSuspendedBlockedSuspendedReadyy;


	pthread_create(&hiloDispatch, NULL, (void*) hilo_dispatch, NULL);
	pthread_create(&hiloNewReady,NULL, (void*) hilo_new_ready_suspendedReady_ready,NULL);
//	pthread_create(&hiloBlockedSuspendedBlocked,NULL, (void*) hilo_blocked_suspendedBlocked,NULL);
//	pthread_create(&hiloSuspendedBlockedSuspendedReadyy,NULL, (void*) hilo_suspended_blocked_suspended_ready,NULL);
//	pthread_create(&hiloBlockedReady,NULL, (void*) hilo_blocked_ready,NULL);


	pthread_detach(hiloNewReady);
	pthread_detach(hiloDispatch);
//	pthread_detach(hiloBlockedSuspendedBlocked);
//	pthread_detach(hiloBlockedReady);

	return;
}




void kernel_inicializar_conexiones(){
	client_interrupt = crear_conexion(kernelIpCpu,kernelPuertoCpuInterrupt);
	socket_memoria = crear_conexion(kernelIpMemoria,kernelPuertoMemoria);
	logKernel = log_create("log.log", "log", 1, LOG_LEVEL_INFO);
	log_info(logKernel, "INICIANDO LOG Kernel...");

	realizar_handshake(client_interrupt,123);
	sem_post(&semHandshake);

	//int servidor_memoria = iniciar_servidor("127.0.0.1",kernelPuertoMemoria,logKernel);
	//int socket_memoria = esperar_cliente(servidor_memoria,logKernel);



}


int gradoDeMultiprogramacionActual(){
	//agregar semaforo
	int cantProcesosReady = list_size(ready);
	int cantProcesosExec = list_size(salida);
	int cantProcesosBlocked = list_size(blocked);

	return cantProcesosReady+cantProcesosExec+cantProcesosBlocked;
}


//unaHora puede ser NULL, la otra si o si debe tener valor
double diferenciaDeTiempoEnMsg(time_t unaHora, time_t otraHora){

	double valor;

	if(unaHora == 0 && otraHora == 0){
		return 0;
	}

	if (unaHora == 0) {
		unaHora = time(NULL);
		valor = difftime(unaHora, otraHora) * 1000;

	} else {
		valor = difftime(unaHora, otraHora) * 1000;
	}

	if (valor < 0) {
		return valor * (-1);
	} else {
		return valor;
	}
}


//---Conexion Consolas---
void nuevo_hilo(int cliente){

	log_info(logKernel,"NUEVO CLIENTE");

	pthread_t newHilo;

	arg_struct* args = malloc(sizeof(arg_struct));
	args->conexion = cliente;

	pthread_create(&newHilo, NULL, (void*)atender_consola, (void*)args);
	pthread_detach(newHilo);

}



void atender_consola(arg_struct* argumentos){
	int size;
	char* buffer;
	int *cliente = malloc(sizeof(int));
	memcpy(cliente,&argumentos->conexion,sizeof(int));

	//Recibe instrucciones
	recv(*cliente, &size, sizeof(int), MSG_WAITALL);


	buffer = malloc(size);
	recv(*cliente, buffer, size, MSG_WAITALL);

	int respuesta = 0;
	//log_info(logKernel, buffer);
	send(*cliente, &respuesta, sizeof(int), 0);//Envio cualquier cosa para destrabar consola
	//Recibe tamanio proceso
	int tamanio_proceso;
	recv(*cliente,&tamanio_proceso,sizeof(int),0);
	char* stringInstruccion = string_substring_until(buffer,size);
	//Actualizo id y lista de sockets
	pthread_mutex_lock(&mutex_id_pcb);
	int nuevo_id = id;
	id++;
	list_add(lista_sockets_consolas,cliente);
	pthread_mutex_unlock(&mutex_id_pcb);

	//Creo proceso
	t_pcb *pcb = kernel_crear_proceso(nuevo_id,tamanio_proceso,stringInstruccion);
	free(buffer);
	sem_wait(&semMutexNew);
	queue_push(new,pcb);
	sem_post(&semMutexNew);

	sem_post(&semHiloNewReadySuspendedRadyReady);
	sem_post(&nuevo_new);
}

void respuesta_consola_fin_proceso(int pid){
	int *socket = list_get(lista_sockets_consolas,pid);
	int respuesta = 1;
	send(*socket,&respuesta,sizeof(int),0);
	free(socket);
}


//t_pcb* atender_IO(t_pcb* unProceso){
//
//	if(!unProceso->IO){
//		log_info(logKernel, "Comienza IO de proceso: %d", unProceso->id);
//		sem_wait(&semMutexIO);
//		usleep(unProceso->timepoEnBlocked*1000);
//		log_info(logKernel,"Termino IO del proceso: %d",unProceso->id);
//		unProceso->IO = true;
//		sem_post(&semMutexIO);
//		sem_post(&semSincroProcesoEnIO);
//	}
//
//
//	return unProceso;
//}

void atiende_IO(t_pcb* pcb){
	sem_wait(&semMutexContadorIO);
	int contadorIO = pcb->contadorIO;
	sem_post(&semMutexContadorIO);

	bool busquedaPorId(t_pcb* unProceso) {
		return unProceso->id == pcb->id;
	}

	sem_wait(&semColaIO);

	log_info(logKernel, "Proceso %d Inicia sleep IO", pcb->id );
	usleep(pcb->timepoEnBlocked*1000);
	log_info(logKernel, "Proceso %d Finaliza sleep IO", pcb->id );

	sem_post(&semColaIO);

	sem_wait(&semMutexBlocked);
	t_pcb* procesoLista =list_find(blocked,(void*)busquedaPorId);
	sem_post(&semMutexBlocked);


	if(procesoLista!=NULL  && procesoLista->contadorIO == contadorIO){
		pcb = calculoSRT(pcb,false);
		kernel_blocked_ready(pcb);
		if(string_equals_ignore_case(kernelAlgoritmoPlanificacion, "SRT")){
			notificar_interrupt();
			sem_wait(&semSincroInterrupt);
		}
	} //si no esta aca es porq se suspendio entonces suspendedBlocked -> suspendedReady
	else{
		kernel_suspended_blocked_suspended_ready(pcb);
	}

}

void suspenderProceso(t_pcb* pcb){
	sem_wait(&semMutexContadorIO);
	int contadorIO = pcb->contadorIO;
	sem_post(&semMutexContadorIO);
	//esto lo hago porq el pcb puede hacer exit o cambiarse por otro
	//antes de que termine de esperar y entonces leeria basura
	int pid = pcb->id;

	bool busquedaPorId(t_pcb* unProceso) {
		return unProceso->id == pid;
	}

	log_info(logKernel, "Inicio espera maximo tiempo bloqueado, tiempoMaxBloq:%d", kernelTiempoMaximoBloqueado );
	usleep(kernelTiempoMaximoBloqueado*1000);
	log_info(logKernel, "Fin espera maximo tiempo bloqueado");

	sem_wait(&semMutexBlocked);
	t_pcb* procesoLista =list_find(blocked,(void*)busquedaPorId);
	sem_post(&semMutexBlocked);

	if(procesoLista!=NULL && procesoLista->contadorIO == contadorIO){
		kernel_blocked_suspended_blocked(pcb);
		sem_wait(&semSincroMemoria);
		log_info(logKernel, "Proceso %d SUSPENDED BLOCKED", pcb->id);
		notificar_memoria(PROCESOSUSPENDIDO,pcb);
		sem_post(&semSincroMemoria);

		sem_post(&semGradoDeMultiprogramacion);
		sem_post(&semHiloSuspendedBlockedSuspendedReady);
	}else{
		log_info(logKernel, "Proceso %d no excede tiempo maximo de bloqueado", pid);
	}




}

t_pcb* calculoSRT(t_pcb* pcb,bool isInterrupt){

	if(string_equals_ignore_case("SRT",kernelAlgoritmoPlanificacion)){


		int timepoEnExec= diferenciaDeTiempoEnMsg(pcb->horaDeSalidaDeExec,pcb->horaDeEntradaAExec);
		log_info(logKernel,"Tiempo en exec del IDPROCESO: %d , t=%d",pcb->id,timepoEnExec);
		int rafagaAnterior = pcb->estimacionRafagaActual;
		pcb->estimacionRafagaAnterior = rafagaAnterior;

		if(timepoEnExec>0 && !isInterrupt){
			pcb->estimacionRafagaActual = pcb->estimacionRafagaAnterior * (1-kernelAlfa) + timepoEnExec * kernelAlfa;
		}

		if(timepoEnExec>0 && isInterrupt){
		pcb->estimacionRafagaActual -= timepoEnExec;

		}




		log_info(logKernel,"Se actualiza valor de la estimacion de rafaga del proceso: %d, valor de la proxima rafaga: %d",
				pcb->id,pcb->estimacionRafagaActual);


	}



		return pcb;
}




int main(int argc, char** argv) {

	kernel_inicializar_estructuras_administrativas(argv[1]);
	kernel_inicializar_semaforos();
	kernel_inicializar_hilos();
	kernel_inicializar_conexiones();

	int socket = iniciar_servidor("127.0.0.1", kernelPuertoEscucha, logKernel);
	while(1){
		int cliente = esperar_cliente(socket, logKernel);
		nuevo_hilo(cliente);
	}



	return 0;
}
