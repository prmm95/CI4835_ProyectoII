#
# Archivo: Makefile
#
# Descripción: Archivo que permite realizar la compilación de los archivos 
# 'cliente.c' y 'servidor.c' para crear los ejecutables 'sem_cli' y 'sem_srv' 
# respectivamente. A su vez, permite eliminar los mismos al ejecutar el 
# comando 'make clean' en su terminal.
#
# Autores:
#	Samuel Arleo, 10-10969.
#	Pablo Maldonado, 12-10561
#
# Última modificación: 27/06/2016
#

Main:
	gcc cliente.c lib_socket.c -o sem_cli
	gcc servidor.c lib_socket.c -lm -o sem_svr

clean:
	rm sem_svr
	rm sem_cli
