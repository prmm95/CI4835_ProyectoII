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
 *
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

 typedef struct operacion {
 	int tipo;
 	int fecha; // no es un int. Buscar como manejar fechas en C.
 	int hora; // no es un int. Buscar tambien como se manejan horas en C.
 	int codigoVehiculo;
 	int montoPagado;
 } Operacion;

//----------------------------------------------------------------------------//

int calcular_costo(Tiempo Entrada, Tiempo Salida) {
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

	// Inicialización de la tarifa
	int tarifa = 0;

	// Cobro de la primera hora;
	if (Salida.tm_sec - Entrada.tm_sec > 0) {
		tarifa += 80;
	}

	return tarifa;

}

//----------------------------------------------------------------------------//
//                       Inicio del código principal                          //
//----------------------------------------------------------------------------//

int main() {

 	// Inicialización de los puestos ocupados en el estacionamiento.
 	int puestosOcupados = 0;
 	int tarifaVehiculo;



 	// Se calcula el tiempo actual:
	time_t t = time(NULL);
	Tiempo tm1 = *localtime(&t);
	Tiempo tm2 = *localtime(&t);
	// Se muestra en pantalla el tiempo actual:
	printf("Fecha: %d/%d/%d \n",tm1.tm_mday,tm1.tm_mon + 1,tm1.tm_year + 1900);
	printf("Hora: %d:%d:%d \n", tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	tarifaVehiculo = calcular_costo(tm1,tm2);
	printf("La tarifa a pagar es %d \n",tarifaVehiculo);

	// Formato de impresión para el cliente al momento de entrara al 
	// estacionamiento:
	printf("-------------------\n");
	printf("Código: %d \n",1);
	printf("Fecha: %d/%d/%d \n",tm1.tm_mday,tm1.tm_mon + 1,tm1.tm_year + 1900);
	printf("Hora: %d:%d:%d", tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
	printf("\n-------------------\n");

 	return(0);
}

//----------------------------------------------------------------------------//
//                         Fin del código principal                           //
//----------------------------------------------------------------------------//