#include "modulo_memoria_swap.h"

void setupLogConfig(char*config){
	//INICIALIZO EL LOG Y EL CONFIG
	logMemoria = log_create("memoria.log", "memoria", 1, LOG_LEVEL_DEBUG);
	log_info(logMemoria, "INICIANDO LOG memoria...");

	configMemoria = config_create(config);
	log_info(logMemoria, "CREANDO CONFIG memoria...");

	if(configMemoria == NULL){
		log_error(logMemoria, "NO SE PUDO CREAR EL CONFIG...");
		exit(-3);
	}
	info.puerto = config_get_string_value(configMemoria, "PUERTO_ESCUCHA");
	info.tamMemoria = config_get_int_value(configMemoria, "TAM_MEMORIA");
	info.tamPagina = config_get_int_value(configMemoria, "TAM_PAGINA");
	info.entradasPorTabla = config_get_int_value(configMemoria, "ENTRADAS_POR_TABLA");
	info.retardoMemoria = config_get_int_value(configMemoria, "RETARDO_EN_MEMORIA");
	info.algoritmo = config_get_string_value(configMemoria, "ALGORITMO_REEMPLAZO");
	info.marcosPorProceso = config_get_int_value(configMemoria, "MARCOS_POR_PROCESO");
  	info.retardoSwap = config_get_int_value(configMemoria, "RETARDO_SWAP");
  	info.pathSwap = config_get_string_value(configMemoria, "PATH_SWAP") ;
}

void intHandler(int dummy) {
	//TODO liberar estructuras
	log_debug(logMemoria, "INT HANDLER: %d",fin);
    fin = 1;
}

void inicializarModulo(){
	handlerTablasDePaginaPrimerNivel = list_create();
	infoProcesos = list_create();
	iniciarEspacioUsuario();
}

t_modulo handshake(int cliente){
	t_modulo modulo;
	recv(cliente, &modulo, sizeof(t_modulo), MSG_WAITALL);
	return modulo;
}

void procesarPeticionCpu(int socketCPU){

	void* data;
	uint32_t dirFisica;
	int tamanio;

	tipoDePeticionMemoria peticion;
	recv(socketCPU, &peticion, sizeof(tipoDePeticionMemoria), 0);
	pthread_mutex_lock(&mutexPeticions);
	log_debug(logMemoria, "PROCESANDO PETICION DE CPU...");
	if(peticion == ENVIARTRADUCCIONES){
		int pid;
		int entradaTabla;
		recv(socketCPU, &pid, sizeof(int), 0);
		recv(socketCPU, &entradaTabla, sizeof(int), 0);
		enviarTraducciones(socketCPU,pid,entradaTabla);
	}

	recv(socketCPU, &peticion, sizeof(tipoDePeticionMemoria), 0);
	recv(socketCPU,&tamanio, sizeof(int),0);
	recv(socketCPU,&dirFisica,sizeof(int),0);

	int respuesta = 1;
	if(peticion == LEERMEMORIA){

		data = leerMemoria(dirFisica, tamanio);
		send(socketCPU,data,tamanio,0);
	}else{
		data = malloc(tamanio);
		recv(socketCPU,data,tamanio,0);
		escribirMemoria(dirFisica,tamanio,data);
	}
	send(socketCPU,&respuesta,sizeof(int),0);
	free(data);
	pthread_mutex_unlock(&mutexPeticions);
}

void hilo_conexion_kernel(int servidor){
	socket_kernel = esperar_cliente(servidor,logMemoria);
	while(!fin){
		procesarPeticionKernel(socket_kernel);
	}
}

void procesarPeticionKernel(int socketKernel){
	int pid,tamanio;

	tipoDePeticionMemoria peticion;
	recv(socketKernel, &peticion, sizeof(tipoDePeticionMemoria), 0);
	log_debug(logMemoria, "PROCESANDO PETICION DE KERNEL...");
	pthread_mutex_lock(&mutexPeticions);
	switch(peticion){

		case(NUEVOPROCESOREADY):;

		recv(socketKernel,&pid,sizeof(int),0);
		recv(socketKernel,&tamanio,sizeof(int),0);

		log_debug(logMemoria, "PETICION RECIBIDA: 'NUEVO PROCESO PID %d EN READY'", pid);

		crearInfoProceso(pid);
		crearArchivoSwap(pid,tamanio);
		//TODO porq recibe pid ???
		pid = iniciarTablaDePaginas(pid,tamanio);

		send(socketKernel,&pid,sizeof(int),0);
		break;

		case(PROCESOSUSPENDIDO): ;

		recv(socketKernel,&pid,sizeof(int),0);
		recv(socketKernel,&tamanio,sizeof(int),0);

		log_debug(logMemoria, "PETICION RECIBIDA: 'PROCESO PID %d A SUSPENDER'", pid);
		suspenderProceso(pid);
		send(socketKernel,&pid,sizeof(int),0);
		break;

		case(PROCESOFINALIZADO): ;

		recv(socketKernel,&pid,sizeof(int),0);
		recv(socketKernel,&tamanio,sizeof(int),0);

		log_debug(logMemoria, "PETICION RECIBIDA: 'PROCESO PID %d FINALIZADO'", pid);
		liberarMemoria(pid);
		send(socketKernel,&pid,sizeof(int),0);

		break;

		default: log_error(logMemoria,"PETICION DESCONOCIDA");
	}
	pthread_mutex_unlock(&mutexPeticions);

}


void mostrarTablaDePagina(int pid){
	char* path = string_from_format("%s%d.swap",info.pathSwap,pid);
	t_pagina* entradaTablaSegundoNivel = malloc(sizeof(t_pagina));

	FILE* f = fopen(path,"r+");
	int i =0;
	fread(entradaTablaSegundoNivel,sizeof(entradaTablaSegundoNivel),1,f);
	while(!feof(f)){
		i++;
		printf("Entrada numero %d \n",entradaTablaSegundoNivel->nroEntrada);
		printf("Iteracion: %d \n", i);
		fread(entradaTablaSegundoNivel,sizeof(entradaTablaSegundoNivel),1,f);
	}

	free(entradaTablaSegundoNivel);
	fclose(f);

}

//Ahora main seria el hilo que habla con cpu
int main(int argc, char** argv) {

	setupLogConfig(argv[1]);
	inicializarModulo();
	pthread_mutex_init(&mutexSwap, NULL);
	pthread_mutex_init(&mutexPeticions, NULL);


    int servidor = iniciar_servidor("127.0.0.1",info.puerto,logMemoria);

    //Creo un hilo para esperar a kernel
	pthread_t newHilo;
	pthread_create(&newHilo, NULL, (void*)hilo_conexion_kernel, (void*)servidor);
	pthread_detach(newHilo);
	//Espero CPU
	socket_cpu = esperar_cliente(servidor,logMemoria);
	send(socket_cpu,&info.tamPagina,sizeof(int),0);
	send(socket_cpu,&info.entradasPorTabla,sizeof(int),0);
	while (!fin) {
		procesarPeticionCpu(socket_cpu);
	}

	close(servidor);

	return EXIT_SUCCESS;
}
