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
#include "lib_socket.h"

//----------------------------------------------------------------------------//
//                          Definición de constantes                          //
//----------------------------------------------------------------------------//

#define NUM_PUESTOS 200 // Número de puestos del estacionamiento.
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

typedef struct Host {
	int confirmado;
	int num_secuencia;
	char *ip;
	struct Host *siguiente;
}Host;

//----------------------------------------------------------------------------//

typedef struct argHilo {
	char *buf;
	char *entradas;
	char *salidas;
	int  *puestosOcupados;
	int  *contadorVehiculos;
	time_t tiempoSegundos;
	Tiempo tiempoFormato;
	Vehiculo **listaVehiculos;
	Host *clientes;
	char *origen;
	struct Skt *skt;
} ArgumentoHilo;

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
	Tiempo fechaB;
	
	// No se si este if se puede hacer mas elegante:
	if (tipoOperacion == "e") {
		fechaB = vehiculo.Entrada.tiempoF;
	}
	
	else {
		fechaB = vehiculo.Salida.tiempoF;
	}
	
	bitacora = fopen(rutaBitacora,"a"); // no se si pasar esto como parametro para no abrir y cerrar el df cada vez
	fprintf(bitacora,"-------------------\n");
	fprintf(bitacora,"Fecha: %d/%02d/%d \n",fechaB.tm_mday,fechaB.tm_mon + 1,fechaB.tm_year + 1900);
	fprintf(bitacora,"Hora: %02d:%02d:%02d\n", fechaB.tm_hour, fechaB.tm_min, fechaB.tm_sec);
	fprintf(bitacora,"Serial: %s \n",vehiculo.serial);
	fprintf(bitacora,"Código: %d",vehiculo.codigo);
	fprintf(bitacora,"\n-------------------\n");	
	fclose(bitacora);
	
}

//----------------------------------------------------------------------------//

void agregarVehiculo(Vehiculo **lisVehic,TiempoV Ent, int *cod, char *ser, char *bitacora,int *id) {
			
	// Se crea el nuevo vehiculo que se agregara a la lista enlazada:
	Vehiculo *nuevoVehiculo = (Vehiculo *) malloc(sizeof(Vehiculo));
	char *serialInd = (char *) malloc(strlen(ser)); //-->CAMBIO Aqui no es strlen en vez de sizeof?<--
	strcpy(serialInd,ser);
	nuevoVehiculo->codigo = *cod;
 	nuevoVehiculo->serial = serialInd;
 	nuevoVehiculo->Entrada = Ent;
 	nuevoVehiculo->siguiente = NULL;
 	nuevoVehiculo->tarifa = 0;

 	// Se actualiza el contador de vehiculo:
 	*cod = *cod + 1;
 	*id = *cod;

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

int buscarVehiculo(Vehiculo **inicioList, char *serial) {

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
		return 1; 
	}
	
	else {
		printf("NO ENCONTRADO\n");
		return 0;
	}


}

//----------------------------------------------------------------------------//

int eliminarVehiculo(Vehiculo **inicioList, char *serial, TiempoV tiempoS, char *bitacora, int *puestosOcupados) {

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

	return tarifaVehiculo;
	
}

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

//----------------------------------------------------------------------------//
//                       Inicio del código principal                          //
//----------------------------------------------------------------------------//

int main(int argc, char *argv[]){

	// Inicialización de variables: 
	int puestosOcupados = 0;
 	int contadorVehiculos = 0; // Global
	int i;
	char respuesta[60];
	Vehiculo *listaVehiculos = NULL;
	Host *listaHosts = NULL;

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
		else {
			printf("Error: Los parametros no siguen el formato correcto\n");
			exit(1);
		}
	}

	struct Skt skt;
	crearSocket(&skt,puerto,1);
	char buf[BUFFER_LEN];
	skt.addr_len = sizeof(struct sockaddr);
	printf("Esperando datos ....\n");

	while (1) {

		if ((skt.numbytes=recvfrom(skt.sockfd, buf, BUFFER_LEN, 0, 
						(struct sockaddr *)&(skt.their_addr), 
						(socklen_t *)&(skt.addr_len))) != -1) {
		buf[skt.numbytes] = '\0';

		printf("DIRECCION IP:%s",inet_ntoa(skt.their_addr.sin_addr));
		// Se calcula el tiempo de llegada del mensaje al servidor:
		time_t t1 = time(NULL);
		Tiempo tm1 = *localtime(&t1);

	 	//printf("MENSAJE\n");
		//printf("Fecha: %02d/%02d/%d \n",tm1.tm_mday,tm1.tm_mon + 1,tm1.tm_year + 1900);
		//printf("Hora: %02d:%02d:%02d \n", tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
		printf("MENSAJE: %s\n",buf);
		//printf("QUE %s\n",argumentos->entradas);
		//printf("TAL %s\n",argumentos->salidas);

		// Se crea la estructura de datos de argumentos del hilo:
		ArgumentoHilo *argumentos = (ArgumentoHilo *) malloc(sizeof(ArgumentoHilo)); 
		argumentos->buf = buf;
		argumentos->entradas = entradas;
		argumentos->salidas = salidas;
		argumentos->puestosOcupados = &puestosOcupados;
		argumentos->contadorVehiculos = &contadorVehiculos;
		argumentos->tiempoSegundos = t1;
		argumentos->tiempoFormato = tm1;
		argumentos->listaVehiculos = &listaVehiculos;
		argumentos->clientes = listaHosts;
		argumentos->origen = malloc(strlen(inet_ntoa(skt.their_addr.sin_addr)));
		strcpy(argumentos->origen,inet_ntoa(skt.their_addr.sin_addr));
		argumentos->skt = &skt;

		// Inicializacion de variables:
		ArgumentoHilo *argumentosBP = argumentos;
		const char separador[2] = "/";
		struct Skt *skt = argumentosBP->skt;
		char *operacion;
		char *tipoMensaje;
		char *num_secuencia;
		char *placa;
		int *confirmado;
		int  *puestosOcupados = argumentosBP->puestosOcupados;
		int  *contadorVehiculos = argumentosBP->contadorVehiculos;
		operacion = strtok(argumentosBP->buf,separador);
		int opcion = atoi(operacion);
		TiempoV tiempo1;
		tiempo1.tiempoF = tm1;
		tiempo1.segundos = t1;


		time_t t2 = t1 + 7201;
		Tiempo tm2 = *localtime(&t2);

		// Prueba
		Vehiculo **inicioList = argumentosBP->listaVehiculos;


		// prints
		// Se muestra en pantalla el tiempo actual:
		printf("Fecha: %02d/%02d/%d \n",tm1.tm_mday,tm1.tm_mon + 1,tm1.tm_year + 1900);
		printf("Hora: %02d:%02d:%02d \n", tm1.tm_hour, tm1.tm_min, tm1.tm_sec);

		// Si el servidor no esta en medio de una comunicacion con el mismo cliente
		printf("TIPO MENSAJE %d\n",opcion);
		// Verificacion de la operación (Entrada o salida):
	    switch(opcion) {

	    	// Entrada:
	    	case 0:

	    		// Verificacion de puestos disponibles:
				if (*puestosOcupados <= NUM_PUESTOS) {
					//printf("PUESTOS DISPONIBLES %d\n",NUM_PUESTOS-*puestosOcupados);
					placa = strtok(NULL,separador);

					int encontrado = buscarVehiculo(inicioList,placa);

					if (encontrado) {
						printf("EL VEHICULO YA ESTA\n");
						memset(respuesta,0,strlen(respuesta));
						strcat(respuesta, "3");

					}

					else {

						// Se busca vehiculo: 
						*puestosOcupados = *puestosOcupados + 1;

						int codigoVehiculo;

						// Se agrega el Vehiculo a la estructura:
						agregarVehiculo(inicioList,tiempo1,contadorVehiculos,placa,
										argumentosBP->entradas,&codigoVehiculo);
						// Se escribe la entrada en la Bitacora:
						//imprimirLista(inicioList);

						char id[10],dia[10],mes[10],anio[10],hora[10],minuto[10],segundo[10];
						sprintf(id,"%d",codigoVehiculo);
						sprintf(dia,"%d",tm1.tm_mday);
						sprintf(mes,"%d",tm1.tm_mon + 1);
						sprintf(anio,"%d",tm1.tm_year + 1900);
						sprintf(hora,"%d",tm1.tm_hour);
						sprintf(minuto,"%d",tm1.tm_min);
						sprintf(segundo,"%d",tm1.tm_sec);

						struct Parametros *p = (struct Parametros *)malloc(sizeof(struct Parametros));
						p->skt = skt;
						p->confirmado = confirmado;

						memset(respuesta,0,strlen(respuesta));

						char aceptado[2] = "1";
						
						strcat(respuesta,aceptado); // Esto es muy bestia, lo se. Luego lo acomodo
						strcat(respuesta,"/");
						strcat(respuesta,dia);
						strcat(respuesta,"/");
						strcat(respuesta,mes);
						strcat(respuesta,"/");
						strcat(respuesta,anio);
						strcat(respuesta,"/");
						strcat(respuesta,hora);
						strcat(respuesta,"/");
						strcat(respuesta,minuto);
						strcat(respuesta,"/");
						strcat(respuesta,segundo);
						strcat(respuesta,"/");
						strcat(respuesta,"ab1234");
						p->mensaje = respuesta;
						//reenviar(p);
					}

				}

				else {

					struct Parametros *p = (struct Parametros *)malloc(sizeof(struct Parametros));
					p->skt = skt;
					p->confirmado = confirmado;
					memset(respuesta,0,strlen(respuesta));
					strcat(respuesta, "0");
					p->mensaje = respuesta;

				}


				break;
			// Salida:
	    	case 1:

	    		// eliminar vehiculo:
	    		placa = strtok(NULL,separador);
	    		int tarifa; 
	    		tarifa = eliminarVehiculo(inicioList,placa,tiempo1,argumentosBP->salidas,puestosOcupados);


	    		if (tarifa == 0) {

	    			memset(respuesta,0,strlen(respuesta));
					strcat(respuesta, "4");

	    		}

	    		else {

	    			char str[15];
					sprintf(str, "%d",tarifa);

		    		memset(respuesta,0,strlen(respuesta));
					strcat(respuesta, "2");
					strcat(respuesta,"/");
					strcat(respuesta,str);
		    		//escribir Bitacora (salida);
		    		//escribirBitacora(argumentosBP->salidas,"s",carro1);
	    		}


	    		break;

	    	default:
	    		perror("Operación incorrecta\n");
	    		break;  
	    }

		printf("PUESTOS DISPONIBLES %d\n",NUM_PUESTOS-*puestosOcupados);
		free(argumentos); // Ver si hace falta

		}

		/* cerramos descriptor del skt */

		if ((skt.numbytes=sendto(skt.sockfd,respuesta,sizeof(respuesta),0,(struct sockaddr *)&(skt.their_addr),
		sizeof(struct sockaddr))) == -1) {
			perror("sendto");
			exit(2);
		}

	}

	close(skt.sockfd);
	exit(0);
}

//----------------------------------------------------------------------------//
//                         Fin del código principal                           //
//----------------------------------------------------------------------------//