// RAHUL PRAVEEN
// 16203022
//rahul@ucdconnect.ie
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <limits.h>
#include <pthread.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

char buffer[100000];
int n, p;
int numberRow =0;
int startRow= 0;
int endRow =0;
float **matA, **matB, **matResult, **slicedA;


typedef struct threadArgs{
	int rowNumber;
	int hostID;
	int socket;
	int size;
	int workers;
	float **matrix1;
	float **matrix2;
	float **matrix3;
}threadArg;

//Function to print the matrix
void printMatrix(float **mat, int n){
		for(int counter1 =0; counter1< n; counter1++ ){
			for(int counter2 = 0; counter2<n; counter2++){
				printf("%f", mat[counter1][counter2]) ;
				printf("\t");
		}
		printf("\n");
	}
}

void* connection_handler(void* arg){

	struct threadArgs *conn = arg;

	int newSocket = conn->socket;
	int worker = conn->workers;
	int size = conn->size;
	int id = conn->hostID;
	int workers = conn->workers;
	int slices = size/workers;

 	sleep(3);
// SIZE
 	printf("\n[+]Sending size of the matrix\n");

 	if(send(newSocket, &size, sizeof(size), 0) <0 ){
 		printf("\n[-] Could not send size of the matrix");
 	}
 	printf("\n[+] Sent the size successfully : %d", size);
 	// WORKERS
 	printf("\n[+] Sending number of workers");
 	
 	if(send(newSocket, &worker, sizeof(worker), 0) <0 ){
 		printf("\n[-] Could not send number of workers");
 	}

 	printf("\n [+] Number of workers sent : %d", workers);

 	printf("\n[+] Sending Matrix B");
 	float value =0.0;
// 	send(newSocket, &conn->matrix1[0][0], sizeof(conn->matrix1[0][0]), 0);
	 for(int counter1=0; counter1<size; counter1++){
	 	for(int counter2 = 0; counter2<size; counter2++){

	 		value = conn->matrix2[counter1][counter2];
	 		if(send(newSocket, &value, sizeof(value), 0)<0){
	 			
	 			printf("\n [-] Could not send the matrix B");
				
	 		}	
	 		printf("\n Sent value : %f ", value);
	 		value = 0.0;	
	 		printf("\n");
	 	}
	 }
	
	
	printf("\n Sent Matrix B successfully \n");

	//Sending sliced matrix A
	float valueMatrixA = 0.0;
	printf("\n Sliced Row number is: %d" , numberRow);
	printf("\n Sending Matrix A");
	//printf("\n Slices: %d", slices);

	for(int counterRow = numberRow; counterRow < numberRow+slices; counterRow++){

		for(int counterColumn = 0; counterColumn< size; counterColumn++){

			valueMatrixA = conn->matrix1[counterRow][counterColumn];

			if(send(newSocket, &valueMatrixA, sizeof(valueMatrixA),0)<0){

				printf("\n[-] Could not send the sliced Matrix A");
			}

			printf("\n[+] sent value : %f", valueMatrixA);
			valueMatrixA =0.0;
			printf("\n");
		}
	}
	numberRow = numberRow + slices;
	printf("\n[+] Sent sliced matrix A");

	//printf("\n New row number is : %d", numberRow);

	// Receiving Matrix C
	printf("\n[+] Receiving Result Mtrix now");
	float valueResult = 0.0;
	for(int c = startRow; c <startRow+slices; c++){
		for(int d = 0; d < size ; d++){
			if(recv(newSocket, &valueResult, sizeof(valueResult),0)<0){
				printf("\n[-] Could not receive Result value");
			}
			matResult[c][d] = valueResult;
			valueResult =0;
		}
	}
	printf("\n[+] Result Mtrix received");
	startRow += slices;
	close(newSocket);
	pthread_exit(NULL);
}



float ** CreateMatrix( int n )
{
  /* Allocate 'size' * 'size' doubles contiguously. */
  float * vals = (float *) malloc( n * n * sizeof(float) );

  /* Allocate array of double* with size 'size' */
  float ** ptrs = (float **) malloc( n * sizeof(float*) );

  int i;
  for (i = 0; i < n; ++i) {
    ptrs[ i ] = &vals[ i * n ];
  }

  return ptrs;
}


int main(int argc, char const *argv[])
{

	if(atoi(argv[1])==0){
		printf("Please enter the size of the matrices");
		exit(EXIT_FAILURE);
	}

	n = atoi(argv[1]); // size of the matrix
	p = atoi(argv[2]); // number of processes

	matA = CreateMatrix(n);
	matB = CreateMatrix(n);
	matResult = CreateMatrix(n);

	//randomiseMatrix(matResult, n);
	srand((unsigned int)time(0));
	for(int i=0; i< n; i++ ){
		for(int x = 0; x < n; x++){
			matA[i][x] = ((float)rand()/(float)(RAND_MAX))*10;
		}
	}
	for(int i=0; i< n; i++ ){
		for(int x = 0; x < n; x++){
			matB[i][x] = ((float)rand()/(float)(RAND_MAX))*10;
		}
	}

//	randomiseMatrix(matA, n);
//	randomiseMatrix(matB, n);
	printf("Matrix A: \n");
	printMatrix(matA, n);

	printf("Matrix B: \n");
	printMatrix(matB, n);

	// creating the socket

	int serverSocket, newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;
	socklen_t addr_size;

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(serverSocket == -1){
		printf("\n [-]Could not create socket \n");
	}
	printf("\n [+] Socket created \n");
	serverAddr.sin_family = AF_INET;

	serverAddr.sin_port = htons(36250);

	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);



	if(bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) <0){
		printf("\n [-]Bind Failed \n");
	}

	printf(" \n[+] Bind succesfull \n");

	if(listen(serverSocket, p) ==0){
		printf("[+]listening \n");
	}
	
	pthread_t * threads;
	threads = (pthread_t *) malloc( p * sizeof(pthread_t) );
	// newSockets = (threadArgs *) malloc(p * sizeof(threadargs));
	int num_hosts=0;
	

	while(num_hosts < p){
		printf("\n [+] Waiting for Client \n");
		addr_size = sizeof serverStorage;
		printf("\n HostId : %d \n", num_hosts);
		struct threadArgs conn;
		//conn.rowNumber = row;
		conn.size = n;
		conn.workers = p;
		conn.hostID= num_hosts;
		conn.matrix1 = matA;
		conn.matrix2 = matB;
		int newSocket;
		newSocket = accept(serverSocket, (struct sockaddr *) &serverStorage, &addr_size);
		conn.socket = newSocket;

		if( pthread_create(&threads[num_hosts], NULL, &connection_handler, (void *)&conn) != 0 ){
			printf("Failed to create thread\n");
		}
		pthread_join(threads[num_hosts], NULL);
		num_hosts+=1;
		
	}	
	printf("\n Ended \n");
	printf("\n[+] Final matrix received is : \n");
	printMatrix(matResult, n);
  return 0;
}