
#include "modulo_consola.h"

void setup_log_config(){
	//INICIALIZO EL LOG Y EL CONFIG
	logConsola = log_create("consola.log", "consola", 1, LOG_LEVEL_INFO);
	log_info(logConsola, "INICIANDO LOG consola...");

	configMemoria = config_create("modulo_consola.config");
	log_info(logConsola, "CREANDO CONFIG consola...");

	if(configMemoria == NULL){
		log_error(logConsola, "NO SE PUDO CREAR EL CONFIG...");
		exit(-3);
	}
	info.puerto_kernel = config_get_string_value(configMemoria, "PUERTO_KERNEL");
	info.ip_kernel = config_get_string_value(configMemoria, "IP_KERNEL");


}

int conectarAKernel(){
	socketKernel = crear_conexion(info.ip_kernel, info.puerto_kernel);

	if(socketKernel < 0){
		log_error(logConsola, "NO SE PUDO CONECTAR CON EL KERNEL. FINALIZANDO.");
		exit(-2);
	}
	return socketKernel;

}

void enviarTamanioEspacioDirecciones(int tam){


	log_info(logConsola, "El tamanio del espacio de memoria es: %d", tam);

	void* buffer = malloc(sizeof(int));
	memcpy(buffer, &tam, sizeof(int));

	send(socketKernel, buffer, sizeof(int), 0);


}


void  leerArchivoPseudocodigo(char* path){
		FILE * file = fopen(path, "rb");
		if(file){
			fseek(file, 0, SEEK_END);
			int file_size = ftell(file);
			fseek(file, 0L, SEEK_SET);


			char* readed = calloc( 1, file_size+1 );
			fread( readed , file_size, 1 , file);

			readed[file_size]='\0';


			log_info(logConsola,"El archivo de codigo es el siguiente: %s",readed);

			int desplazamiento=0;
			void* buffer = malloc(sizeof(int)+file_size);
			memcpy(buffer + desplazamiento, &file_size, sizeof(int));
			desplazamiento+= sizeof(int);
			memcpy(buffer + desplazamiento, readed, file_size);


			send(socketKernel, buffer, sizeof(int)+file_size, 0);

			int ack;
			recv(socketKernel, &ack, sizeof(int), MSG_WAITALL);

			log_info(logConsola, "Kernel devolvio: %d", ack);


		}
		else{
			log_error(logConsola, "ARCHIVO DE PSEUDOCODIGO NO EXISTE. FINALIZANDO.");
			exit(-1);
		}
}



int main(int argc, char** argv) {


	setup_log_config(argv[3]);

	/*if(argc != 2){
		log_error(logConsola, "NUMERO DE ARGUMENTOS INCORRECTO. FINALIZANDO.");
		return EXIT_FAILURE;
	}*/
	log_info(logConsola,"Argc: %d",argc);
	log_info(logConsola,"Argv: %s",argv[1]);

	socketKernel = conectarAKernel();

	leerArchivoPseudocodigo(argv[1]);
	//leerArchivoPseudocodigo("/home/utnso/Escritorio/tp-2022-1c-Los-Picateclas-2.0/modulo_consola/test.txt");

	//enviarTamanioEspacioDirecciones(argv[1]);
	enviarTamanioEspacioDirecciones(atoi(argv[2]));


	int respuesta;
	recv(socketKernel, &respuesta, sizeof(int), 0);
	log_info(logConsola,"Consola Termina");


	return EXIT_SUCCESS;
}
