#include "pruebaMemoria.h"

int main(){
	int servidor = handshake(KERNEL);
	pruebaNuevoProcesoReady(servidor);

	sleep(5);
	pruebaSuspenderProceso(servidor);


	sleep(2);
	mostrarTablaDePagina(1);

	close(servidor);
}

void pruebaNuevoProcesoReady(int servidor){
	tipoDePeticionMemoria peticion = NUEVOPROCESOREADY;
	send(servidor,&peticion,sizeof(peticion),0);

	int pid = 1;
	int tamanio = 2;
	send(servidor,&pid,sizeof(int),0);
	send(servidor,&tamanio, sizeof(int),0);

}

void pruebaSuspenderProceso(int servidor){
	tipoDePeticionMemoria peticion = PROCESOSUSPENDIDO;
	send(servidor,&peticion,sizeof(peticion),0);
	int pid = 1;
	send(servidor,&pid,sizeof(int),0);
}

void probarPeticion(t_modulo modulo, tipoDePeticionMemoria peticion){
	int servidor = handshake(modulo);
	send(servidor,&peticion,sizeof(peticion),0);
}

//Funcion para simular que se conectan desde un módulo
int handshake(t_modulo modulo){

	int servidor = crear_conexion("127.0.0.1", "8889");


	int bytesAEnviar = sizeof(t_modulo);

	send(servidor,&bytesAEnviar,sizeof(int),0);
	send(servidor, &modulo, sizeof(t_modulo),0);
	return servidor;

}

void mostrarTablaDePagina(int pid){
	char* path = string_from_format("../modulo_memoria_swap/swap/%d.swap",pid);
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


	fclose(f);

}




