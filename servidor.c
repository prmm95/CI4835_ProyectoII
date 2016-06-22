/*
* Ejemplo de server de chat simple con datagramas (UDP).
*
* Leandro Lucarella - Copyleft 2004
* Basado en otros ejemplos públicos.
*
*/
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#define NUM_THREADS 1
#define SERVER_PORT 4321
#define BUFFER_LEN 1024

struct Skt
{
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int addr_len;
	int numbytes;
}skt;

void *beginProto(void *sockt){

	struct Skt *skt = sockt;
	char buf[BUFFER_LEN];
	if ((skt->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("skt");
		exit(1);
	}

	/* Se establece la estructura my_addr para luego llamar a bind() */
	skt->my_addr.sin_family = AF_INET; /* usa host byte order */
	skt->my_addr.sin_port = htons(SERVER_PORT); /* usa network byte order */
	skt->my_addr.sin_addr.s_addr = INADDR_ANY; /* escuchamos en todas las IPs */
	bzero(&(skt->my_addr.sin_zero), 8); /* rellena con ceros el resto de la estructura */
	/* Se le da un nombre al skt (se lo asocia al puerto e IPs) */

	printf("Asignado direccion al skt ....\n");
	printf("FILE-DESC: %d\n",skt->sockfd);
	if (bind(skt->sockfd, (struct sockaddr *)&(skt->my_addr), sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(2);
	}
	printf("HOLAA");
	/* Se reciben los datos (directamente, UDP no necesita conexión) */
	skt->addr_len = sizeof(struct sockaddr);
	printf("Esperando datos ....\n");
	while ((skt->numbytes=recvfrom(skt->sockfd, buf, BUFFER_LEN, 0, (struct sockaddr *)&skt->their_addr,
	(socklen_t *)&skt->addr_len)) != -1) {
		/* Se visualiza lo recibido */
		printf("paquete proveniente de : %s\n",inet_ntoa(skt->their_addr.sin_addr));
		printf("longitud del paquete en bytes: %d\n",skt->numbytes);
		buf[skt->numbytes] = '\0';
		printf("el paquete contiene: %s\n", buf);
		while (1){
		}
	}
	/* cerramos descriptor del skt */
	close(skt->sockfd);
}

int main(int argc, char *argv[]){

	struct Skt skt1;
	struct Skt skt2;
	struct Skt skt3;

	struct Skt sockets[3];
	sockets[0] = skt1;
	sockets[1] = skt2;
	sockets[2] = skt3;

	pthread_t threads[NUM_THREADS];
	int rc;
	long t;
	for(t=0; t<NUM_THREADS; t++){
		printf("In main: creating thread %ld\n", t);
		rc = pthread_create(&threads[t], NULL, beginProto, &sockets[t]);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	pthread_exit(NULL);
	exit (0);
}

/*

Mensajes:

CLIENTE --> SERVIDOR:
* Al pisar boton: Solicitud de puesto. DATOS: ID vehiculo, Tipo operacion (e)
* Al pasar el ticket: Calculo de la tarifa. DATOS: ID vehiculo, Tipo operacion (s)

---------------------------------------------------------------------------
|	TIPO OPERACION	|						DATOS						  |
---------------------------------------------------------------------------

Entrada
---------------------------------------------------------------------------
|	0	|XXXXXXX														  |
---------------------------------------------------------------------------

Salida
---------------------------------------------------------------------------
|	1	|XXXXXXX														  |
---------------------------------------------------------------------------



SERVIDOR --> CLIENTE:
* Permitiendo el paso del cliente. DATOS: Aprobado (1), Hora, fecha y codigo.
* Denegando la entrada por falta de puestos. DATOS: No aprobado (0) el paso.
* Monto a cancelar calculado segun el ID y el tiempo de estadia. DATOS: Salida (2), Tarifa

---------------------------------------------------------------------------
|	TIPO MENSAJE	|						DATOS						  |
---------------------------------------------------------------------------

---------------------------------------------------------------------------
|		0		|						VACIO							  |
---------------------------------------------------------------------------

Mensaje de aprobacion de entrada de un vehiculo con placa AAAHC51 a las 00:15:25 del dia 05/10/2016
---------------------------------------------------------------------------
|		1		|0510201600152512547									  |
---------------------------------------------------------------------------

Mensaje de salida de vehiculo con un monto de 150 (minimo 00000, maximo 9999 -casi 10 mil-)
---------------------------------------------------------------------------
|		2		|0150													  |
---------------------------------------------------------------------------



- Estructura
- Tamano campos seccion datos mensaje
- Argumentos cliente
- 


*/