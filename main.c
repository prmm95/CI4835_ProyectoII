/*
 * Archivo: main.c
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
 * Referencias:
 * 1.- http://stackoverflow.com/questions/1442116/how-to-get-date-and-time-value-in-c-program
 * 2.- http://en.cppreference.com/w/c/chrono/difftime
 *
 *
 *
 *
 * 
*/

//----------------------------------------------------------------------------//
//                        Directivas de preprocesador                         //
//----------------------------------------------------------------------------//

#include <stdio.h> // Uso de la entrada/salida estándar (I/O).
#include <stdlib.h> 
#include <math.h>
#include <time.h>  //
#include <errno.h> // Uso de la función perror()


//----------------------------------------------------------------------------//
//                          Definición de constantes                          //
//----------------------------------------------------------------------------//

#define NUMERO_PUESTOS 200 // Número de puestos del estacionamiento.

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
 	int serial; // cambiar, porque no es int.
 	int tarifa;
 	struct vehiculo *siguiente;
 } Vehiculo;

//----------------------------------------------------------------------------//
//                          Definición de funciones                           //
//----------------------------------------------------------------------------//

// Notas:
// borrar de la lista al vehiculo cuando sale

void agregarVehiculo(Vehiculo **lisVehic,TiempoV Ent, TiempoV Sal, int *cod, int ser) {
			
	// Se crea el nuevo vehiculo que se agregara a la lista enlazada:
	Vehiculo *nuevoVehiculo = (Vehiculo *) malloc(sizeof(Vehiculo));
	nuevoVehiculo->codigo = *cod;
 	nuevoVehiculo->serial = ser;
 	nuevoVehiculo->Entrada = Ent;
 	nuevoVehiculo->Salida = Sal;
 	nuevoVehiculo->siguiente = NULL;
 	// Se actualiza el contador de vehiculo:
 	*cod = *cod + 1;

 	// En caso de agregar el primer vehiculo a la lista:
 	if (*lisVehic == NULL) {
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
 
}

//----------------------------------------------------------------------------//

void eliminarVehiculo(Vehiculo **inicioList, int cod) {

	
	
}

//----------------------------------------------------------------------------//

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
	if (tipoOperacion == "Entrada") {
		fechaB = vehiculo.Entrada.tiempoF;
	}
	
	else {
		fechaB = vehiculo.Salida.tiempoF;
	}
	
	
	fprintf(bitacora,"-------------------\n");
	fprintf(bitacora,"Fecha: %d/%02d/%d \n",fechaB.tm_mday,fechaB.tm_mon + 1,fechaB.tm_year + 1900);
	fprintf(bitacora,"Hora: %02d:%02d:%02d\n", fechaB.tm_hour, fechaB.tm_min, fechaB.tm_sec);
	fprintf(bitacora,"Serial: %d \n",vehiculo.serial);
	fprintf(bitacora,"Código: %d",vehiculo.codigo);
	fprintf(bitacora,"\n-------------------\n");
	
	close(bitacora);
	
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

int main() {

 	// Inicialización de los puestos ocupados en el estacionamiento.
 	int puestosOcupados = 0;
 	int codigoVehiculo = 0; // Global
 	int tarifaVehiculo;

 	// Se calcula el tiempo actual:
	time_t t1 = time(NULL);
	Tiempo tm1 = *localtime(&t1);
	
    tm1.tm_hour = 0;
    tm1.tm_min = 0;
    tm1.tm_sec = 0;
    tm1.tm_mday = 22;
	
	//sleep(10);
	time_t t2 = t1 + 7201;
	Tiempo tm2 = *localtime(&t2);
	// Se muestra en pantalla el tiempo actual:
	printf("Fecha: %02d/%02d/%d \n",tm1.tm_mday,tm1.tm_mon + 1,tm1.tm_year + 1900);
	printf("Hora: %02d:%02d:%02d \n", tm1.tm_hour, tm1.tm_min, tm1.tm_sec);


	// Formato de impresión para el cliente al momento de entrar al 
	// estacionamiento: (esto lo haria el cliente)
	//printf("-------------------\n");
	//printf("Código: %d \n",1);
	//printf("Fecha: %02d/%02d/%d \n",tm1.tm_mday,tm1.tm_mon + 1,tm1.tm_year + 1900);
	//printf("Hora: %02d:%02d:%02d", tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	//printf("\n-------------------\n");
	
	printf("-------------------\n");
	//printf("Código: %d \n",1);
	printf("Fecha: %02d/%02d/%d \n",tm2.tm_mday,tm2.tm_mon + 1,tm2.tm_year + 1900);
	printf("Hora: %02d:%02d:%02d", tm2.tm_hour, tm2.tm_min, tm2.tm_sec);
	printf("\n-------------------\n");

	// Prueba
	Vehiculo carro1;
	carro1.Entrada.tiempoF = tm1;
	carro1.Entrada.segundos = t1;
 	carro1.Salida.tiempoF = tm2;
 	carro1.Salida.segundos = t2;
 	carro1.codigo = 123;
 	carro1.serial = 456;
 	carro1.tarifa = 0;
	char *rutaBitacora;
	rutaBitacora = "hola";
	
	TiempoV tiempo1;
	tiempo1.tiempoF = tm1;
	tiempo1.segundos = t1;
	TiempoV tiempo2;
	tiempo2.tiempoF = tm2;
	tiempo2.segundos = t2;
	
	// antes de salir, hay que calcular tarifa.
	escribirBitacora(rutaBitacora,"Entrada",carro1);
	
	tarifaVehiculo = calcular_costo(carro1);
	//printf("La tarifa a pagar es %d \n",tarifaVehiculo);
	
	Vehiculo *inicioList = NULL;
	
	agregarVehiculo(&inicioList,tiempo1,tiempo2,&codigoVehiculo,0);
	agregarVehiculo(&inicioList,tiempo1,tiempo2,&codigoVehiculo,1);
	agregarVehiculo(&inicioList,tiempo1,tiempo2,&codigoVehiculo,2);
	agregarVehiculo(&inicioList,tiempo1,tiempo2,&codigoVehiculo,3);
	agregarVehiculo(&inicioList,tiempo1,tiempo2,&codigoVehiculo,4);
	agregarVehiculo(&inicioList,tiempo1,tiempo2,&codigoVehiculo,5);

 	return(0);
}

//----------------------------------------------------------------------------//
//                         Fin del código principal                           //
//----------------------------------------------------------------------------//