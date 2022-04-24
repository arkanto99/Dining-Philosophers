/*Programacion del problema de los filósofos utilizando mutexes y variables de condicion*/

//Compilar con -pthread

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Libreria que incluye fork()
#include <sys/stat.h> //Biblioteca donde se encuentran los modos de creacion del semafoto
#include <fcntl.h> //Biblioteca donde se encuentran  las flags necesarias para crear el semaforo
#include <pthread.h>//Libreria necesaria para el uso de hilos
#include <semaphore.h>
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
#define PENSANDO 0
#define HAMBRIENTO 1
#define COMIENDO 2

int N=0; //Numero de filosofos. En la solucion de Tanenbaum es una constante, en esta no pues se nos pide que el usuario elija el numero al iniciar el programa
int *estado; //Array que lleva registro del estado de todos los filosofos
pthread_mutex_t mutex;//Toma los valores 0 o 1.Garantiza la exclusion mutua
pthread_cond_t *condicion; //Una variable de condicion por filosofo
char *tenedores;


//Cadena que contiene los colores disponibles
char *colors[]={GREEN,BLUE,YELLOW,RED,MAGENTA,CYAN,GRAY,WHITE,LIGHTRED,LIGHTGREEN,LIGHTBLUE,LIGHTMAGENTA,LIGHTCYAN,DARKGRAY,BLACK,BROWN};

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

  //Reserva de memoria
  estado=(int *)malloc(N*sizeof(int));
  condicion=(pthread_cond_t *)malloc(N*sizeof(pthread_cond_t));
  tenedores=(char *)malloc(N*sizeof(char));

	//Inicializacion de mutexes y variables de condifion
	//Eliminamos los mutex antes de inicializarlos, por si no se hubiese eliminado correctamente en ejecuciones anteriores
  pthread_mutex_destroy(&mutex);
	pthread_mutex_init(&mutex,0);
	for(int i=0;i<N;i++){
		pthread_cond_destroy(&condicion[i]);
		pthread_cond_init(&condicion[i],0);
		estado[i]=0;
		tenedores[i]=i+65; //Codigo ASCII del abecedario
	}


  //'Presentamos' la mesa
  printf("%s\n",WHITE);
  printf("-Numeros: Filosofos -Letras: Tenedores\nEl ultimo tenedor se comparte con el primer filosofo.");
  printf("La disposicion de la mesa es la siguiente:\n          ");
  for(int i=0;i<N;i++){
    printf("%d-%c-",i,tenedores[i]);
  }
  printf("\n\n\n");
  //Creacion de filosofos
  int numeroHilo[N];
  for(int i=0;i<N;i++){
    numeroHilo[i]=i;
    pthread_create(&filosofos[i],0,filosofo,&numeroHilo[i]);
  }

  //Esperamos a que terminen los filosofos
  for(int i=0;i<N;i++){
    pthread_join(filosofos[i],0);
  }

	//Destruimos los mutexes y las variables de condicion
  pthread_mutex_destroy(&mutex);
  for(int i=0;i<N;i++){
    	pthread_cond_destroy(&condicion[i]);
  }

  return EXIT_SUCCESS;
}

void pensar(){
	sleep(rand()%3);
}

void comer(){
	sleep(rand()%4);
}

void probar(int nF){

  int IZQUIERDO = (nF+N-1)%N;
  int DERECHO = (nF+1)%N;
  char *color = colors[nF%15];
  if(estado[nF]==HAMBRIENTO && estado[IZQUIERDO]!=COMIENDO && estado[DERECHO]!=COMIENDO){
    estado[nF]=COMIENDO;
    printf("%s El filosofo %d toma los tenedores %c y %c\n",color,nF,tenedores[IZQUIERDO],tenedores[nF]);
    printf("%s El filosofo %d esta comiendo\n",color,nF);
    pthread_cond_signal(&condicion[nF]); //Avisamos al filosofo nF
  }
}

void tomar_tenedores(int nF){
  pthread_mutex_lock(&mutex);
  estado[nF]=HAMBRIENTO;
  printf("%s El filosofo %d esta hambriento\n",colors[nF%15],nF);
  probar(nF); //Intentamos obtener los tenedores
	while(estado[nF]!=COMIENDO) pthread_cond_wait(&condicion[nF],&mutex);
  pthread_mutex_unlock(&mutex);
  sleep(1);
}

void poner_tenedores(int nF,int IZQUIERDO,int DERECHO){

  char *color= colors[nF%15];
  pthread_mutex_lock(&mutex);
  estado[nF]=PENSANDO;
  printf(" %s El filosofo %d pone los tenedores %c y %c disponibles\n",color,nF,tenedores[IZQUIERDO],tenedores[nF]);
  printf(" %s El filosofo %d esta pensando\n",color,nF);
  probar(IZQUIERDO); //Comprobamos si el filosofo izquierdo puede apovechar el tenedor izquierdo para ponerse a comer
  probar(DERECHO); //Comprobamos si el filosofo derecho puede apovechar el tenedor derecho para ponerse a comer
  pthread_mutex_unlock(&mutex);

}

void *filosofo(void *ptr_numero){

  int numFilosofo=*(int*)ptr_numero;
  int IZQUIERDO = (numFilosofo+N-1)%N;
  int DERECHO = (numFilosofo+1)%N;
  srand(numFilosofo);
	int iteraciones=5;

  while(iteraciones--){
    pensar();
    tomar_tenedores(numFilosofo);
    comer();
    poner_tenedores(numFilosofo,IZQUIERDO,DERECHO);
  }
	return 0;
}
