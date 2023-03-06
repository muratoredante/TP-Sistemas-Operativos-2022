#include "espacio_usuario.h"

void iniciarEspacioUsuario(){

    tamanio_memoria = info.tamMemoria;
    tamanio_frame = info.tamPagina;

    espacioUsuario = malloc(tamanio_memoria);
    int frames_totales = tamanio_memoria/tamanio_frame;
    lista_frames = list_create();


	t_frame *frame;
	int offset = 0;
    for(int i=0; i<frames_totales; i++){
        frame = malloc(sizeof(t_frame));
        frame->nro_frame = i;
        frame->ptr_inicio = espacioUsuario + offset;
        frame->vacio = 1;
        list_add(lista_frames, frame);
        offset += tamanio_frame;
    }
}


void* leerMemoria(uint32_t dirMemoria, int tamanio){

	int nroFrame = floor(dirMemoria / info.tamPagina);
	t_frame* frame = list_get(lista_frames,nroFrame);
	void *dataLeida = malloc(tamanio);
	uint32_t numero ;

	int offset = dirMemoria - nroFrame * info.tamPagina;

	usleep(info.retardoMemoria*1000);

	memcpy(dataLeida,frame->ptr_inicio + offset,tamanio);
	memcpy(&numero, dataLeida,sizeof(uint32_t));

	actualizarBitPaginaEnUso(nroFrame);

	log_info(logMemoria, "DATA %d LEIDA DE LA DIRECCION: %d",numero,dirMemoria);
	log_info(logMemoria, "NRO FRAME: %d OFFSET:%d",nroFrame,offset);

	return dataLeida;
}

void escribirMemoria(uint32_t dirMemoria, int tamanio, void* data){
	int nroFrame = floor(dirMemoria / info.tamPagina);
	t_frame* frame = list_get(lista_frames,nroFrame);
	int offset = dirMemoria - nroFrame * info.tamPagina;
	uint32_t numero ;

	actualizarBitPaginaModificada(nroFrame);
	actualizarBitPaginaEnUso(nroFrame);

	usleep(info.retardoMemoria*1000);

	memcpy(frame->ptr_inicio + offset, data,tamanio);
	memcpy(&numero, data,sizeof(uint32_t));

	log_info(logMemoria, "DATA %d ESCRITA EN LA DIRECCION: %d",numero ,dirMemoria);
	log_info(logMemoria, "NRO FRAME: %d OFFSET:%d",nroFrame,offset);

}

void actualizarBitPaginaModificada(int nroFrame){
	bool encontrarPorNroFrame(t_pagina* pagina){
		return pagina->nroFrame == nroFrame;
	}

	int cantidad_info_procesos = list_size(infoProcesos);
	int i = 0;
	t_pagina* paginaAActualizar = NULL;
	while(cantidad_info_procesos > i && paginaAActualizar == NULL){
		t_infoProceso *info_proceso = list_get(infoProcesos,i);
		paginaAActualizar = list_find(info_proceso->framesEnMemoria,(void*)encontrarPorNroFrame);
		i++;
	}

	paginaAActualizar->modificado = 1;
}

void actualizarBitPaginaEnUso(int nroFrame){

	bool encontrarPorNroFrame(t_pagina* pagina){
		return pagina->nroFrame == nroFrame;
	}

	int cantidad_info_procesos = list_size(infoProcesos);
	int i = 0;
	t_pagina* paginaAActualizar = NULL;
	while(cantidad_info_procesos > i && paginaAActualizar == NULL){
		t_infoProceso *info_proceso = list_get(infoProcesos,i);
		paginaAActualizar = list_find(info_proceso->framesEnMemoria,(void*)encontrarPorNroFrame);
		i++;
	}

	paginaAActualizar->uso = 1;
}

