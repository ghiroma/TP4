CC=g++
CFLAGS = -I ./src/includes -pthread
LFLAGS = -L ./src/libs -pthread

all:FuncionesServidorPartida.o ServidorPartida.o CommunicationSocket.o ServerSocket.o Felix.o Edificio.o Ventana.o Semaforo.o Helper.o
	$(CC) $(LFLAGS) -o ServidorPartida FuncionesServidorPartida.o ServidorPartida.o CommunicationSocket.o ServerSocket.o Felix.o Edificio.o Ventana.o Semaforo.o Helper.o
FuncionesServidorPartida.o:FuncionesServidorPartida.cpp
	$(CC) $(CFLAGS) -c FuncionesServidorPartida.cpp
ServidorPartida.o:ServidorPartida.cpp
	$(CC) $(CFLAGS) -c ServidorPartida.cpp
CommunicationSocket.o:Clases/CommunicationSocket.cpp
	$(CC) $(CFLAGS) -c Clases/CommunicationSocket.cpp
ServerSocket.o:Clases/ServerSocket.cpp
	$(CC) $(CFLAGS) -c Clases/ServerSocket.cpp
Felix.o:Clases/Felix.cpp
	$(CC) $(CFLAGS) -c Clases/Felix.cpp
Edificio.o:Clases/Edificio.cpp
	$(CC) $(CFLAGS) -c Clases/Edificio.cpp
Ventana.o:Clases/Ventana.cpp
	$(CC) $(CFLAGS) -c Clases/Ventana.cpp
Semaforo.o:Clases/Semaforo.cpp
	$(CC) $(CFLAGS) -c Clases/Semaforo.cpp
Helper.o:Support/Helper.cpp
	$(CC) $(CFLAGS) -c Support/Helper.cpp
