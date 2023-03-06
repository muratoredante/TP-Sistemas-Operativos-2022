#ifndef MODULO_MEMORIA_SWAMP_H_
#define MODULO_MEMORIA_SWAMP_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <server/utils.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <signal.h>

#include "espacio_kernel.h"
#include "espacio_usuario.h"

//Sockets
int socket_cpu;
int socket_kernel;

t_log* logMemoria;
t_config* configMemoria;
t_list* handlerTablasDePaginaPrimerNivel;
t_list* infoProcesos;
pthread_mutex_t mutexSwap;
pthread_mutex_t mutexPeticions;
static volatile int fin = 0;


typedef struct{
  char* puerto;
  int tamMemoria;
  int tamPagina;
  int entradasPorTabla;
  int retardoMemoria;
  char* algoritmo;
  int marcosPorProceso;
  int retardoSwap;
  char* pathSwap;
}confMemoria;
confMemoria info;

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

typedef enum{
	NUEVOPROCESOREADY,
	ENVIARTRADUCCIONES,
	LEERMEMORIA,
	ESCRIBIRMEMORIA,
	COPIARMEMORIA,
	PROCESOSUSPENDIDO,
	PROCESOFINALIZADO,
}tipoDePeticionMemoria;

typedef enum{
	KERNEL,
	CPU
}t_modulo;

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;


void mostrarTablaDePagina(int pid);
void procesarPeticionKernel(int socketKernel);
void procesarPeticionCpu(int socketCPU);
t_modulo handshake(int cliente);

void setupLogConfig(char* config);



#endif /* MODULO_MEMORIA_SWAMP_H_ */
