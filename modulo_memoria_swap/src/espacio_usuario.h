#ifndef ESPACIO_USUARIO_H_
#define ESPACIO_USUARIO_H_

#include "modulo_memoria_swap.h"

int tamanio_memoria,tamanio_frame,limite_frames;
void* espacioUsuario;
uint32_t null_alloc;
t_list* lista_frames;

void iniciarEspacioUsuario();

void* leerMemoria(uint32_t dirMemoria, int tamanio);
void copiarMemoria(uint32_t dirMemoriaOrigen,uint32_t dirMemoriaDestino);
void escribirMemoria(uint32_t dirMemoria, int tamanio, void* data);
void actualizarBitPaginaModificada(int nroFrame);
void actualizarBitPaginaEnUso(int nroFrame);

#endif /* ESPACIO_USUARIO_H_ */
