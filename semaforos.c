/*Programacion del problema de los filósofos utilizando semáforos*/

//Compilar con -pthread

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Libreria que incluye fork()
#include <sys/stat.h> //Biblioteca donde se encuentran los modos de creacion del semafoto
#include <fcntl.h> //Biblioteca donde se encuentran  las flags necesarias para crear el semaforo
#include <pthread.h>//Libreria necesaria para el uso de up y wait con los semaforos
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

int N=0; //Numero de filosofos.
int *estado; //Array que lleva registro del estado de todos los filosofos
sem_t *mutex;//Toma los valores 0 o 1.Garantiza la exclusion mutua
sem_t **semaforo; //Un semáforo por filósofo, para bloquear a los filósofos hambrientos si sus tendores están ocupados
char *tenedores; //Almacena una letra por cada tenedor, para facilitar la lectura por terminal

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
  semaforo=(sem_t **)malloc(N*sizeof(sem_t));
  tenedores=(char *)malloc(N*sizeof(char));
  //Inicializacion de semaforos
  char name[10]; //Variable para almacenar el nombre del semaforo
  for(int i=0;i<N;i++){
    sprintf(name,"%d",i); //Pasamos el indice a la cadena
    strcat(name,"SEM"); //Concatenamos con un sufijo generico
    sem_unlink(name); // Eliminamos posibles semaforos previos con el mismo nombre
    semaforo[i] = sem_open(name,O_CREAT,0600,0); //Creamos el semaforo, inicialmente a 0
    estado[i]=0;
    tenedores[i]=i+65; //Codigo ASCII del abecedario
  }
  sem_unlink("MUTEX");
  mutex= sem_open("MUTEX",O_CREAT,0600,1);

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

  sem_close(mutex);
  for(int i=0;i<N;i++){
    sem_close(semaforo[i]);
  }

  return EXIT_SUCCESS;
}

void pensar(int nF){
	sleep(rand()%3);
}

void comer(int nF){
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
    sem_post(semaforo[nF]); //Avisamos al filosofo nF
  }
}

void tomar_tenedores(int nF){
  sem_wait(mutex);
  estado[nF]=HAMBRIENTO;
  printf("%s El filosofo %d esta hambriento\n",colors[nF%15],nF);
  probar(nF); //Intentamos obtener los tenedores
  sem_post(mutex);
  sem_wait(semaforo[nF]);
  sleep(1);
}

void poner_tenedores(int nF,int IZQUIERDO,int DERECHO){

  char *color= colors[nF%15];
  sem_wait(mutex);
  estado[nF]=PENSANDO;
  printf(" %s El filosofo %d pone los tenedores %c y %c disponibles\n",color,nF,tenedores[IZQUIERDO],tenedores[nF]);
  printf(" %s El filosofo %d esta pensando\n",color,nF);
  probar(IZQUIERDO); //Comprobamos si el filosofo izquierdo puede apovechar el tenedor izquierdo para ponerse a comer
  probar(DERECHO); //Comprobamos si el filosofo derecho puede apovechar el tenedor derecho para ponerse a comer
  sem_post(mutex);

}

void *filosofo(void *ptr_numero){

  int numFilosofo=*(int*)ptr_numero;
  int IZQUIERDO = (numFilosofo+N-1)%N;
  int DERECHO = (numFilosofo+1)%N;
  srand(numFilosofo);
	int iteraciones=5;

  while(iteraciones--){
    pensar(numFilosofo);
    tomar_tenedores(numFilosofo);
    comer(numFilosofo);
    poner_tenedores(numFilosofo,IZQUIERDO,DERECHO);
  }
	return 0;
}
