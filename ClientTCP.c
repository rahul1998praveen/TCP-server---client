//RAHUL PRAVEEN
//16203022
//rahul@ucdconnect.ie
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include<pthread.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include<pthread.h>



int main(int argc, char const *argv[]){
	int clientSocket;
	if(argc != 2){
		printf("[-] Please enter the IP Address of the Computer, Home address is :\n 127.0.0.1");
	}

  	struct sockaddr_in serverAddr;
  	socklen_t addr_size;


  	// Create the socket. 
  	clientSocket = socket(AF_INET, SOCK_STREAM, 0);
  	if(clientSocket == -1){
  		printf("\n [-]Could not create socket \n");
  	}

  	printf("\n [+]Socket Created");
  	//Configure settings of the server address
  	// Address family is Internet 
  	serverAddr.sin_family = AF_INET;
  	//Set port number, using htons function 
  	serverAddr.sin_port = htons(36250);
  	//Set IP address to localhost
  	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

  	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
  	//Connect the socket to the server using the address
  	addr_size = sizeof serverAddr;



  	if(connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size)<0){
  		printf("\n[-] Connection failed");
 	 }
  	printf("\n [+] connected succesfully\n");

  	int sizeofMatrix = 0;
  	// Read the message from the server into the buffer
  	printf("[+] Receiving size of the matrix from Server");
  	if(recv(clientSocket, &sizeofMatrix, sizeof(sizeofMatrix), 0) < 0)
  	{
  		printf("\n [-]Size Receive failed");
  	}

  	printf("\n[+] Received size of the matrix: %d ", sizeofMatrix);

  	//PReceiving the number of workers
  	printf("\n [+] Receiving The number of workers");
  	int workers = 0;
  	if(recv(clientSocket, &workers, sizeof(workers),0)<0){
  		printf("\n [-] Could not receive number of workers");
  	}
  	printf("\n[+] Received number of workers : %d" , workers);
  	

  	// Receiving the matrix B

  	printf("\n[+] Receiving matrix B");
  	float matB[sizeofMatrix][sizeofMatrix];
  	float value = 0.0;

//  	recv(clientSocket, &value, sizeof(value), 0);
  	for(int counter1 = 0; counter1 < sizeofMatrix; counter1++){
  		for(int counter2 = 0; counter2< sizeofMatrix; counter2++){
  			if(recv(clientSocket, &value, sizeof(value), 0) < 0)
  			{	
  				printf("\n [-]Size Receive failed");
  			}
  			matB[counter1][counter2] = value;
  		}	
  	}
  	// printf("%f", value);
  	printf("\n[+] received matrix from server \n");
  	for(int i = 0; i< sizeofMatrix; i++){
  		for(int x = 0; x < sizeofMatrix; x++){
  			printf("%f", matB[i][x]);
  			printf("\t");
  		}
  		printf("\n");
  	}
  	

 	//Receive the sliced matrix from server
 	int numRows = sizeofMatrix/workers;

 	printf("[+] The number of rows received should be : %d \n" , numRows); 
 	float valueMatrixA = 0.0;
 	float matA[numRows][sizeofMatrix];
 	for(int i =0; i <numRows; i++){
 		for(int x = 0; x < sizeofMatrix; x++){
 			if(recv(clientSocket, &valueMatrixA, sizeof(valueMatrixA), 0) < 0){
 				printf("\n[-] Could not receive value for Matrix A");
 			}
 			matA[i][x] = valueMatrixA;
 		}
 	}
	// //print received sliced matrix
 	for(int counter1 = 0; counter1 <numRows; counter1++){
 		for(int counter2 = 0; counter2 < sizeofMatrix; counter2 ++){
 			printf("%f", matA[counter1][counter2]);
 			printf("\t");
 		}
 		printf("\n");
 	}

 	// Matrix multiplication
 	float result = 0.0; 
 	for(int start =0 ;start< numRows; start++){

 		for(int end = 0 ; end < sizeofMatrix; end++){

 			for(int mid =0; mid< sizeofMatrix; mid++){
 				result = result +(matA[start][mid] * matB[mid][end]);
 			}
 			if(send(clientSocket, &result, sizeof(result),0)<0){
 				printf("\n [-] Could not Send value");
 			}
 			result = 0.0;
 		}
 	}

  	close(clientSocket);
	
  	return 0;

}