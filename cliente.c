/*
 * Archivo: cliente.c
 * 
 * Descripción: 
 *
 * Autores:
 *	 Samuel Arleo, 10-10969.
 *   Pablo Maldonado, 12-10561.
 *
 * Última fecha de modificación: 27/06/2016
 *
 *
*/

//----------------------------------------------------------------------------//
//                        Directivas de preprocesador                         //
//----------------------------------------------------------------------------//

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <time.h>
#include "lib_socket.h"

//----------------------------------------------------------------------------//
//                          Definición de constantes                          //
//----------------------------------------------------------------------------//

#define SERVER_PORT 4321
#define BUFFER_LEN 1024

//----------------------------------------------------------------------------//
//                       Inicio del código principal                          //
//----------------------------------------------------------------------------//

int main(int argc, char *argv[]) {

	// Inicialización de variables:
	char *placa,*modulo,*opcion;
	int i;
	long puerto;

	// Verificación del pase correcto de argumentos:
	if (argc < 8){
		//perror("Pase incorrecto parámetros\n");
		printf("\nUso: sem_cli -d nombre_modulo_atencion -p puerto -c op -i id_vehiculo\n\n");
		exit(0);
	}

	/* Aqui vamos contemplar casos como que corran el cliente con sem_cli hola -d hola ? (Ver enunciado)*/
	for (i = 1;i < argc;i++) { 
		if (strcmp(argv[i],"-p") == 0){
			puerto = atoll(argv[i+1]);
		}else if(strcmp(argv[i],"-d") == 0){
			modulo = malloc(sizeof(argv[i+1]));
			strcpy(modulo,argv[i+1]);
		}else if(strcmp(argv[i],"-c") == 0){
			if (strcmp(argv[i+1],"e") == 0){
				opcion = "0";
			}else{
				opcion = "1";
			}
		}else if(strcmp(argv[i],"-i") == 0){
			placa = malloc(sizeof(argv[i+1]));
			strcpy(placa,argv[i+1]);
		}
	}

	/*printf("PLACA: %s\n", placa);
	printf("MODULO: %s\n", modulo);
	printf("OPCION: %s\n", opcion);
	printf("PUERTO: %ld\n", puerto);*/

	// NUMERO RANDOM PARA EL NUM DE SECUENCIA

	srand(time(NULL));
	int random = (rand() % 999) + 1000; /* Se le suma 1000 para que sea de 4 digitos */
	char num_sec[8] = "";
	sprintf(num_sec,"%d",random);

	// Creando el socket
	struct Skt skt;
	crearSocket(&skt,puerto,0);
	struct hostent *he; /* para obtener nombre del host */
	
	/* convertimos el hostname a su direccion IP */
	if ((he=gethostbyname(modulo)) == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	char mensaje[30] = ""; // Se puede hacer una func para formar el mensaje
	strcat(mensaje, opcion);
	strcat(mensaje,"/");
	strcat(mensaje,"0"); // Tipo mensaje
	strcat(mensaje,"/");
	strcat(mensaje,num_sec);
	strcat(mensaje,"/");
	strcat(mensaje, placa);
	printf("el string es: %s \n",mensaje);

	/* a donde mandar */
	skt.their_addr.sin_family = AF_INET; /* usa host byte order */
	skt.their_addr.sin_port = htons(puerto); /* usa network byte order */
	skt.their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(skt.their_addr.sin_zero), 8); /* pone en cero el resto */
	
	struct Parametros p;
	p.skt = &skt;
	int confirmado = 0; // Entero que compartiran los hilos para comunicarse y saber si el servidor respondio
	p.confirmado = &confirmado;
	p.mensaje = mensaje;
	/* enviamos el mensaje */
	if ((skt.numbytes=sendto(skt.sockfd,mensaje,strlen(mensaje),0,(struct sockaddr *)&(skt.their_addr),
	sizeof(struct sockaddr))) == -1) {
		perror("sendto");
		exit(2);
	}
	// // CREANDO HILO PARA CONTAR TIEMPO DE ESPERA DEL MENSAJE ACK
	// pthread_t *thread;
	// int rc = pthread_create(&thread, NULL, reenviar, &p);
	// if (rc){
	// 	printf("ERROR; return code from pthread_create() is %d\n", rc);
	// 	exit(-1);
	// }

	char buf[BUFFER_LEN];
	skt.numbytes=recvfrom(skt.sockfd, buf, BUFFER_LEN, 0, (struct sockaddr *)&(skt.their_addr), 
	 											  (socklen_t *)&(skt.addr_len));

	




	char *tipoMensaje;
	char separador[2] = "/";

	tipoMensaje = strtok(buf,separador);

	if ( strcmp(tipoMensaje,"0") == 0) {
		printf("Disculpe, el estacionamiento no tiene puestos disponibles.\n");
	}

	else if (strcmp(tipoMensaje,"1") == 0) {

		char *dia;
		char *mes;
		char *anho;
		char *hora;
		char *minuto;
		char *segundo;
		char *codigo;

		dia = strtok(NULL,separador);
		mes = strtok(NULL,separador);
		anho = strtok(NULL,separador);
		hora = strtok(NULL,separador);
		minuto = strtok(NULL,separador);
		segundo = strtok(NULL,separador);
		codigo = strtok(NULL,separador);

		printf("\n---------------\n");
		printf("ENTRADA:\n");
		printf("    - Código: %s\n",codigo);
		printf("    - Fecha: %s/%s/%s \n",dia,mes,anho);
		printf("    - Hora: %s:%s:%s",hora,minuto,segundo);
		printf("\n---------------\n");

	}

	else if (strcmp(tipoMensaje,"2") == 0) {

		char *monto;

		monto = strtok(NULL,separador);


		printf("\n---------------\n");
		printf("SALIDA: \n");
		printf("    - Precio: %s Bs.",monto);
		printf("\n---------------\n");
	}

	// confirmado = 1;
	/* cierro socket */
	close(skt.sockfd);
	pthread_exit(NULL);
	exit(0);
}

//----------------------------------------------------------------------------//
//                         Fin del código principal                           //
//----------------------------------------------------------------------------//

