/*
 * Archivo: lib_socket.h
 * 
 * Descripción: Encabezado del archivo 'lib_socket.h'
 *
 * Autores:
 *	 Samuel Arleo, 10-10969.
 *   Pablo Maldonado, 12-10561.
 *
 * Última fecha de modificación: 27/06/2016
 *
*/

#ifndef FUNCION_SOCKET
#define FUNCION_SOCKET

//----------------------------------------------------------------------------//
//                             Tipos estructurados                            //
//----------------------------------------------------------------------------//

struct Skt {
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int addr_len;
	int numbytes;
} skt;

//----------------------------------------------------------------------------//

struct Parametros {
	struct Skt *skt;
	int *confirmado;
	char *mensaje;
};

//----------------------------------------------------------------------------//
//                                Funciones                                   //
//----------------------------------------------------------------------------//

void *crearSocket(struct Skt *skt,int puerto, int equipo);
void *reenviar(void *parametros);

#endif