#ifndef ESPACIO_KERNEL_H_
#define ESPACIO_KERNEL_H_

#include "modulo_memoria_swap.h"
#include "espacio_usuario.h"

typedef struct{
	int pid;
	t_list* entradasTabla;
}t_tablaPrimerNivel;

typedef struct{
	int pid;
	int nroEntrada;
	t_list* tablaSegundoNivel;
}t_entradaTablaPrimerNivel;

typedef struct{
	int pid;
	int reloj;
	t_list* framesEnMemoria;
}t_infoProceso;

typedef struct{
	int nroEntrada;
	int nroFrame;
	int nroPag;
	bool uso;
	bool presencia;
	bool modificado;
}t_pagina;

typedef struct{
	int nro_frame;
	bool vacio;
	void *ptr_inicio;
}t_frame;


int iniciarTablaDePaginas(int pid, int tamanio);
void crearInfoProceso(int pid);
void crearArchivoSwap(int pid, int tamanio);

t_entradaTablaPrimerNivel* obtenerTablaSegundoNivel(int socketCPU, int pid, int entradaTabla);
bool buscarTablaPrimerNivel(t_tablaPrimerNivel* tablaPrimerNivel);

void cargarPaginaAMemoria(t_pagina* pagina, int pid,int numeroEntrada1Nivel);
void buscarFrameVacioYCargarPagina(t_pagina* pagina, t_infoProceso* infoProceso,void* frameSwap);
void actualizarFrameEnMemoria(t_frame* frameAReemplazar,void* frameNuevo, t_pagina* paginaAReemplazar,t_pagina* paginaNueva);
bool estaLibre(t_frame* frame );
bool encontrarTabla(t_tablaPrimerNivel* tablaPrimerNivel);

void reemplazarPagina(t_pagina* nuevaPagina, int pid, void* frameSwap, int numeroEntrada1Nivel);
bool encontrarTabla(t_tablaPrimerNivel* tablaPrimerNivel);
t_pagina* encontrarPaginaClock(t_infoProceso* infoProceso);
t_pagina* encontrarPaginaClockModificado(t_infoProceso* infoProceso);
t_pagina* encontrarPaginaSinUso(t_infoProceso* infoProceso);
t_pagina* encontrarPaginaSinUsoYSinModificar(t_infoProceso* infoProceso);

void actualizarListaPaginasEnMemoria( t_list* framesEnMemoria, t_pagina* paginaAReemplazar, t_pagina* nuevaPagina);
void actualizarReloj(t_infoProceso* infoProceso);
t_pagina* encontrarPaginaClock(t_infoProceso* infoProceso);

t_infoProceso* buscarInfoProceso(int pid);
bool encontrarPorPid(t_infoProceso* infoProceso);

void escribirFrameAReemplazarEnSwap(t_frame* frameAReemplazar, int pid, t_pagina *paginaAReemplazar, int numeroEntrada1Nivel);


int obtenerNroFrame(t_list* tablaSegundoNivel, int entradaTabla2Nivel,int numeroEntrada1Nivel, int pid);
void enviarTraducciones(int socketCPU,int numeroTablaPrimerNivel,int entradaTabla);
void escribirFramesEnDisco(t_tablaPrimerNivel* tablaPaginasPrimerNivel);
void* recuperarPaginaDeSwap(int pid, t_pagina *pagina, int numeroEntrada1Nivel);
void suspenderProceso(int pid);
void liberarMemoria(int pid);


#endif /* ESPACIO_KERNEL_H_ */
