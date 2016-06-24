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

	printf("el numero de argumentos es: %d \n",argc);

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
			opcion = malloc(sizeof(argv[i+1]));
			strcpy(opcion,argv[i+1]);
		}else if(strcmp(argv[i],"-i") == 0){
			placa = malloc(sizeof(argv[i+1]));
			strcpy(placa,argv[i+1]);
		}
	}

	/*printf("PLACA: %s\n", placa);
	printf("MODULO: %s\n", modulo);
	printf("OPCION: %s\n", opcion);
	printf("PUERTO: %ld\n", puerto);*/

	// Inicialización de variables:
	int sockfd; /* descriptor a usar con el socket */
	struct sockaddr_in their_addr; /* almacenara la direccion IP y numero de puerto del servidor */
	struct hostent *he; /* para obtener nombre del host */
	int numbytes; /* conteo de bytes a escribir */

	/* convertimos el hostname a su direccion IP */
	if ((he=gethostbyname(modulo)) == NULL) {
		perror("gethostbyname");
		exit(1);
	}

	/* Creamos el socket */
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(2);
	}

	/* a donde mandar */
	their_addr.sin_family = AF_INET; /* usa host byte order */
	their_addr.sin_port = htons(puerto); /* usa network byte order */
	their_addr.sin_addr = *((struct in_addr *)he->h_addr);
	bzero(&(their_addr.sin_zero), 8); /* pone en cero el resto */
	
	/* enviamos el mensaje */
	char *mensaje = "holaaaaa";
	if ((numbytes=sendto(sockfd,mensaje,strlen(mensaje),0,(struct sockaddr *)&their_addr,
	sizeof(struct sockaddr))) == -1) {
		perror("sendto");
		exit(2);
	}
	
	printf("enviados %d bytes hacia %s\n",numbytes,inet_ntoa(their_addr.sin_addr));
	/* cierro socket */
	close(sockfd);
	exit(0);
}

//----------------------------------------------------------------------------//
//                         Fin del código principal                           //
//----------------------------------------------------------------------------//