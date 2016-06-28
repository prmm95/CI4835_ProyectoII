/*
 * Archivo: lib_socket.c
 * 
 * Descripción: Librería que incluye la función de creación de sockets. 
 * Esta es usada por los archivos 'cliente.c' y 'servidor.c'
 *
 * Autores:
 *	 Samuel Arleo, 10-10969.
 *   Pablo Maldonado, 12-10561.
 *
 * Última fecha de modificación: 27/06/2016
 *
*/

//----------------------------------------------------------------------------//
//                        Directivas de preprocesador                         //
//----------------------------------------------------------------------------//

#include <arpa/inet.h>  
#include <errno.h>      
#include <netdb.h>       
#include <sys/types.h>  
#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>      
#include <time.h>       
#include <unistd.h>     
#include "lib_socket.h" 

//----------------------------------------------------------------------------//
//                          Definición de funciones                           //
//----------------------------------------------------------------------------//

void *crearSocket(struct Skt *skt,int puerto, int equipo) { 

	if (equipo == 0) {
		puerto = puerto + 1;
	}

	if ((skt->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("skt");
		exit(1);
	}

	/* Se establece la estructura my_addr para luego llamar a bind() */
	skt->my_addr.sin_family = AF_INET; /* usa host byte order */
	skt->my_addr.sin_port = htons(puerto); /* usa network byte order */
	skt->my_addr.sin_addr.s_addr = INADDR_ANY; /* escuchamos en todas las IPs */
	bzero(&(skt->my_addr.sin_zero), 8); /* rellena con ceros el resto de la estructura */
	/* Se le da un nombre al skt (se lo asocia al puerto e IPs) */

	//printf("Asignado direccion al skt ....\n");
	//printf("FILE-DESC: %d\n",skt->sockfd);

	if (bind(skt->sockfd, (struct sockaddr *)&(skt->my_addr), 
							 sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(2);
	}
}

//----------------------------------------------------------------------------//
//                        Fin de definición de funciones                      //
//----------------------------------------------------------------------------//