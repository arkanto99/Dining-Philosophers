/*Programacion del problema de los filósofos utilizando paso de mensajes*/

//Compilar con -pthread -lrt


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Libreria que incluye fork()
#include <pthread.h>//Libreria necesaria para el manejo de hilos
#include <mqueue.h> //Libreria que incluye las funciones de paso de mensajes
#include <fcntl.h>  //Incluye las flags del tipo O_*
#include <sys/wait.h> //Libreria en la cual se encuentra la funcion wait
#include <string.h> //Necesaria para manejar cadenas

//Definimos colores para facilitar la lectura de  resultados
#define BLACK "\033[22;30m"
#define WHITE "\033[01;37m"
#define BROWN "\033[22;33m"
#define YELLOW "\033[01;33m"
#define RED "\033[22;31m"
#define LIGHTRED "\033[01;31m"
#define GREEN "\033[22;32m"
#define LIGHTGREEN "\033[01;32m"
#define BLUE "\033[22;34m"
#define LIGHTBLUE "\033[01;34m"
#define MAGENTA "\033[22;35m"
#define LIGHTMAGENTA "\033[01;35m"
#define CYAN "\033[22;36m"
#define LIGHTCYAN "\033[01;36m"
#define GRAY "\033[22;37m"
#define DARKGRAY "\033[01;30m"

#define TRUE 1
#define FALSE 0

#define PENSANDO 0
#define HAMBRIENTO 1
#define COMIENDO 2

int N=0; //Numero de filosofos. En la solucion de Tanenbaum es una constante, en esta no pues se nos pide que el usuario elija el numero al iniciar el programa
int *estado; //Array que lleva registro del estado de todos los filosofos
mqd_t *almacenFilosofo;
mqd_t almacenCamarero;
char *tenedores;

//Cadena que contiene los colores disponibles
char *colors[]={GREEN,BLUE,YELLOW,RED,MAGENTA,CYAN,GRAY,WHITE,LIGHTRED,LIGHTGREEN,LIGHTBLUE,LIGHTMAGENTA,LIGHTCYAN,DARKGRAY,BLACK,BROWN};

void *camarero();
void *filosofo(void *ptr_numero);

int main(int argc, char * argv[]){//Argumentos:  nºFilosofos

  //Obtencion de parametros
	if(argc!=2){
    printf("Numero de parametros invalidos: Introduzca el numero de filosofos");
    exit(EXIT_FAILURE);
  }
  else{
    N=atoi(argv[1]);//Numero de filosofos
  }

  pthread_t filosofos[N]; //Array de filosofos
	pthread_t hiloCamarero; ///Hilo camarero
  //Reserva de memoria
  estado=(int *)malloc(N*sizeof(int));
  almacenFilosofo=(mqd_t *)malloc(N*sizeof(mqd_t));
  tenedores=(char *)malloc(N*sizeof(char));
  //Inicializacion de colas de mensajes

	struct mq_attr attr;
	/* Atributos de la cola */
	attr.mq_maxmsg = N; //Tamaño de la cola
	attr.mq_msgsize = 2*sizeof(char); //Tamaño de cada mensaje enviado
	/* Borrado de los buffers de entrada por si existían de una ejecución previa*/

  char name[10]; //Variable para almacenar el nombre del semaforo
	char aux[2];
  for(int i=0;i<N;i++){
		memset(name, 0, sizeof(name)); //Reiniciamos la cadena
		strcat(name,"/ALMACENF"); //Concatenamos con un sufijo generico
    sprintf(aux,"%d",i); //Pasamos el indice a la cadena
    strcat(name,aux); //Concatenamos con un sufijo generico
		mq_unlink(name); // Eliminamos posibles semaforos previos con el mismo nombre
		almacenFilosofo[i] = mq_open(name, O_CREAT|O_RDWR, 0777, &attr);
		if (almacenFilosofo[i] == -1) {
			perror ("mq_open");
			exit(EXIT_FAILURE);
		}
		estado[i]=0;
    tenedores[i]=i+65; //Codigo ASCII del abecedario
  }

	mq_unlink("/ALMACENC");
	almacenCamarero = mq_open("/ALMACENC", O_CREAT|O_RDWR, 0777, &attr);
	if (almacenCamarero == -1) {
    perror ("mq_open");
    exit(EXIT_FAILURE);
  }
  //'Presentamos' la mesa
  printf("%s\n",WHITE);
  printf("-Numeros: Filosofos -Letras: Tenedores\nEl ultimo tenedor se comparte con el primer filosofo.");
  printf("La disposicion de la mesa es la siguiente:\n          ");
  for(int i=0;i<N;i++){
    printf("%d-%c-",i,tenedores[i]);
  }
  printf("\n\n\n");
	//Creacion del camarero
	pthread_create(&hiloCamarero,0,camarero,0);
  //Creacion de filosofos
  int numeroHilo[N];
  for(int i=0;i<N;i++){
    numeroHilo[i]=i;
    pthread_create(&filosofos[i],0,filosofo,&numeroHilo[i]);
  }

  //Esperamos a que terminen los filosofos. El camarero se cierra automaticamente al acabar el proceso principal
  for(int i=0;i<N;i++){
    pthread_join(filosofos[i],0);
  }

  mq_close(almacenCamarero);
  for(int i=0;i<N;i++){
    mq_close(almacenFilosofo[i]);
  }

  return EXIT_SUCCESS;
}

void pensar(int nF){
	sleep(rand()%3);
}

void comer(int nF){
	sleep(rand()%4);
}

void tomar_tenedores(int nF){
	char mensaje[2], vacio[0];
	sprintf(mensaje,"%d",nF); //Pasamos el numero a la cadena
  estado[nF]=HAMBRIENTO;
  printf("%s El filosofo %d esta hambriento\n",colors[nF%15],nF);
	mq_send(almacenCamarero,mensaje,sizeof(mensaje),0); //Enviamos el mensaje al camarero
	mq_receive(almacenFilosofo[nF],vacio,sizeof(mensaje),0); //Esperamos a que llegue un mensaje vacio
}

void poner_tenedores(int nF,int IZQUIERDO){


  char *color= colors[nF%15];
	char mensaje[2], vacio[0];
	sprintf(mensaje,"%d",nF); //Pasamos el numero a la cadena
  estado[nF]=PENSANDO;
	printf(" %s El filosofo %d pone los tenedores %c y %c disponibles\n",color,nF,tenedores[IZQUIERDO],tenedores[nF]);
	printf(" %s El filosofo %d esta pensando\n",color,nF);
	mq_send(almacenCamarero,mensaje,sizeof(mensaje),0); //Enviamos el mensaje al camarero
	mq_receive(almacenFilosofo[nF],vacio,sizeof(mensaje),0); //Esperamos a que llegue un mensaje vacio

}

void *camarero(){

		int filosofosEsperando[N];
		int ultimo=0;
		int nF; int IZQUIERDO; int DERECHO;
		int tenedoresLibres[N];
		int salir; //Utilizada para salir de un bucle
		char mensaje[2];

		for(int i=0;i<N;i++){ //Inicialmente todos los tenedores estan libres
			tenedoresLibres[i]=TRUE;
		}

		while(TRUE){

			mq_receive(almacenCamarero,mensaje,sizeof(mensaje),0); //Esperamos a que llegue un mensaje vacio
			//Extraemos el mensaje;
			nF=atoi(mensaje);
			char *color= colors[nF%15];
			IZQUIERDO = (nF+N-1)%N;
			DERECHO = nF;

			if(estado[nF]== HAMBRIENTO){ //Si esta hambriento, el mensaje es para pedir tenedores
				if(tenedoresLibres[IZQUIERDO]==TRUE && tenedoresLibres[DERECHO]==TRUE){ //Comprobamos si ambos tenedores estan tenedoresLibres
					tenedoresLibres[IZQUIERDO]=FALSE; //Marcamos los tenedores como ocupados
					tenedoresLibres[DERECHO]=FALSE;
					estado[nF]=COMIENDO; //Cambiamos el estado del filosofo
					printf("%s El filosofo %d toma los tenedores %c y %c\n",color,nF,tenedores[IZQUIERDO],tenedores[DERECHO]);
					printf("%s El filosofo %d esta comiendo\n",color,nF);
					mq_send(almacenFilosofo[nF],mensaje,sizeof(mensaje),0); //Indicamos al filosofo que puede comer
				}
				else{ //Si no estan disponibles los tenedores, añadimos el filosofo a la cola de espera
					filosofosEsperando[ultimo]=nF;
					++ultimo;
				}
			}
			else{ //Si no se esta hambriento significa que el mensaje es para dejar los tenedores
				tenedoresLibres[IZQUIERDO]=TRUE; //Marcamos los tenedores como disponibles
				tenedoresLibres[DERECHO]=TRUE;
				mq_send(almacenFilosofo[nF],mensaje,sizeof(mensaje),0); //Indicamos al filosofo que puede dejar la mesa
				//Enviamos un mensaje uno de los filosofos que estaba esperando por un tenedor
				salir=0;
				for(int i=0;i<ultimo && salir==0;i++){
					IZQUIERDO = (filosofosEsperando[i]+N-1)%N;
					DERECHO = filosofosEsperando[i];
					if(tenedoresLibres[IZQUIERDO]==TRUE && tenedoresLibres[DERECHO]==TRUE){
						tenedoresLibres[IZQUIERDO]=FALSE;//Marcamos los tenedores como ocupados
						tenedoresLibres[DERECHO]=FALSE;
						estado[nF]=COMIENDO; //Cambiamos el estado del filosofo
						printf("%s El filosofo %d toma los tenedores %c y %c\n",colors[filosofosEsperando[i]%15],filosofosEsperando[i],tenedores[IZQUIERDO],tenedores[DERECHO]);
						printf("%s El filosofo %d esta comiendo\n",colors[filosofosEsperando[i]%15],filosofosEsperando[i]);
						mq_send(almacenFilosofo[filosofosEsperando[i]],mensaje,sizeof(mensaje),0); //Indicamos al filosofo que puede comer
						for(int j=i+1;j<ultimo;j++){//Reorganizamos la cola de espera
							filosofosEsperando[j-1]=filosofosEsperando[j];
						}
						--ultimo; //Hay un filosofo menos en la cola
						salir=1;
					}
				}
			}
		}
}

void *filosofo(void *ptr_numero){

  int numFilosofo=*(int*)ptr_numero;
  int IZQUIERDO = (numFilosofo+N-1)%N;
  srand(numFilosofo);
	int iteraciones=5;

  while(iteraciones--){
		pensar(numFilosofo);
		tomar_tenedores(numFilosofo);
    comer(numFilosofo);
    poner_tenedores(numFilosofo,IZQUIERDO);
  }
	return 0;
}
