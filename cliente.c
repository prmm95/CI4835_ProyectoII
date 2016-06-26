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

struct Parametros
{
	struct Skt *skt;
	int *confirmado;
	char *mensaje;
};


//----------------------------------------------------------------------------//

// esto es una prueba: 

char* concat(char *s1, char *s2) {
    char *result = malloc(strlen(s1)+1+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcpy(result,"/");
    strcat(result, s2);
    return result;
}

void *reenviar(void *parametros){

	struct Parametros *p = parametros;
	struct Skt *skt = p->skt;
	int *confirmado = p->confirmado;
	printf("ENTRA?%d",*confirmado);
	// Mientras el servidor no responda con un ACK
	while (!(*confirmado)){
		sleep(1);
		// Si el servidor no ha respondido, vuelve a enviar el mensaje
		if (!(*confirmado)){
			if ((skt->numbytes=sendto(skt->sockfd,p->mensaje,strlen(p->mensaje),0,
				(struct sockaddr *)&(skt->their_addr),sizeof(struct sockaddr))) == -1) {
				perror("sendto");
				exit(2);
			}
		}
	}
	printf("hola\n" );
}

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
	// CREANDO HILO PARA CONTAR TIEMPO DE ESPERA DEL MENSAJE ACK
	pthread_t *thread;
	int rc = pthread_create(&thread, NULL, reenviar, &p);
	if (rc){
		printf("ERROR; return code from pthread_create() is %d\n", rc);
		exit(-1);
	}

	char buf[BUFFER_LEN];
	skt.numbytes=recvfrom(skt.sockfd, buf, BUFFER_LEN, 0, (struct sockaddr *)&(skt.their_addr), 
												  (socklen_t *)&(skt.addr_len));
	confirmado = 1;
	/* cierro socket */
	close(skt.sockfd);
	pthread_exit(NULL);
	exit(0);
}

//----------------------------------------------------------------------------//
//                         Fin del código principal                           //
//----------------------------------------------------------------------------//

