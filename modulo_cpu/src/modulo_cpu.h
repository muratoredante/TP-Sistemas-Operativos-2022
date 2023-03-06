#ifndef MODULO_CPU_H_
#define MODULO_CPU_H_

#include <client/client.h>
#include <server/utils.h>
#include <semaphore.h>
#include <server/comunicacion_Kernel_CPU.h>

int fin_programa;
int interrupt;
int socket_kernel_dispatch;
int socket_memoria;
t_log* logCPU;
t_config* configCPU;
pthread_mutex_t mutex_interrupt;
sem_t semHandshake;

int tamanio_pag;
int nro_entradas_x_tabla;

t_list *tlb;
t_list *pila_LRU_TLB;
typedef struct t_entrada_tlb{
	int pagina;
	int marco;
} t_entrada_tlb;

void realizar_handshake(int socket);
void *hilo_interrupt();
void actualizar_LRU(t_entrada_tlb *entrada_tlb);
int checkear_TLB(int nro_pag);
void actualizar_TLB(int nro_pag,int nro_marco);
void limpiar_TLB();
void *pedido_memoria(tipoDePeticionMemoria tipo, int dir_log, int tamanio, void *a_escribir, t_pcb *pcb);
void *leer(int dir_logica, t_pcb *pcb);
void escribir(int dir_logica,void *a_escribir, t_pcb *pcb);
t_pcb *ejecutar_instrucciones(t_pcb *pcb);
void inicializar_CPU(char* config);
void devolver_pcb_a_kernel(t_pcb *pcb);


typedef struct{
  //char* ip;
  char* puertoDispatch;
  char* puertoInterrupt;
  char* ipMemoria;
  char* puertoMemoria;
  int entradas_TLB;
  int retardoNoOp;
  char* algoReemplzoTLB;
}infConfCPU;
infConfCPU info;



int setup_log_config(char* config){
	//INICIALIZO EL LOG Y EL CONFIG
	logCPU = log_create("cpu.log", "cpu", 1, LOG_LEVEL_INFO);
	log_info(logCPU, "INICIANDO LOG CPU...");

	configCPU = config_create(config);
	log_info(logCPU, "CREANDO CONFIG CPU...");

	if(configCPU == NULL){
		log_error(logCPU, "NO SE PUDO CREAR EL CONFIG...");
		exit(-3);
	}
	//info.ip = config_get_string_value(configMemoria, "IP");
	info.ipMemoria = config_get_string_value(configCPU, "IP_MEMORIA");
	info.puertoMemoria = config_get_string_value(configCPU, "PUERTO_MEMORIA");
	info.puertoDispatch = config_get_string_value(configCPU, "PUERTO_ESCUCHA_DISPATCH");
	info.puertoInterrupt = config_get_string_value(configCPU, "PUERTO_ESCUCHA_INTERRUPT");
  	info.entradas_TLB = config_get_int_value(configCPU, "ENTRADAS_TLB");
	info.algoReemplzoTLB = config_get_string_value(configCPU, "REEMPLAZO_TLB");
	info.retardoNoOp = config_get_int_value(configCPU, "RETARDO_NOOP");
	return 1;
}



#endif /* MODULO_CPU_H_ */
