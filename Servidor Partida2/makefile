CC=g++
CFLAGS = -I ./src/includes -pthread -c
LFLAGS = -L ./src/libs -pthread -o

all:FuncionesServidorPartida.o ServidorPartida.o CommunicationSocket.o ServerSocket.o Felix.o Edificio.o Ventana.o Semaforo.o Helper.o Timer.o Mensajes.o Partida.o
	$(CC) $(LFLAGS) ServidorPartida FuncionesServidorPartida.o ServidorPartida.o CommunicationSocket.o ServerSocket.o Felix.o Edificio.o Ventana.o Semaforo.o Helper.o Mensaje.o Partida.o Timer.o
FuncionesServidorPartida.o:FuncionesServidorPartida.cpp
	$(CC) $(CFLAGS) FuncionesServidorPartida.cpp
ServidorPartida.o:ServidorPartida.cpp
	$(CC) $(CFLAGS) ServidorPartida.cpp
CommunicationSocket.o:Clases/CommunicationSocket.cpp
	$(CC) $(CFLAGS) Clases/CommunicationSocket.cpp
ServerSocket.o:Clases/ServerSocket.cpp
	$(CC) $(CFLAGS) Clases/ServerSocket.cpp
Felix.o:Clases/Felix.cpp
	$(CC) $(CFLAGS) Clases/Felix.cpp
Edificio.o:Clases/Edificio.cpp
	$(CC) $(CFLAGS) Clases/Edificio.cpp
Ventana.o:Clases/Ventana.cpp
	$(CC) $(CFLAGS) Clases/Ventana.cpp
Semaforo.o:Clases/Semaforo.cpp
	$(CC) $(CFLAGS) Clases/Semaforo.cpp
Helper.o:Support/Helper.cpp
	$(CC) $(CFLAGS) Support/Helper.cpp
Timer.o:Clases/Timer.cpp
	$(CC) $(CFLAGS) Clases/Timer.cpp
Partida.o:Clases/Partidas.cpp
	$(CC) $(CFLAGS) Clases/Partidas.cpp
Mensaje.o:Clases/Mensajes.cpp
	$(CC) $(CFLAGS) Clases/Mensajes.cpp

.PHONY: clean
clean:
	rm -f *.o
