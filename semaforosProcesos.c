/*Programacion del problema de los filósofos utilizando semáforos y procesos en lugar de hilos*/

//Compilar con -pthread

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Libreria que incluye fork()
#include <sys/stat.h> //Biblioteca donde se encuentran los modos de creacion del semafoto
#include <fcntl.h> //Biblioteca donde se encuentran  las flags necesarias para crear el semaforo
#include <pthread.h>//Libreria necesaria para el uso de up y wait con los semaforos
#include <semaphore.h>
#include <sys/wait.h> //Libreria en la cual se encuentra la funcion wait
#include <string.h> //Necesaria para manejar
#include <sys/mman.h> //Biblioteca necesaria para realizar las proyecciones en memoria (mman, munmap)

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
sem_t *mutex;//Toma los valores 0 o 1.Garantiza la exclusion mutua
sem_t **semaforo;
char *tenedores;

//Cadena que contiene los colores disponibles
char *colors[]={GREEN,BLUE,YELLOW,RED,MAGENTA,CYAN,GRAY,WHITE,LIGHTRED,LIGHTGREEN,LIGHTBLUE,LIGHTMAGENTA,LIGHTCYAN,DARKGRAY,BLACK,BROWN};

void filosofo(int numeroFilosofo,int *estado);

int main(int argc, char * argv[]){//Argumentos:  nºFilosofos

  //Obtencion de parametros
	if(argc!=2){
    printf("Numero de parametros invalidos: Introduzca el numero de filosofos");
    exit(EXIT_FAILURE);
  }
  else{
    N=atoi(argv[1]);//Numero de filosofos
  }
	/*MMAP:
  -El primer argumento de mmap indica la direccion de memoria donde se va a comenzar a proyectar el archivo.
  Si es NULL, el kernel elige esta direccion
	-El segundo argumento indica la cantidad de memoria que se reserva: En este caso, el tamaño de los enteros
  -El tercer argumento hace refrencia a los permisos que tendran las paginas de memoria donde se reserva memoria: En este caso, permisos de lectura y escritura
  .El cuarto argumento (flags) indican la visibilidad del mapa por otros procesos: En este caso, memoria compartida y mapeo anonimo (sin fichero de respalf)
  -El quinto argumento es el "objeto de memoria" que queremos proyectar, en este caso ninguno
  -El sexto argumento indica el desplazamiento dentro de la zona de memoria indicada en el primer argumento*/
	if((estado=(int*)mmap(NULL,N*sizeof(int),PROT_READ | PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,0,0))==((void *)-1)){
    perror("Error al proyectar el fichero en el mapa de memoria: ejecucion abortada\n");
    exit(-1);
  }

  //Reserva de memoria
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
  //Creacion de los procesos filosofos
	int hijo;
	for(int i=0;i<N;i++){//Se usaran H procesos
    if((hijo=fork())<0){ //Creamos los procesos
      printf("Erro na creacion dun dos procesos: Execucion abortada\n");
      exit(-1);
    }
    else if(hijo==0){ //Proceso hijo
      filosofo(i,estado); //O proceso calcula o valor que lle corresponde
      exit(1); //Salimos do proceso
    }
    else //Proceso padre
      ; //Contiuamos co bucle de creacion de procesos fillo
  }
  //Esperamos a que terminen los filosofos
  for(int i=0;i<N;i++){
    wait(&hijo);
  }

  sem_close(mutex);
  for(int i=0;i<N;i++){
    sem_close(semaforo[i]);
  }

  return EXIT_SUCCESS;
}

void pensar(){
  sleep(rand()%3);
}

void comer(){
  sleep(rand()%4);
}

void probar(int nF,int *estado){

  int IZQUIERDO = (nF+N-1)%N;
  int DERECHO = (nF+1)%N;
  char *color = colors[nF%15];
  if(estado[nF]==HAMBRIENTO && estado[IZQUIERDO]!=COMIENDO && estado[DERECHO]!=COMIENDO){
    estado[nF]=COMIENDO;
    //sleep(2);
    printf("%s El filosofo %d toma los tenedores %c y %c\n",color,nF,tenedores[IZQUIERDO],tenedores[nF]);
    printf("%s El filosofo %d esta comiendo\n",color,nF);
    sem_post(semaforo[nF]);
    //sleep(2);
  }
}

void tomar_tenedores(int nF,int *estado){
  sem_wait(mutex);
  estado[nF]=HAMBRIENTO;
  printf("%s El filosofo %d esta hambriento\n",colors[nF%15],nF);
  probar(nF,estado);
  sem_post(mutex);
  sem_wait(semaforo[nF]);
  sleep(1);
}

void poner_tenedores(int nF,int IZQUIERDO,int DERECHO,int *estado){

  char *color= colors[nF%15];
  sem_wait(mutex);
  estado[nF]=PENSANDO;
  printf(" %s El filosofo %d pone los tenedores %c y %c disponibles\n",color,nF,tenedores[IZQUIERDO],tenedores[nF]);
  printf(" %s El filosofo %d esta pensando\n",color,nF);
  probar(IZQUIERDO,estado);
  probar(DERECHO,estado);
  sem_post(mutex);

}

void filosofo(int numeroFilosofo,int *estado){

  int IZQUIERDO = (numeroFilosofo+N-1)%N;
  int DERECHO = (numeroFilosofo+1)%N;
  srand(numeroFilosofo);
	int iteraciones=5;

  while(iteraciones--){
    pensar();
    tomar_tenedores(numeroFilosofo,estado);
    comer();
    poner_tenedores(numeroFilosofo,IZQUIERDO,DERECHO,estado);
  }

}
