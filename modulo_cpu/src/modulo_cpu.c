#include "modulo_cpu.h"

void realizar_handshake(int socket){
	uint32_t mensaje;
	recv(socket,&mensaje,sizeof(uint32_t),0);
	log_info(logCPU,"Recibo mensaje de handshake %d",mensaje);
	mensaje++;
	log_info(logCPU,"Envio mensaje de handshake %d",mensaje);
	send(socket,&mensaje,sizeof(uint32_t),0);
}

void *hilo_interrupt(){
	int servidor_interrupt = iniciar_servidor("127.0.0.1",info.puertoInterrupt,logCPU);
	int socket_kernel_interrupt = esperar_cliente(servidor_interrupt,logCPU);

	realizar_handshake(socket_kernel_interrupt);
	sem_post(&semHandshake);

	while(!fin_programa){
		uint32_t mensaje;
		recv(socket_kernel_interrupt, &mensaje, sizeof(uint32_t), 0);
		if(mensaje == 123){
			pthread_mutex_lock(&mutex_interrupt);
			interrupt = 1;
			pthread_mutex_unlock(&mutex_interrupt);
		}
	}
}

void actualizar_LRU(t_entrada_tlb *entrada_tlb){
	bool condicion_igual_entrada(void *arg){
		t_entrada_tlb *otra_entrada = (t_entrada_tlb *)arg;
		return (otra_entrada == entrada_tlb);
	}
	list_remove_by_condition(tlb,&condicion_igual_entrada);
	list_add(tlb,entrada_tlb);
}

int checkear_TLB(int nro_pag){
	bool condicion_esta_en_tlb(void *arg){
		t_entrada_tlb *una_entrada_tlb = (t_entrada_tlb *)arg;
		return(una_entrada_tlb->pagina == nro_pag);
	}
	t_entrada_tlb *entrada_tlb = list_find(tlb, &condicion_esta_en_tlb);

	if(entrada_tlb == NULL){
		return -1;
	}
	else{
		log_info(logCPU,"TLB HIT, NRO PAG: %d NRO FRAME: %d",nro_pag, entrada_tlb->marco);
		if(!strcmp(info.algoReemplzoTLB,"LRU")){
			actualizar_LRU(entrada_tlb);
		}
		return entrada_tlb->marco;
	}
}

void actualizar_TLB(int nro_pag,int nro_marco){

	bool condicion_mismo_frame(void *arg){
		t_entrada_tlb *una_entrada_tlb = (t_entrada_tlb *)arg;
		return(una_entrada_tlb->marco == nro_marco);
	}

	t_entrada_tlb *nueva_entrada = malloc(sizeof(t_entrada_tlb));
	nueva_entrada->pagina = nro_pag;
	nueva_entrada->marco = nro_marco;

	//Checkeo si ya habia una pagina con el mismo frame y la saco
	t_entrada_tlb *pagina_no_presente = list_remove_by_condition(tlb,&condicion_mismo_frame);
	free(pagina_no_presente);

	if(list_size(tlb) == info.entradas_TLB){
		t_entrada_tlb *victima = list_remove(tlb,0);
		log_info(logCPU,"Entrada TLB Victima pagina:%d marco:%d - Nueva entrada TLB pagina:%d marco:%d",
		victima->pagina,victima->marco,nueva_entrada->pagina,nueva_entrada->marco);
		free(victima);
	}
	list_add(tlb,nueva_entrada);
}

void limpiar_TLB(){
	int i = 0;
	int cantidad_entradas_TLB = list_size(tlb);
	while(cantidad_entradas_TLB > i){
		t_entrada_tlb *entrada_tlb =  list_remove(tlb,0);
		free(entrada_tlb);
		i++;
	}
}

void *pedido_memoria(tipoDePeticionMemoria tipo, int dir_log, int tamanio, void *a_escribir, t_pcb *pcb){

	int nro_pag = floor(dir_log/tamanio_pag);

	int nro_tabla_de_pagina_1er_nivel = pcb->tabla_paginas;

	int nro_entrada_tabla_1er_nivel = floor(nro_pag/nro_entradas_x_tabla);


	int nro_marco = checkear_TLB(nro_pag);
	if(nro_marco == -1){
		tipoDePeticionMemoria msg = ENVIARTRADUCCIONES;
		send(socket_memoria, &msg, sizeof(tipoDePeticionMemoria),0);

		send(socket_memoria, &nro_tabla_de_pagina_1er_nivel, sizeof(int), 0);
		send(socket_memoria, &nro_entrada_tabla_1er_nivel, sizeof(int), 0);
		int numero_tabla_2do_nivel;
		recv(socket_memoria, &numero_tabla_2do_nivel, sizeof(int),0);

		int nro_entrada_tabla_2do_nivel = nro_pag % (nro_entradas_x_tabla);
		send(socket_memoria, &nro_entrada_tabla_2do_nivel, sizeof(int), 0);

		recv(socket_memoria, &nro_marco, sizeof(int), 0);

		log_info(logCPU,"TLB MISS, NRO PAG: %d NRO FRAME: %d",nro_pag, nro_marco);
		if(nro_marco < 0){
			log_info(logCPU, "-------------------------------------------------------");
		}
		actualizar_TLB(nro_pag,nro_marco);
	}
	else{
		tipoDePeticionMemoria msg = NUEVOPROCESOREADY; //Tiro uno incorrecto
		send(socket_memoria, &msg, sizeof(tipoDePeticionMemoria),0);
	}


	int desplazamiento = dir_log - nro_pag*tamanio_pag;

	int direccion_fisica = nro_marco*tamanio_pag + desplazamiento;
	send(socket_memoria, &tipo, sizeof(tipoDePeticionMemoria),0);
	send(socket_memoria, &tamanio, sizeof(int), 0);
	send(socket_memoria, &direccion_fisica, sizeof(int), 0);

	int respuesta;
	if(tipo == ESCRIBIRMEMORIA){
		send(socket_memoria, a_escribir, tamanio, 0);
		recv(socket_memoria,&respuesta,sizeof(int),0);
	}

	if(tipo == LEERMEMORIA){
		void *data_leida = malloc(tamanio);
		recv(socket_memoria, data_leida, tamanio,0);
		recv(socket_memoria,&respuesta,sizeof(int),0);
		return data_leida;
	}

	//TODO era server o cliente el que hacia esto???
	//close(socket_memoria);
	return NULL;

}

void *leer(int dir_logica, t_pcb *pcb){
	int tamanio = 4;
	int tamanio_a_leer;
	int tamanio_restante = tamanio;
	int cantidad_leida = 0;
	int numero_pagina = floor(dir_logica/tamanio_pag);
	int desplazamiento = dir_logica - numero_pagina*tamanio_pag;
	int espacio_restante_en_pagina;
	void *data_final = malloc(tamanio);
	while(cantidad_leida < tamanio){
		espacio_restante_en_pagina = tamanio_pag - desplazamiento;
		if(espacio_restante_en_pagina >= tamanio_restante){
			tamanio_a_leer = tamanio_restante;
		}
		else{
			tamanio_a_leer = espacio_restante_en_pagina;
		}
		void *data = pedido_memoria(LEERMEMORIA,dir_logica + cantidad_leida,tamanio_a_leer,NULL,pcb);
		memcpy(data_final + cantidad_leida,data,tamanio_a_leer);
		free(data);
		desplazamiento = 0;
		cantidad_leida += tamanio_a_leer;
		tamanio_restante = tamanio - cantidad_leida;
		numero_pagina++;
	}
	return data_final;
}

void escribir(int dir_logica,void *a_escribir, t_pcb *pcb){
	int tamanio = 4;
	int tamanio_a_escribir;
	int tamanio_restante = tamanio;
	int cantidad_escrita = 0;
	int numero_pagina = floor(dir_logica/tamanio_pag);
	int desplazamiento = dir_logica - numero_pagina*tamanio_pag;
	int espacio_restante_en_pagina;
	while(cantidad_escrita < tamanio){
		espacio_restante_en_pagina = tamanio_pag - desplazamiento;
		if(espacio_restante_en_pagina >= tamanio_restante){
			tamanio_a_escribir = tamanio_restante;
		}
		else{
			tamanio_a_escribir = espacio_restante_en_pagina;
		}
		pedido_memoria(ESCRIBIRMEMORIA,dir_logica + cantidad_escrita,tamanio_a_escribir,a_escribir + cantidad_escrita, pcb);
		desplazamiento = 0;
		cantidad_escrita += tamanio_a_escribir;
		tamanio_restante = tamanio - cantidad_escrita;
		numero_pagina++;
	}
}






t_pcb *ejecutar_instrucciones(t_pcb *pcb){
	int hay_interrupcion(){
		pthread_mutex_lock(&mutex_interrupt);
		if(interrupt){
			interrupt = 0;
			pthread_mutex_unlock(&mutex_interrupt);
			return 1;
		}
		pthread_mutex_unlock(&mutex_interrupt);
		return 0;
	}

	uint32_t numero;
	void *data_leida;
	void *a_escribir;

	while(1){
		t_instruccion *instruccion = list_get(pcb->lista_instrucciones,pcb->programCounter);
		switch(instruccion->tipo)
		{
		case NO_OP:
			log_info(logCPU,"Proceso %d Instruccion: NO_OP",pcb->id);
			usleep(1000*info.retardoNoOp);
			break;
		case IO:
			log_info(logCPU,"Proceso %d Instruccion: I/O Duracion: %d",pcb->id,instruccion->parametro_1);
			return pcb;
		case EXIT:
			log_info(logCPU,"Proceso %d Instruccion: EXIT",pcb->id);
			return pcb;
		case READ:
			log_info(logCPU,"Proceso %d Instruccion: READ direccion %d",pcb->id,instruccion->parametro_1);
			data_leida = leer(instruccion->parametro_1,pcb);
			memcpy(&numero,data_leida,sizeof(uint32_t));
			log_info(logCPU, "READ %d", numero);
			free(data_leida);
			break;
		case WRITE:
			log_info(logCPU,"Proceso %d Instruccion: WRITE direccion %d Valor a escribir %d"
					,pcb->id,instruccion->parametro_1,instruccion->parametro_2);
			a_escribir = &(instruccion->parametro_2);
			escribir(instruccion->parametro_1, a_escribir, pcb);
			break;
		case COPY:
			log_info(logCPU,"Proceso %d Instruccion: COPY direccion destino %d direccion origen %d"
					,pcb->id,instruccion->parametro_1,instruccion->parametro_2);
			a_escribir = leer(instruccion->parametro_2,pcb);
			escribir(instruccion->parametro_1, a_escribir, pcb);
			free(a_escribir);
			break;
		}
		if(hay_interrupcion()){
			log_info(logCPU,"Hay interrupcion. Se devuelve PCB");
			return pcb;
		}else{
			pcb->programCounter++;
		}
	}


}

/*arg_struct* args = malloc(sizeof(arg_struct));
args->conexion = cliente;*/

void inicializar_CPU(char* config){
	fin_programa = 0;
	interrupt = 0;
	setup_log_config(config);
    pthread_mutex_init(&mutex_interrupt,NULL);
    sem_init(&semHandshake,0,0);
    tlb = list_create();
    pila_LRU_TLB = list_create();


	//Conexion inicial memoria
	socket_memoria = crear_conexion(info.ipMemoria,info.puertoMemoria);

	recv(socket_memoria,&tamanio_pag,sizeof(int),0);
	log_info(logCPU,"Recibo tamanio de paginas %d",tamanio_pag);
	recv(socket_memoria,&nro_entradas_x_tabla,sizeof(int),0);
	log_info(logCPU,"Recibo entradas por tabla %d",nro_entradas_x_tabla);

	//Conexiones con Kernel

    pthread_t newHilo;
  	pthread_create(&newHilo, NULL, (void*)hilo_interrupt, NULL);
  	pthread_detach(newHilo);

	int servidor_dispatch = iniciar_servidor("127.0.0.1",info.puertoDispatch,logCPU);
	socket_kernel_dispatch = esperar_cliente(servidor_dispatch,logCPU);

	sem_wait(&semHandshake);
	//espero handshake interrupt
	realizar_handshake(socket_kernel_dispatch);

}

void devolver_pcb_a_kernel(t_pcb *pcb){
	void enviar_PCB_a_Kernel(tipo_mensaje_CPU_Kernel tipo_mensaje){
		send(socket_kernel_dispatch, &tipo_mensaje, sizeof(tipo_mensaje_CPU_Kernel),0);
		enviar_PCB(pcb,socket_kernel_dispatch);
	}
	//pruebo todos los casos y por ultimo asumo que es interrupt
	t_instruccion *instruccion = list_get(pcb->lista_instrucciones,pcb->programCounter);
	pcb->programCounter++;
	if(instruccion->tipo == EXIT){
		enviar_PCB_a_Kernel(MSG_END);
	} else if(instruccion->tipo == IO){
		enviar_PCB_a_Kernel(MSG_IO);
		//y ademas envio cuanto tiempo se bloquea
		uint32_t duracion_bloqueo = instruccion->parametro_1;
		send(socket_kernel_dispatch, &duracion_bloqueo, sizeof(uint32_t),0);
	}else{
		enviar_PCB_a_Kernel(MSG_INTERRUPT);
	}

	limpiar_TLB();
	liberar_pcb(pcb);
}



int main(int argc, char**argv){ //seria hilo dispatch
	printf("xx");
	inicializar_CPU(argv[1]);


	while(!fin_programa){
		t_pcb *pcb = recibir_PCB(socket_kernel_dispatch);
		//mostrar_pcb(logCPU,pcb);
		devolver_pcb_a_kernel(ejecutar_instrucciones(pcb));
	}


	/*pthread_t newHilo;

	pthread_create(&newHilo, NULL, (void*)hilo_dispatch, NULL);
	void *y;
	int x;
	pthread_join(newHilo,y);
	memcpy(&x,y,sizeof(int));
	log_info(logMemoria,"%d",x);*/
}


