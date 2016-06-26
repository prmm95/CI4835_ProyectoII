/*
 * Archivo: lib_socket.h
 * 
 * Descripción: Código correspondiente a una puerta de estacionamiento del 
 * Centro Comercial "Moriah". Implementa una interfaz que permite a un vehículo
 * entrar o salir de las instalaciones. Se encarga de la comunicación con el 
 * computador central (CC) del sistema; el cual, administra la distribución
 * de puestos del estacionamiento. 
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


struct Skt {
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int addr_len;
	int numbytes;
} skt;

void *crearSocket(struct Skt *skt,int puerto, int equipo);

#endif