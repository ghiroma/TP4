CFLAGS= -g
cliente: cliente_1.0.o CommunicationSocket.o
	g++ -o cliente cliente_1.0.o CommunicationSocket.o -lSDL -lSDL_ttf -lpthread 
cliente_1.0.o: cliente_1.0.cpp Clases/CommunicationSocket.h Support/Constantes.h
	g++ -c cliente_1.0.cpp -lSDL -lSDL_ttf -lpthread 
CommunicationSocket.o: Clases/CommunicationSocket.cpp 
	g++ -c Clases/CommunicationSocket.cpp  -lpthread 

.PHONY: clean
clean:
	rm -f *.o
