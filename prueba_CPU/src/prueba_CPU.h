#ifndef PRUEBA_CPU_H_
#define PRUEBA_CPU_H_
#include "../shared/src/server/probando.h"


#define IP "127.0.0.1"
#define PUERTO "6668"


int fin_programa;
t_log* logCPU;
t_config* configCPU;



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




int setup_log_config(){
	//INICIALIZO EL LOG Y EL CONFIG
	logCPU = log_create("cpu.log", "cpu", 1, LOG_LEVEL_INFO);
	log_info(logCPU, "INICIANDO LOG CPU...");

	configCPU = config_create(".../modulo_CPU/CPU.config");
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



#endif /* PRUEBA_CPU_H_ */
