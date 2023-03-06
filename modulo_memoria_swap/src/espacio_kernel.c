#include "espacio_kernel.h"

//ENVIARTRADUCCIONES
void enviarTraducciones(int socketCPU,int pid,int numeroEntrada1erNivel){

	t_entradaTablaPrimerNivel* entradaTablaPrimerNivel = obtenerTablaSegundoNivel(socketCPU,pid,numeroEntrada1erNivel);

	int daIgual = 666;
	send(socketCPU, &daIgual, sizeof(int),0);

	usleep(info.retardoMemoria*1000);

	int numeroEntrada2Nivel;
	recv(socketCPU,&numeroEntrada2Nivel,sizeof(int),0);
	int nroFrame;

	if(entradaTablaPrimerNivel!=NULL){
		nroFrame = obtenerNroFrame(entradaTablaPrimerNivel->tablaSegundoNivel,numeroEntrada2Nivel, pid,numeroEntrada1erNivel);
		usleep(info.retardoMemoria*1000);
		send(socketCPU,&nroFrame,sizeof(int),0);
	}


	log_debug(logMemoria, "ENVIANDO TRADUCCIONES DEL PROCESO PID %d A CPU, NRO FRAME OBTENIDO %d", pid, nroFrame);
}

t_entradaTablaPrimerNivel* obtenerTablaSegundoNivel(int socketCPU, int pid, int entradaTabla){

	bool buscarTablaPrimerNivel(t_tablaPrimerNivel* tablaPrimerNivel){
		return (tablaPrimerNivel->pid == pid);
	};


	t_tablaPrimerNivel* tablaPrimerNivel = list_find(handlerTablasDePaginaPrimerNivel, (void*)buscarTablaPrimerNivel);

	if(tablaPrimerNivel==NULL){
		log_error(logMemoria, "Error al obtener la tabla de página de 1er nivel");
		exit(1);
	}
	t_entradaTablaPrimerNivel* entradaTablaPrimerNivel =  list_get(tablaPrimerNivel->entradasTabla,entradaTabla);

	if(entradaTablaPrimerNivel == NULL){
		log_error(logMemoria,"Error al obtener la tabla de página de 2do nivel");
		exit(1);
	}

	return entradaTablaPrimerNivel;
}

int obtenerNroFrame(t_list* tablaSegundoNivel, int entradaTabla2Nivel, int pid, int numeroEntrada1Nivel){

	t_pagina* pagina= list_get(tablaSegundoNivel,entradaTabla2Nivel);

	if(pagina==NULL){
		log_error(logMemoria,"Error al obtener la pagina");
		return 0;
	}

	if(pagina->presencia == 0){cargarPaginaAMemoria(pagina,pid,numeroEntrada1Nivel);}

	return pagina->nroFrame;

}

void cargarPaginaAMemoria(t_pagina* pagina, int pid,int numeroEntrada1Nivel){

	void* frameSwap =recuperarPaginaDeSwap(pid,pagina, numeroEntrada1Nivel);

	if(frameSwap == NULL){
		log_error(logMemoria,"No se ha podido recuperar la página de swap");
		exit(1);
	}

	t_infoProceso* infoProceso = buscarInfoProceso(pid);
	int cantPaginasEnMemoria = list_size(infoProceso->framesEnMemoria);

	if(cantPaginasEnMemoria < info.marcosPorProceso){
		buscarFrameVacioYCargarPagina(pagina,infoProceso,frameSwap);
	}else{
		reemplazarPagina(pagina, pid,frameSwap, numeroEntrada1Nivel);
	}
	free(frameSwap);

}

void* recuperarPaginaDeSwap(int pid, t_pagina *pagina, int numeroEntrada1Nivel){

	//int nroPaginaSwap =numeroEntrada1Nivel * info.entradasPorTabla+ pagina->nroEntrada;
	void* pagina_de_swap = malloc(info.tamPagina);
	char* pathArchivo = string_from_format("%s%d.swap",info.pathSwap,pid);

	FILE *f = fopen(pathArchivo, "rb+");
	fseek(f, pagina->nroPag * info.tamPagina,SEEK_SET);
	fread(pagina_de_swap,info.tamPagina,1,f);
	usleep(info.retardoSwap*1000);
	fclose(f);

	log_debug(logMemoria,"Se lee pag: %d con valor %d",pagina->nroPag,*(int *)pagina_de_swap);
//	free(pathArchivo);
	return pagina_de_swap;
}


t_infoProceso* buscarInfoProceso(int pid){

	bool encontrarPorPid(t_infoProceso* infoProceso){
		return (pid == infoProceso->pid);
	}
	t_infoProceso* infoProceso =list_find(infoProcesos,(void*)encontrarPorPid);

	return infoProceso;
}

void buscarFrameVacioYCargarPagina(t_pagina* pagina, t_infoProceso* infoProceso,void* frameSwap){
	bool condicion_frame_libre(t_frame *frame){
			return frame->vacio;
	}

	t_frame *frame_vacio = list_find(lista_frames,(void*)condicion_frame_libre);
	frame_vacio->vacio = 0;

	if(frame_vacio==NULL){
		log_error(logMemoria,"Error al cargar la pagina en frame vacio");
		exit(1);
	}

	memcpy(frame_vacio->ptr_inicio,frameSwap,info.tamPagina);
	pagina->nroFrame = frame_vacio->nro_frame;
	pagina->uso = 1;
	pagina->presencia = 1;
	list_add(infoProceso->framesEnMemoria,pagina);
}

void actualizarFrameEnMemoria(t_frame* frameAReemplazar,void* frameNuevo, t_pagina* paginaAReemplazar,t_pagina* paginaNueva){

	if(frameNuevo==NULL){
		return ;
	}

	memcpy(frameAReemplazar->ptr_inicio,frameNuevo,info.tamPagina);

	paginaNueva->presencia = 1;
	paginaNueva->uso = 1;
	paginaNueva->nroFrame = paginaAReemplazar->nroFrame;

	paginaAReemplazar->presencia = 0;
	paginaAReemplazar->modificado = 0;
	paginaAReemplazar->nroFrame = -1;
}



void reemplazarPagina(t_pagina* nuevaPagina, int pid, void* frameSwap, int numeroEntrada1Nivel){
	t_pagina* paginaAReemplazar = NULL;

	bool encontrarTabla(t_tablaPrimerNivel* tablaPrimerNivel){
		return tablaPrimerNivel->pid == pid;
	}


	t_infoProceso* infoProceso = buscarInfoProceso(pid);

	if(strcmp(info.algoritmo, "CLOCK") == 0){
		paginaAReemplazar = encontrarPaginaClock(infoProceso);
	}else{
		paginaAReemplazar = encontrarPaginaClockModificado(infoProceso);
	}
	actualizarReloj(infoProceso);

	t_frame* frameAReemplazar = list_get(lista_frames, paginaAReemplazar->nroFrame);

	log_debug(logMemoria,"FRAME ELEGIDO PARA REEMPLAZAR: %d CLOCK en %d", paginaAReemplazar->nroFrame, infoProceso->reloj);

	if(paginaAReemplazar->modificado == 1 ){

		pthread_mutex_lock(&mutexSwap);
		escribirFrameAReemplazarEnSwap(frameAReemplazar,pid,paginaAReemplazar, numeroEntrada1Nivel);
		pthread_mutex_unlock(&mutexSwap);
	}

	actualizarFrameEnMemoria(frameAReemplazar,frameSwap,paginaAReemplazar,nuevaPagina);
	actualizarListaPaginasEnMemoria(infoProceso->framesEnMemoria,paginaAReemplazar,nuevaPagina);

}


t_pagina* encontrarPaginaClock(t_infoProceso* infoProceso){
	t_pagina* paginaVictima = NULL ;

	//Recorrremos buscando pagina con uso en 0
	paginaVictima = encontrarPaginaSinUso(infoProceso);

	//Si no encuentra ninguna, recorre nuevamente la lista seteando el uso en 0
	while(paginaVictima==NULL){
		paginaVictima = list_get(infoProceso->framesEnMemoria,infoProceso->reloj);

		if(paginaVictima->uso==0){
			return paginaVictima;
		}else{
			paginaVictima->uso = 0;
			paginaVictima = NULL;
		}
		actualizarReloj(infoProceso);
	}
	return paginaVictima;

}

t_pagina* encontrarPaginaClockModificado(t_infoProceso* infoProceso){
	t_pagina* paginaLeida ;

	//Recorremos buscando (U,M)=(0,0)
	t_pagina* paginaElegida	;
	paginaElegida = encontrarPaginaSinUsoYSinModificar(infoProceso);

	while(paginaElegida==NULL){
		paginaLeida = list_get(infoProceso->framesEnMemoria,infoProceso->reloj);
		//Recorremos buscando (U,M)=(0,1)
		if(paginaLeida->uso == 0 && paginaLeida->modificado == 1){
			 paginaElegida = paginaLeida;
			 return paginaElegida;
		}

		//Si U = 1 -> U = 0
		if(paginaLeida->uso == 1){
			paginaLeida->uso = 0;
		}
		paginaElegida = encontrarPaginaSinUsoYSinModificar(infoProceso);
		if(paginaElegida != NULL){
			return paginaElegida;
		}
		actualizarReloj(infoProceso);
	}
	return paginaElegida;
}

void escribirFrameAReemplazarEnSwap(t_frame* frameAReemplazar, int pid, t_pagina *paginaAReemplazar, int numeroEntrada1Nivel){

	//int nroPaginaSwap = numeroEntrada1Nivel * info.entradasPorTabla +paginaAReemplazar->nroEntrada ;
	void* frameAEscribir = malloc(info.tamPagina);
	memcpy(frameAEscribir,frameAReemplazar->ptr_inicio,info.tamPagina);

	char* pathArchivo = string_from_format("%s%d.swap",info.pathSwap,pid);

	FILE *f = fopen(pathArchivo, "rb+");
	fseek(f, paginaAReemplazar->nroPag * info.tamPagina,SEEK_SET);
	fwrite(frameAEscribir,info.tamPagina,1,f);
	usleep(info.retardoSwap*1000);
	fclose(f);

	log_debug(logMemoria,"Se escribe pag: %d con valor %d",paginaAReemplazar->nroPag,*(int *)frameAEscribir);
	free(frameAEscribir);

}

void actualizarListaPaginasEnMemoria( t_list* framesEnMemoria, t_pagina* paginaAReemplazar, t_pagina* nuevaPagina){
	bool encontrarPorNroFrame(t_pagina* pagina){
		return pagina->nroFrame == paginaAReemplazar->nroFrame;
	}
	t_pagina *pagina_auxiliar = NULL;
	int index = 0;
	while(pagina_auxiliar == NULL){
		pagina_auxiliar = list_get(framesEnMemoria,index);
		if(pagina_auxiliar == paginaAReemplazar){
			break;
		}
		index++;
		pagina_auxiliar = NULL;
	}
	if(pagina_auxiliar != NULL){
		list_replace(framesEnMemoria,index,nuevaPagina);
	}
}


void actualizarReloj(t_infoProceso* infoProceso){
	int reloj = infoProceso->reloj;

	if((reloj + 1) >= list_size(infoProceso->framesEnMemoria) ){
		infoProceso->reloj = 0;
	}else{
		infoProceso->reloj++;
	}

}

t_pagina* encontrarPaginaSinUso(t_infoProceso* infoProceso){
	int i = 0;

	while(i < list_size(infoProceso->framesEnMemoria)){
		t_pagina* pagina =list_get(infoProceso->framesEnMemoria,infoProceso->reloj);
		if(pagina->uso == 0){return pagina;}
		actualizarReloj(infoProceso);
		i++;
	}

	return NULL;
}

t_pagina* encontrarPaginaSinUsoYSinModificar(t_infoProceso* infoProceso){
	int i = 0 ;
	t_pagina* paginaLeida;
	t_pagina* paginaElegida = NULL;

	while(i<list_size(infoProceso->framesEnMemoria) && paginaElegida == NULL){
		paginaLeida = list_get(infoProceso->framesEnMemoria,infoProceso->reloj);
		if(paginaLeida->uso == 0 && paginaLeida->modificado == 0){
			paginaElegida = paginaLeida;
		}
		actualizarReloj(infoProceso);
		i++;
	}
	return paginaElegida;
}

//NUEVOPROCESOREADY
void crearInfoProceso(int pid){

	t_infoProceso* infoProceso = malloc(sizeof(t_infoProceso));
	infoProceso->pid = pid;
	infoProceso->reloj = 0;
	infoProceso->framesEnMemoria = 	list_create();
	list_add(infoProcesos,infoProceso);
}

void crearArchivoSwap(int pid, int tamanio){

	char* pathArchivo = string_from_format("%s%d.swap",info.pathSwap,pid);


	FILE* fp = fopen(pathArchivo,"wb");
	ftruncate(fileno(fp), tamanio);
	fclose(fp);
//	free(pathArchivo);


}

int iniciarTablaDePaginas(int pid, int tamanio){

	t_tablaPrimerNivel* tablaPrimerNivel = malloc(sizeof(t_tablaPrimerNivel));
	tablaPrimerNivel->pid = pid;
	tablaPrimerNivel->entradasTabla = list_create();
	//int nroEntradasMax = info.entradasPorTabla * info.entradasPorTabla;
	int k = 0;
	for(int i=0;i<info.entradasPorTabla;i++){
		t_entradaTablaPrimerNivel* entradaTablaPrimerNivel = malloc(sizeof(t_entradaTablaPrimerNivel));
		entradaTablaPrimerNivel->nroEntrada= i;
		entradaTablaPrimerNivel->pid = pid;
		entradaTablaPrimerNivel->tablaSegundoNivel = list_create();


		for(int j=0;j<info.entradasPorTabla;j++){
			t_pagina* entradaTablaSegundoNivel = malloc(sizeof(t_pagina));
			entradaTablaSegundoNivel->nroEntrada = j;
			entradaTablaSegundoNivel->nroFrame = -1;
			entradaTablaSegundoNivel->nroPag = k;
			entradaTablaSegundoNivel->presencia = 0;
			entradaTablaSegundoNivel->uso = 0;
			entradaTablaSegundoNivel->modificado = 0;
			list_add(entradaTablaPrimerNivel->tablaSegundoNivel,entradaTablaSegundoNivel);
			k++;
		}
		list_add(tablaPrimerNivel->entradasTabla,entradaTablaPrimerNivel);

	}

	list_add(handlerTablasDePaginaPrimerNivel,tablaPrimerNivel);
	log_debug(logMemoria,"Iniciando tabla de paginas para el proceso pid %d", pid);
	return pid;
}

//PROCESOSUSPENDIDO
void escribirFramesEnDisco(t_tablaPrimerNivel* tablaPaginasPrimerNivel){

	int nroEntrada1Nivel;
	char* pathArchivo = string_from_format("%s%d.swap",info.pathSwap,tablaPaginasPrimerNivel->pid);

	void escribirEnSwap(t_pagina* entradaTablaSegundoNivel){

		if(entradaTablaSegundoNivel->presencia && entradaTablaSegundoNivel->modificado){
			t_frame* frame = list_get(lista_frames,entradaTablaSegundoNivel->nroFrame);
			void* frameAEscribir = malloc(info.tamPagina);
			memcpy(frameAEscribir, frame->ptr_inicio,info.tamPagina);

			int nroPaginaSwap =nroEntrada1Nivel * info.entradasPorTabla+ entradaTablaSegundoNivel->nroEntrada;

			FILE *f = fopen(pathArchivo, "rb+");
			fseek(f, nroPaginaSwap * info.tamPagina,SEEK_SET);
			fwrite(frameAEscribir,info.tamPagina,1,f);
			usleep(info.retardoSwap*1000);
			fclose(f);

			frame->vacio = 1;
//			entradaTablaSegundoNivel->presencia = 0;
			//estabamos marcando presencia = 0 solo en los modificado=1
			entradaTablaSegundoNivel->modificado = 0;
			free(frameAEscribir);
		}
	}

	void recorrerTablaProcesoYEscribirEnSwap(t_entradaTablaPrimerNivel* entradaTablaPrimerNivel){
		nroEntrada1Nivel = entradaTablaPrimerNivel->nroEntrada;
		list_iterate(entradaTablaPrimerNivel->tablaSegundoNivel,(void*)escribirEnSwap);
	}

	list_iterate(tablaPaginasPrimerNivel->entradasTabla, (void*)recorrerTablaProcesoYEscribirEnSwap);
//	free(pathArchivo);

}


void suspenderProceso(int pid){


	bool obtenerTablaPrimerNivel(t_tablaPrimerNivel* tablaPaginasPrimerNivel){
		return pid == tablaPaginasPrimerNivel->pid;
	}

	t_tablaPrimerNivel* tablaPaginasPrimerNivel = list_find(handlerTablasDePaginaPrimerNivel,(void*)obtenerTablaPrimerNivel);

	pthread_mutex_lock(&mutexSwap);
	escribirFramesEnDisco(tablaPaginasPrimerNivel);
	pthread_mutex_unlock(&mutexSwap);


	//Saco de framesEnMemoria las paginas que ya no estan en memoria
	t_infoProceso* infoProceso = buscarInfoProceso(pid);
	int i = 0;
	int cantidadFramesEnMemoria = list_size(infoProceso->framesEnMemoria);
	while(i < cantidadFramesEnMemoria){
		t_pagina *pagina_no_presente = list_remove(infoProceso->framesEnMemoria,0);
		pagina_no_presente->presencia = 0;
		t_frame *frameLiberado = list_get(lista_frames,pagina_no_presente->nroFrame);
		frameLiberado->vacio = 1;
		i++;
	}

	log_debug(logMemoria,"Proceso pid %d suspendido",pid);


}


//PROCESOFINALIZADO
void liberarMemoria(int pid){

	bool buscarTablaPrimerNivel(t_tablaPrimerNivel* tablaPrimerNivel){
		return (tablaPrimerNivel->pid == pid);
	};



	char* pathArchivo = string_from_format("%s%d.swap",info.pathSwap,pid);
	remove(pathArchivo);
//	free(pathArchivo);

	t_tablaPrimerNivel* tablaPrimerNivel = list_remove_by_condition(handlerTablasDePaginaPrimerNivel, (void*)buscarTablaPrimerNivel);

	while(list_size(tablaPrimerNivel->entradasTabla) > 0){
		t_entradaTablaPrimerNivel *entrada_primer_nivel = list_remove(tablaPrimerNivel->entradasTabla,0);
		while(list_size(entrada_primer_nivel->tablaSegundoNivel) > 0){
			t_pagina *entrada_segundo_nivel = list_remove(entrada_primer_nivel->tablaSegundoNivel,0);
			free(entrada_segundo_nivel);
		}
		list_destroy(entrada_primer_nivel->tablaSegundoNivel);
		free(entrada_primer_nivel);
	}
	list_destroy(tablaPrimerNivel->entradasTabla);
	free(tablaPrimerNivel);

	bool encontrarPorPid(t_infoProceso* infoProceso){
		return (pid == infoProceso->pid);
	}
	t_infoProceso* infoProceso = list_remove_by_condition(infoProcesos,(void*)encontrarPorPid);
	list_destroy(infoProceso->framesEnMemoria);
	free(infoProceso);


	log_debug(logMemoria,"Memoria liberada del proceso, pid:%d",pid);
}

