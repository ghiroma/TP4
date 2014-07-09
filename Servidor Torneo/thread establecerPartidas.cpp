/**
 * THREAD -> Crea los servidores de partidas
 */
/*
void* establecerPartidas(void* data) {
	cout << "Thread establecerPartidas - PID:" << getpid() << endl;
	pid_t pid;
	int idJugador;
	int idOponente;
	int nroPartida = 1;
	string nombreSemaforo;
	int i = 1;
	while (!torneoFinalizado()) {
		//cout << "pasada de busqueda nro: " << i++ << " PID:" << getpid() << endl;

		//recorro la lista de jugadores viendo a quien le puedo asignar un oponente y que comienze la partida
		//cout << "mutex establecerPartidas" << endl;
		pthread_mutex_lock(&mutex_listJugadores);
		for (map<int, Jugador*>::iterator it = listJugadores.begin(); it != listJugadores.end(); it++) {
			idJugador = it->first;
			idOponente = it->second->obtenerOponente();

			//si no se encuentra jugando actualmente y se encontro un oponente lanzo el servidor de partida
			if (idOponente > 0) {
				//habilito el temporizador del torneo
				if (nroPartida == 1) {
					//cout << "Se crea la primer partida y doy permiso a iniciar el temporizador" << endl;
					sem_inicializarTemporizador.V();
				}
				nroPartida++;

				puertoPartida++;
				key_t key = ftok("/bin/ls", puertoPartida);
				if (key == -1) {
					cout << "Error al generar clave de memoria compartida" << endl;
					break;
				}
				//cout << "ftok key generada: " << key << endl;
				int idShm = shmget(key, sizeof(struct puntajesPartida) * 1,
				IPC_CREAT | PERMISOS_SHM);
				if (idShm == -1) {
					cout << "Error al obtener memoria compartida" << endl;
					break;
				}

				//Le mando a los jugadores el nro de Puerto en el que comenzara la partida
				char auxPuertoNuevaPartida[LONGITUD_CONTENIDO];
				sprintf(auxPuertoNuevaPartida, "%d", puertoPartida);
				string message(CD_PUERTO_PARTIDA);
				message.append(fillMessage(auxPuertoNuevaPartida));
				cout << "le mando a ID: " << idJugador << " - el socket:" << auxPuertoNuevaPartida << endl;
				listJugadores[idJugador]->SocketAsociado->SendBloq(message.c_str(), message.length());
				cout << "le mando a ID: " << idOponente << " - el socket:" << auxPuertoNuevaPartida << endl;
				listJugadores[idOponente]->SocketAsociado->SendBloq(message.c_str(), message.length());

				//Les mando el nombre de su oponente
				char auxnombreOponente1[LONGITUD_CONTENIDO];
				sprintf(auxnombreOponente1, "%s", listJugadores[idOponente]->Nombre.c_str());
				string nombreOponente1(CD_NOMBRE);
				nombreOponente1.append(fillMessage(auxnombreOponente1));
				char auxnombreOponente2[LONGITUD_CONTENIDO];
				sprintf(auxnombreOponente2, "%s", listJugadores[idJugador]->Nombre.c_str());
				string nombreOponente2(CD_NOMBRE);
				nombreOponente2.append(fillMessage(auxnombreOponente2));
				cout << "le mando a ID: " << idJugador << " - el nombre oponente:" << nombreOponente1 << endl;
				listJugadores[idJugador]->SocketAsociado->SendBloq(nombreOponente1.c_str(), nombreOponente1.length());
				cout << "le mando a ID: " << idOponente << " - el nombre oponente:" << nombreOponente2 << endl;
				listJugadores[idOponente]->SocketAsociado->SendBloq(nombreOponente2.c_str(), nombreOponente2.length());
				cout << "Termino de mandar los nombres de oponentes" << endl;

				if ((pid = fork()) == 0) {
					//Proceso hijo
					cout << "Thread establecerPartidas FORK - PID:" << getpid() << " (" << idJugador << "vs" << idOponente << ") socket:" << auxPuertoNuevaPartida << endl;

					char auxCantVidas[2];
					sprintf(auxCantVidas, "%d", cantVidas);

					char *argumentos[] = { auxPuertoNuevaPartida, auxCantVidas, NULL };
					execv("../Servidor Partida/Debug/Servidor Partida", argumentos);
					cout << "ERROR al ejecutar execv Nueva Partida" << endl;
					exit(1);
				} else if (pid < 0) {
					//Hubo error
					cout << "Error al forkear" << endl;
				} else {
					//Soy el padre.
					//inicializo el BLOQUE DE SHM
					puntajesPartida* resumenPartida = (struct puntajesPartida *) shmat(idShm, (char *) 0, 0);
					resumenPartida->idJugador1 = -1;
					resumenPartida->idJugador2 = -1;
					resumenPartida->puntajeJugador1 = 0;
					resumenPartida->puntajeJugador2 = 0;
					resumenPartida->partidaFinalizadaOK = false;

					//genero y cargo los datos de la partida en una lista
					datosPartida structuraDatosPartida;
					structuraDatosPartida.idShm = idShm;
					structuraDatosPartida.pidPartida = pid;

					//cout << "mutex establecerPartidas partidasActivas" << endl;
					pthread_mutex_lock(&mutex_partidasActivas);
					partidasActivas.push_back(structuraDatosPartida);
					pthread_mutex_unlock(&mutex_partidasActivas);
					//cout << "unmutex establecerPartidas partidasActivas" << endl;
				}
			} else {
				////cout << "J" << idJugador << " No puede jugar" << endl;
			}
		}
		pthread_mutex_unlock(&mutex_listJugadores);
		//cout << "unmutex establecerPartidas" << endl;

		usleep(INTERVALO_ENTRE_BUSQUEDA_DE_OPONENTES);
	}

	//cout << "Thread EstablecerPartidas va a hacer un Exit" << endl;
	pthread_exit(NULL);
}
*/
