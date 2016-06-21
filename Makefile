#
# Archivo: Makefile
#
# Descripción:
#
# Autores:
#	Samuel Arleo, 10-10969.
#	Pablo Maldonado, 12-10561
#
# Última modificación: 27/06/2016
#

Main:
	gcc cliente.c -o sem_svr
	gcc servidor.c -o sem_cli

clean:
	rm sem_svr
	rm sem_cli