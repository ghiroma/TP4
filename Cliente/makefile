 CFLAGS= -g

Cliente: Cliente.o CommunicationSocket.o Helper.o FuncionesCliente.o
	g++ -o Cliente Cliente.o CommunicationSocket.o Helper.o FuncionesCliente.o -lSDL -lSDL_ttf -lpthread 
Cliente.o: Cliente.cpp Clases/CommunicationSocket.h Support/Constantes.h
	g++ -c Cliente.cpp -lSDL -lSDL_ttf -lpthread 
CommunicationSocket.o: Clases/CommunicationSocket.cpp 
	g++ -c Clases/CommunicationSocket.cpp  -lpthread 
Helper.o:Support/Helper.cpp
	g++ -c Support/Helper.cpp -lpthread
FuncionesCliente.o:FuncionesCliente.cpp
	g++ -c FuncionesCliente.cpp

.PHONY: clean
clean:
	rm -f *.o Cliente
