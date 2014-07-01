CC=g++
CFLAGS = -I ./src/includes
LFLAGS = -L ./src/libs 
LIBS = -pthread -lSDL -lSDL_ttf -lpthread 

all:FuncionesServidorTorneo.o ServidorTorneo.o Jugador.o CommunicationSocket.o ServerSocket.o Semaforo.o 
	$(CC) $(LFLAGS) -o ServidorTorneo FuncionesServidorTorneo.o ServidorTorneo.o Jugador.o CommunicationSocket.o ServerSocket.o Semaforo.o $(LIBS)
ServidorTorneo.o:ServidorTorneo.cpp
	$(CC) $(CFLAGS) -c ServidorTorneo.cpp $(LIBS)
FuncionesServidorTorneo.o:FuncionesServidorTorneo.cpp
	$(CC) $(CFLAGS) -c FuncionesServidorTorneo.cpp $(LIBS)
Jugador.o:Jugador.cpp
	$(CC) $(CFLAGS) -c Jugador.cpp $(LIBS)
CommunicationSocket.o:Clases/CommunicationSocket.cpp
	$(CC) $(CFLAGS) -c Clases/CommunicationSocket.cpp $(LIBS)
ServerSocket.o:Clases/ServerSocket.cpp
	$(CC) $(CFLAGS) -c Clases/ServerSocket.cpp $(LIBS)
Semaforo.o:Clases/Semaforo.cpp
	$(CC) $(CFLAGS) -c Clases/Semaforo.cpp $(LIBS)
