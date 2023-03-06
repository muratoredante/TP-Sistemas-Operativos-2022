#include "prueba_CPU.h"

t_pcb *crear_pcb_prueba(){
	t_instruccion *instruccion1 = malloc(sizeof(t_instruccion));
	instruccion1->tipo = NO_OP;
	instruccion1->parametro_1 = 666;
	instruccion1->parametro_2 = 333;

	t_instruccion *instruccion2 = malloc(sizeof(t_instruccion));
	instruccion2->tipo = IO;
	instruccion2->parametro_1 = 888;
	instruccion2->parametro_2 = 321;


	t_instruccion *instruccion3 = malloc(sizeof(t_instruccion));
	instruccion3->tipo = EXIT;
	instruccion3->parametro_1 = 123;
	instruccion3->parametro_2 = 321;

	t_pcb *pcb = malloc(sizeof(t_pcb));
	pcb->id = 159;
	pcb->estimacion_rafaga = 951;
	pcb->program_counter = 0;
	pcb->tamanio = 777;
	pcb->tabla_paginas = 465;
	t_list *lista_instrucciones = list_create();
	list_add(lista_instrucciones, instruccion1);
	list_add(lista_instrucciones, instruccion2);
	list_add(lista_instrucciones, instruccion3);
	pcb->lista_instrucciones = lista_instrucciones;

	return pcb;

}


int main(void) {
	logCPU = log_create("cpu.log", "cpu", 1, LOG_LEVEL_INFO);
	log_info(logCPU, "INICIANDO LOG CPU...");
	int socket_dispatch = crearConexion("127.0.0.1","8001");
	t_pcb *pcb = crear_pcb_prueba();
	mostrar_pcb(logCPU,pcb);
	enviar_PCB(pcb,socket_dispatch);

	while(1){
		//esperar recibir devuelta PCB de CPU
		tipo_mensaje_CPU mensaje_CPU;
		recv(socket_dispatch, &mensaje_CPU, sizeof(tipo_mensaje_CPU),0);
		pcb = recibir_PCB(socket_dispatch);

		uint32_t duracion_bloqueo;
		switch(mensaje_CPU)
		{
		case MSG_IO:
			//recibe el tiempo de bloqueo
			recv(socket_dispatch, &duracion_bloqueo, sizeof(uint32_t), 0);
			log_info(logCPU,"duracion bloqueo: %d",duracion_bloqueo);
			//...
			//ponele que kernel hace algo y despues lo devuelve
			pcb->tabla_paginas = 767;
			enviar_PCB(pcb,socket_dispatch);
			break;
		case MSG_END:
			log_info(logCPU,"llego mensaje exit");
			mostrar_pcb(logCPU,pcb);
			//...
			break;
		case MSG_INTERRUPT:
			//...
			break;
		}

	}

	//close(socket_dispatch);
	sleep(999);

}
