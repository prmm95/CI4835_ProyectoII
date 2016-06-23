#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define NUM_THREADS 3
#define BUFFER_LEN 1024

struct Skt
{
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int addr_len;
	int numbytes;
}skt;

void *beginProtocol(void *buf);
void *crearSocket(struct Skt *skt,int puerto);

int main(int argc, char *argv[]){

	int i;
	long puerto;
	char entradas[50];
	char salidas[50];
	for (i=1;i<argc;i++){
		if (strcmp(argv[i],"-l") == 0){
			puerto = atoll(argv[i+1]);
		}else if(strcmp(argv[i],"-i") == 0){
			strcpy(entradas,argv[i+1]);
		}else if(strcmp(argv[i],"-o") == 0){
			strcpy(salidas,argv[i+1]);
		}
	}
	if (argc < 7){
		printf("\nUso: sem_srv -l puerto_sem_svr -i bitacora_entrada -o bitacora_salida\n\n");
		exit(0);
	}

	struct Skt skt;
	crearSocket(&skt,puerto);
	char buf[BUFFER_LEN];
	pthread_t threads[NUM_THREADS];
	int num_hilos = 0;
	int rc;
	skt.addr_len = sizeof(struct sockaddr);
	printf("Esperando datos ....\n");
	while ((skt.numbytes=recvfrom(skt.sockfd, buf, BUFFER_LEN, 0, 
						(struct sockaddr *)&(skt.their_addr), 
						(socklen_t *)&(skt.addr_len))) != -1) {
		buf[skt.numbytes] = '\0';
		rc = pthread_create(&threads[num_hilos], NULL, beginProtocol, &buf);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
		num_hilos = num_hilos + 1;
		if (num_hilos == 2){
			num_hilos = 0;
		}
	}
	/* cerramos descriptor del skt */
	close(skt.sockfd);
	pthread_exit(NULL);
	exit (0);
}

void *crearSocket(struct Skt *skt,int puerto){
	char buf[BUFFER_LEN];
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

	printf("Asignado direccion al skt ....\n");
	printf("FILE-DESC: %d\n",skt->sockfd);
	if (bind(skt->sockfd, (struct sockaddr *)&(skt->my_addr), 
							 sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(2);
	}
}

void *beginProtocol(void *buf){
	char *buffer = buf;
	printf("el paquete contiene: %s\n", buffer);
}