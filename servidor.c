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
#define NUM_THREADS 3
#define BUFFER_LEN 1024

//----------------------------------------------------------------------------//
//                      Definición de variables globales                      //
//----------------------------------------------------------------------------//

// Descripción:
pthread_mutex_t semaforoListaVehiculos = PTHREAD_MUTEX_INITIALIZER;

// Descripción:
pthread_mutex_t semaforoListaSecuencias = PTHREAD_MUTEX_INITIALIZER;

// Descripción:
pthread_mutex_t semaforoBitacora = PTHREAD_MUTEX_INITIALIZER;

// Descripción:
pthread_mutex_t semaforoPuestosOcupados = PTHREAD_MUTEX_INITIALIZER;

// Descripción:
pthread_mutex_t semaforoCodigoVehiculo = PTHREAD_MUTEX_INITIALIZER;

// Descripción:
pthread_mutex_t semaforoListaClientes = PTHREAD_MUTEX_INITIALIZER;

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

typedef struct Host
{
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
	Tiempo fechaB;
	
	// No se si este if se puede hacer mas elegante:
	if (tipoOperacion == "e") {
		fechaB = vehiculo.Entrada.tiempoF;
	}
	
	else {
		fechaB = vehiculo.Salida.tiempoF;
	}
	
	pthread_mutex_lock(&semaforoBitacora);
	bitacora = fopen(rutaBitacora,"a"); // no se si pasar esto como parametro para no abrir y cerrar el df cada vez
	fprintf(bitacora,"-------------------\n");
	fprintf(bitacora,"Fecha: %d/%02d/%d \n",fechaB.tm_mday,fechaB.tm_mon + 1,fechaB.tm_year + 1900);
	fprintf(bitacora,"Hora: %02d:%02d:%02d\n", fechaB.tm_hour, fechaB.tm_min, fechaB.tm_sec);
	fprintf(bitacora,"Serial: %s \n",vehiculo.serial);
	fprintf(bitacora,"Código: %d",vehiculo.codigo);
	fprintf(bitacora,"\n-------------------\n");	
	fclose(bitacora);
	pthread_mutex_unlock(&semaforoBitacora);
	
}


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

 	pthread_mutex_lock(&semaforoCodigoVehiculo);
 	// Se actualiza el contador de vehiculo:
 	*cod = *cod + 1;
 	*id = *cod;
 	pthread_mutex_unlock(&semaforoCodigoVehiculo);


 	pthread_mutex_lock(&semaforoListaVehiculos);
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
 	pthread_mutex_unlock(&semaforoListaVehiculos);

 	escribirBitacora(bitacora,"e",*nuevoVehiculo);
}

//----------------------------------------------------------------------------//

void eliminarVehiculo(Vehiculo **inicioList, char *serial, TiempoV tiempoS, char *bitacora, int *puestosOcupados) {

	Vehiculo *aux = *inicioList;
	Vehiculo *anterior = NULL;
	int encontrado = 0;
	int comparador;
	int tarifaVehiculo = 0;


	pthread_mutex_lock(&semaforoListaVehiculos);
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
	pthread_mutex_unlock(&semaforoListaVehiculos);


	// Con el vehiculo a eliminar, se acualiz
	if (encontrado) {
		printf("ENCONTRADO \n");

		aux->Salida = tiempoS;
		tarifaVehiculo = calcular_costo(*aux);
		printf("LA TARIFA ES: %d\n",tarifaVehiculo);
		escribirBitacora(bitacora,"s",*aux);

		pthread_mutex_lock(&semaforoListaVehiculos);
		free(aux);
		pthread_mutex_unlock(&semaforoListaVehiculos);

		pthread_mutex_lock(&semaforoPuestosOcupados);
		*puestosOcupados = *puestosOcupados -1;
		pthread_mutex_unlock(&semaforoPuestosOcupados);
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

// Busca un cliente por su direccion IP en la lista de clientes y retorna
// en "h" la direccion del cliente buscado en caso de encontrarlo.
int getCliente(Host *clientes,char *dir_origen,Host *h){

	pthread_mutex_lock(&semaforoListaClientes);
	Host *aux = clientes;
	while (aux != NULL){
		if (strcmp(aux->ip,dir_origen) == 0){
			h = aux;
			return 1;
		}
		else{
			aux = aux->siguiente;
		}
	}
	pthread_mutex_lock(&semaforoListaClientes);
	return 0;
}

void agregarCliente(Host *clientes, Host *h){

	pthread_mutex_lock(&semaforoListaClientes);
	Host *aux = clientes;
	while (aux != NULL){
		aux = aux->siguiente;
	}
	aux = h;
	pthread_mutex_lock(&semaforoListaClientes);
}

void crearCliente(int num_secuencia,char *dir_origen,Host *cliente){

	cliente = (Host *)malloc(sizeof(Host));
	cliente->confirmado = 0;
	cliente->num_secuencia = num_secuencia;
	cliente->ip = (char *)malloc(strlen(dir_origen));
	strcpy(cliente->ip,dir_origen);
	cliente->siguiente = NULL;

}

// Determina si el servidor posee una conexion abierta con el cliente indicado
// Si no existia conexion con el host o no hay sesiones abiertas, agrega al
// cliente a la lista de hosts 

int sesionAbierta(Host *clientes,char *dir_origen,int num_secuencia,int *confirmado){

	Host *cliente;
	if (getCliente(clientes,dir_origen,cliente) == 0){
		crearCliente(num_secuencia,dir_origen,cliente);
		confirmado = &(cliente->confirmado);
		agregarCliente(clientes,cliente); // Colocar confirmado en 0
		return 0; // No habia una sesion abierta
	}else{
		if (cliente->num_secuencia == 0){ //Ya ha habido comunicacion con el mismo cliente
			cliente->num_secuencia = num_secuencia; // pero no hay una sesion actualmente
			confirmado = &(cliente->confirmado);
			return 0;
		}else{
			return 1; // Cambiar num_secuencia a 0 cuando se confirme el mensaje en el case 2 linea 455
		}
	}
}

//----------------------------------------------------------------------------//

void *beginProtocol(void *argumentos) {

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
	//tipoMensaje = strtok(NULL,separador); Este campo se elimino
	num_secuencia = strtok(NULL,separador);
 	int opcion = atoi(operacion);

 	//char *placa = placaStr;


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
	printf("Fecha: %02d/%02d/%d \n",tm1.tm_mday,tm1.tm_mon + 1,tm1.tm_year + 1900);
	printf("Hora: %02d:%02d:%02d \n", tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	// printf("HOLA %s\n",argumentosBP->buf);
	// printf("QUE %s\n",argumentosBP->entradas);
	// printf("TAL %s\n",argumentosBP->salidas);
	// printf("el paquete contiene: %s\n", argumentosBP->buf);
	// printf("La operacion es -> %s\n",operacion);
	// printf("El tipo de mensaje es -> %s\n",tipoMensaje);
	// printf("El numero de secuencia es es -> %s\n",numeroSecuencia);
	//printf("La placa es -> %s\n",placa);
	// printf("El numero de puestos ocupados es -> %d\n",*puestosOcupados);
	// printf("El codigo del Vehiculo es -> %d\n",*argumentosBP->contadorVehiculos);

	// Si el servidor no esta en medio de una comunicacion con el mismo cliente
	printf("TIPO MENSAJE %d\n",opcion);
	if (!sesionAbierta(argumentosBP->clientes,argumentosBP->origen,
									atoi(num_secuencia),confirmado)){
		// Verificacion de la operación (Entrada o salida):
	    switch(opcion) {

	    	// Entrada:
	    	case 0:
	    		// Verificacion de puestos disponibles:
				if (*puestosOcupados <= NUM_PUESTOS) {
					//printf("PUESTOS DISPONIBLES %d\n",NUM_PUESTOS-*puestosOcupados);
					placa = strtok(NULL,separador);
					pthread_mutex_lock(&semaforoPuestosOcupados);
					*puestosOcupados = *puestosOcupados + 1;
					pthread_mutex_unlock(&semaforoPuestosOcupados);

					int codigoVehiculo;

					// Se agrega el Vehiculo a la estructura:
					agregarVehiculo(inicioList,tiempo1,contadorVehiculos,placa,
									argumentosBP->entradas,&codigoVehiculo);
					// Se escribe la entrada en la Bitacora:
					imprimirLista(inicioList);

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
					char respuesta[60];
					strcat(respuesta, "1"); // Esto es muy bestia, lo se. Luego lo acomodo
					strcat(respuesta,"/");
					strcat(respuesta,num_secuencia);
					strcat(respuesta,"/");
					strcat(respuesta,id);
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
					p->mensaje = respuesta;
					reenviar(p);
				}

				else {
					struct Parametros *p = (struct Parametros *)malloc(sizeof(struct Parametros));
					p->skt = skt;
					p->confirmado = confirmado;
					char respuesta[30];
					strcat(respuesta, "0");
					strcat(respuesta,"/");
					strcat(respuesta,num_secuencia);
					p->mensaje = respuesta;
					reenviar(p);
				}
				break;
			// Salida:
	    	case 1:
	    		//carro1.Salida.tiempoF = tm2;
	 			//carro1.Salida.segundos = t2
	    		// eliminar vehiculo:
	    		placa = strtok(NULL,separador);
	    		eliminarVehiculo(inicioList,placa,tiempo1,argumentosBP->salidas,puestosOcupados);
	    		// escribir Bitacora (salida);
	    		//escribirBitacora(argumentosBP->salidas,"s",carro1);
	    		break;
	    	// ACK
	    	case 2:
	    		printf("HOLAAAAAAAA OPCION 0");
	    		Host *h;
	    		pthread_mutex_lock(&semaforoListaClientes);
	    		int encontrado = getCliente(argumentosBP->clientes,argumentosBP->origen,h);
	    		if (encontrado){
	    			h->confirmado = 0;
	    			h->num_secuencia = 0;
	    		}
	    		pthread_mutex_lock(&semaforoListaClientes);
	    		break;

	    	default:
	    		perror("Operación incorrecta\n");
	    		break;  
	    }
	}
    printf("PUESTOS DISPONIBLES %d\n",NUM_PUESTOS-*puestosOcupados);
    free(argumentos); // Ver si hace falta
	
}

//----------------------------------------------------------------------------//
//                       Inicio del código principal                          //
//----------------------------------------------------------------------------//

int main(int argc, char *argv[]){

	// Inicialización de variables: 
	int puestosOcupados = 0;
 	int contadorVehiculos = 0; // Global
	int i;
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
	}

	struct Skt skt;
	crearSocket(&skt,puerto,1);
	char buf[BUFFER_LEN];
	pthread_t threads[NUM_THREADS];
	int num_hilos = 0;
	int rc;
	skt.addr_len = sizeof(struct sockaddr);
	printf("Esperando datos ....\n");
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

	if ((skt.numbytes=sendto(skt.sockfd,"2/1000",sizeof("2/1000"),0,(struct sockaddr *)&(skt.their_addr),
	sizeof(struct sockaddr))) == -1) {
		perror("sendto");
		exit(2);
	}

	imprimirLista(&listaVehiculos);

	close(skt.sockfd);
	pthread_exit(NULL);
	exit (0);
}

//----------------------------------------------------------------------------//
//                         Fin del código principal                           //
//----------------------------------------------------------------------------//