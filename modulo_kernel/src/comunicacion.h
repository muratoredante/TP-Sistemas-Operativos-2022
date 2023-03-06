
#ifndef COMUNICACION_H_
#define COMUNICACION_H_



#include <client/client.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include <commons/temporal.h>
#include "transicion.h"
#include<commons/collections/list.h>
#include<commons/config.h>

void* serializarPaquete(t_buffer* buffer, uint8_t codigoOperacion);
void* serializarYEnviarPcb(t_pcb* pcb, uint8_t codigoOperacion, int conexion);
void* serializarYEnviarMensaje(int conexion, uint8_t mensaje);
t_pcb* deserealizarPcb(t_buffer* buffer);


#endif /* COMUNICACION_H_ */
