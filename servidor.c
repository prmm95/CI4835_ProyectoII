/*
 * Archivo: servidor.c
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

#include <arpa/inet.h>  //
#include <errno.h>      //
#include <math.h>       //
#include <netdb.h>      //
#include <pthread.h>    //
#include <time.h>       // 
#include <stdio.h>      // Uso de la entrada/salida estándar (I/O).
#include <stdlib.h>     //
#include <string.h>     //
#include <sys/types.h>  //
#include <unistd.h>     //

//----------------------------------------------------------------------------//
//                          Definición de constantes                          //
//----------------------------------------------------------------------------//

#define NUM_PUESTOS 200 // Número de puestos del estacionamiento.
#define NUM_THREADS 3
#define BUFFER_LEN 1024

//----------------------------------------------------------------------------//
//                      Definición del tipos estructurados                    //
//----------------------------------------------------------------------------//

typedef struct tm Tiempo;

//----------------------------------------------------------------------------//

typedef struct tiempo {
	Tiempo tiempoF;
	time_t segundos;
} TiempoV;

//----------------------------------------------------------------------------//

typedef struct vehiculo {
 	TiempoV Entrada;
 	TiempoV Salida;
 	int codigo;
 	char *serial; // cambiar, porque no es int.
 	int tarifa;
 	struct vehiculo *siguiente;
} Vehiculo;

//----------------------------------------------------------------------------//

struct Skt {
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	int addr_len;
	int numbytes;
} skt;

//----------------------------------------------------------------------------//

typedef struct argHilo {
	char *buf;
	char *entradas;
	char *salidas;
	int  *puestosOcupados;
	int  *codigoVehiculo;
	time_t tiempoSegundos;
	Tiempo tiempoFormato;
	Vehiculo **listaVehiculos;
} ArgumentoHilo;

//----------------------------------------------------------------------------//

// lista enlazada para los mensajes


//----------------------------------------------------------------------------//
//                          Definición de funciones                           //
//----------------------------------------------------------------------------//

// Notas:
// borrar de la lista al vehiculo cuando sale

void escribirBitacora(char *rutaBitacora,char *tipoOperacion,Vehiculo vehiculo) {
	/*
	 * Descripción: 
	 *
	 * Variables de entrada:
	 *
	 * Variables de salida:
	 *
	*/

	// Inicializción de variables:
	// Revisar si esto puede ir en el main para no abrir y cerrar el fb cada vz.
	FILE *bitacora;
	bitacora = fopen(rutaBitacora,"a"); // no se si pasar esto como parametro para no abrir y cerrar el df cada vez
	Tiempo fechaB;
	
	// No se si este if se puede hacer mas elegante:
	if (tipoOperacion == "e") {
		fechaB = vehiculo.Entrada.tiempoF;
	}
	
	else {
		fechaB = vehiculo.Salida.tiempoF;
	}
		
	fprintf(bitacora,"-------------------\n");
	fprintf(bitacora,"Fecha: %d/%02d/%d \n",fechaB.tm_mday,fechaB.tm_mon + 1,fechaB.tm_year + 1900);
	fprintf(bitacora,"Hora: %02d:%02d:%02d\n", fechaB.tm_hour, fechaB.tm_min, fechaB.tm_sec);
	fprintf(bitacora,"Serial: %s \n",vehiculo.serial);
	fprintf(bitacora,"Código: %d",vehiculo.codigo);
	fprintf(bitacora,"\n-------------------\n");
	
	fclose(bitacora);
	
}


void agregarVehiculo(Vehiculo **lisVehic,TiempoV Ent, int *cod, char *ser, char *bitacora) {
			
	// Se crea el nuevo vehiculo que se agregara a la lista enlazada:
	Vehiculo *nuevoVehiculo = (Vehiculo *) malloc(sizeof(Vehiculo));
	char *serialInd = (char *) malloc(sizeof(ser));
	strcpy(serialInd,ser);
	nuevoVehiculo->codigo = *cod;
 	nuevoVehiculo->serial = serialInd;
 	nuevoVehiculo->Entrada = Ent;
 	nuevoVehiculo->siguiente = NULL;
 	nuevoVehiculo->tarifa = 0;
 	// Se actualiza el contador de vehiculo:
 	*cod = *cod + 1;

 	// En caso de agregar el primer vehiculo a la lista:
 	if (*lisVehic == NULL) {
 		printf("ES NULL\n");
 		*lisVehic = nuevoVehiculo;
 	}
 	
 	// En caso de que ya existieran vehiculos en la lista.
 	else {
 		
 		// Se recorre la lista de vehiculos para agregar la entrada al final:
 		Vehiculo *aux;
 		aux = *lisVehic;
 	
 		while (aux->siguiente != NULL) {
 		
 			//printf("Este es el vehiculo de codigo %d \n",aux->codigo);

	 		aux = aux->siguiente;
	 	}
 	
 		aux->siguiente = nuevoVehiculo;
 	
 	}

 	escribirBitacora(bitacora,"e",*nuevoVehiculo);
 
}

//----------------------------------------------------------------------------//

void eliminarVehiculo(Vehiculo **inicioList, char *serial, TiempoV tiempoS, char *bitacora, int *puestosOcupados) {

	Vehiculo *aux = *inicioList;
	Vehiculo *anterior = NULL;
	int encontrado = 0;
	int comparador;
	int tarifaVehiculo = 0;

	// Busca al vehiculo a eliminar y elimina su referencia de la lista:
	while (aux != NULL) {

		comparador = strcmp(aux->serial,serial);

		printf("SERIAL AUX: %s / SERIAL A ELIMINAR %s \n",aux->serial,serial);

		if (comparador == 0) {
			

			if (anterior == NULL) {
				*inicioList = aux->siguiente;
				encontrado = 1;
				break;
			}

			else {
				anterior->siguiente = aux->siguiente;
				encontrado = 1;
				break;
			}
		}

		anterior = aux;
		aux = aux->siguiente;

	}

	// Con el vehiculo a eliminar, se acualiz
	if (encontrado) {
		printf("ENCONTRADO \n");

		aux->Salida = tiempoS;
		tarifaVehiculo = calcular_costo(*aux);
		printf("LA TARIFA ES: %d\n",tarifaVehiculo);
		escribirBitacora(bitacora,"s",*aux);
		free(aux);
		*puestosOcupados = *puestosOcupados -1;
	}
	
	else {
		printf("NO ENCONTRADO\n");
	}
	
}


void imprimirLista(Vehiculo **inicioList) {

	Vehiculo *aux = *inicioList;
	int i = 0;

	while (aux != NULL) {
		printf("-------------------\n");
		printf("Serial: %s \n",aux->serial);
		printf("Código: %d",aux->codigo);
		printf("\n-------------------\n");
		i++;
		aux = aux->siguiente;
	}

	printf("Numero de elementos en la lista -> %d\n",i);

}


//----------------------------------------------------------------------------//



//----------------------------------------------------------------------------//


int calcular_costo(Vehiculo vehiculo) {
	/*
	 * Descripción: Esta función determina el costo a pagar de un vehículo por
	 * su estadía en el estacionamiento. Considere que la primera hora de 
	 * estacionamiento cuesta 80 Bs y la fracción (1 hora) cuesta 30 Bs
	 *
	 * Variables de entrada:
	 *     - Entrada: Fecha y hora de entrada del vehículo al estacionamiento.
	 *     - Salida: Fecha y hora de salida del vehículo al estacionamiento.
	 * Variables de salida:
	 *   - tarifa : int // Tarifa a pagar por el tiempo de estadía.
	 *
	*/

	// Inicialización de variables:
	int tarifa = 80;
	double segundos;
	double horas;
	segundos = difftime(vehiculo.Salida.segundos,vehiculo.Entrada.segundos);
	horas = ceil(segundos / 3600) - 1;
	printf("El numero de segundos es %f\n",segundos);
	printf("El numero de horas es %f\n",horas);
	tarifa += horas*30;
	
	printf("La tarifa es %d\n",tarifa);
	
	return tarifa;

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

//----------------------------------------------------------------------------//

void *beginProtocol(void *argumentos) {

	// Inicializacion de variables:
	ArgumentoHilo *argumentosBP = argumentos;
	const char separador[2] = "/";
	char *operacion;
	char *tipoMensaje;
	char *numeroSecuencia;
	char *placaStr;
	int  *puestosOcupados = argumentosBP->puestosOcupados;
	int  *codigoVehiculo = argumentosBP->codigoVehiculo;
	operacion = strtok(argumentosBP->buf,separador);
	tipoMensaje = strtok(NULL,separador);
	numeroSecuencia = strtok(NULL,separador);
	placaStr = strtok(NULL,separador);
 	int opcion = atoi(operacion);

 	char *placa = placaStr;


	time_t t1 = argumentosBP->tiempoSegundos;
	Tiempo tm1 = argumentosBP->tiempoFormato;

	TiempoV tiempo1;
	tiempo1.tiempoF = tm1;
	tiempo1.segundos = t1;


	time_t t2 = t1 + 7201;
	Tiempo tm2 = *localtime(&t2);

	//armar tiempo 1
	// Prueba
	Vehiculo **inicioList = argumentosBP->listaVehiculos;


 	// prints
 	// Se muestra en pantalla el tiempo actual:
 	printf("HILO\n");
	printf("Fecha: %02d/%02d/%d \n",tm1.tm_mday,tm1.tm_mon + 1,tm1.tm_year + 1900);
	printf("Hora: %02d:%02d:%02d \n", tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	// printf("HOLA %s\n",argumentosBP->buf);
	// printf("QUE %s\n",argumentosBP->entradas);
	// printf("TAL %s\n",argumentosBP->salidas);
	// printf("el paquete contiene: %s\n", argumentosBP->buf);
	// printf("La operacion es -> %s\n",operacion);
	// printf("El tipo de mensaje es -> %s\n",tipoMensaje);
	// printf("El numero de secuencia es es -> %s\n",numeroSecuencia);
	printf("La placa es -> %s\n",placa);
	// printf("El numero de puestos ocupados es -> %d\n",*puestosOcupados);
	// printf("El codigo del Vehiculo es -> %d\n",*argumentosBP->codigoVehiculo);


 	// Verificacion de la operación (Entrada o salida):

    switch(opcion) {

    	// Entrada:
    	case 0:

    		// Verificacion de puestos disponibles:
			if (*puestosOcupados <= NUM_PUESTOS) {
				//printf("PUESTOS DISPONIBLES %d\n",NUM_PUESTOS-*puestosOcupados);
				
				// semaforo:
				*puestosOcupados = *puestosOcupados + 1;
				// Se agrega el Vehiculo a la estructura:
				agregarVehiculo(inicioList,tiempo1,codigoVehiculo,placa,argumentosBP->entradas);
				// Se escribe la entrada en la Bitacora:
				imprimirLista(inicioList);
				
				break;
			}

			else {
				printf("NO HAY PUESTOS \n");
				break;
			}

		// Salida:
    	case 1:

    		//carro1.Salida.tiempoF = tm2;
 			//carro1.Salida.segundos = t2
    		// eliminar vehiculo:
    		eliminarVehiculo(inicioList,placa,tiempo1,argumentosBP->salidas,puestosOcupados);
    		// escribir Bitacora (salida);
    		//escribirBitacora(argumentosBP->salidas,"s",carro1);
    		break;

    	default:
    		perror("Operación incorrecta\n");
    		break;  
    }

    printf("PUESTOS DISPONIBLES %d\n",NUM_PUESTOS-*puestosOcupados);
	
}

//----------------------------------------------------------------------------//
//                       Inicio del código principal                          //
//----------------------------------------------------------------------------//

int main(int argc, char *argv[]){

	// Inicialización de variables: 
	int puestosOcupados = 0;
 	int codigoVehiculo = 0; // Global
	int i;
	Vehiculo *listaVehiculos = NULL;


	long puerto;
	char entradas[50]; /* HAY QUE DEFINIR EL TAMANO DEL STRING DE LA RUTA DEL ARCHIVO */
	char salidas[50];
	if (argc < 7){
		printf("\nUso: sem_srv -l puerto_sem_svr -i bitacora_entrada -o bitacora_salida\n\n");
		exit(0);
	}
	for (i=1;i<argc;i++){
		if (strcmp(argv[i],"-l") == 0){
			puerto = atoll(argv[i+1]);
		}else if(strcmp(argv[i],"-i") == 0){
			strcpy(entradas,argv[i+1]);
		}else if(strcmp(argv[i],"-o") == 0){
			strcpy(salidas,argv[i+1]);
		}
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

		// Se calcula el tiempo de llegada del mensaje al servidor:
		time_t t1 = time(NULL);
		Tiempo tm1 = *localtime(&t1);

	 	//printf("MENSAJE\n");
		//printf("Fecha: %02d/%02d/%d \n",tm1.tm_mday,tm1.tm_mon + 1,tm1.tm_year + 1900);
		//printf("Hora: %02d:%02d:%02d \n", tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
		//printf("%HOLA s\n",argumentos->buf);
		//printf("QUE %s\n",argumentos->entradas);
		//printf("TAL %s\n",argumentos->salidas);

		// Se crea la estructura de datos de argumentos del hilo:
		ArgumentoHilo *argumentos = (ArgumentoHilo *) malloc(sizeof(ArgumentoHilo)); 
		argumentos->buf = buf;
		argumentos->entradas = entradas;
		argumentos->salidas = salidas;
		argumentos->puestosOcupados = &puestosOcupados;
		argumentos->codigoVehiculo = &codigoVehiculo;
		argumentos->tiempoSegundos = t1;
		argumentos->tiempoFormato = tm1;
		argumentos->listaVehiculos = &listaVehiculos;

		rc = pthread_create(&threads[num_hilos], NULL, beginProtocol,argumentos);

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

	imprimirLista(&listaVehiculos);

	close(skt.sockfd);
	pthread_exit(NULL);
	exit (0);
}

//----------------------------------------------------------------------------//
//                         Fin del código principal                           //
//----------------------------------------------------------------------------//