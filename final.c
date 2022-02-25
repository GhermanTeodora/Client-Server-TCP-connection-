#include <stdio.h> //printf
#include <sys/socket.h> //socket
#include <sys/types.h>
#include <arpa/inet.h> //inet_addr
#include <string.h>
#include <stdlib.h>
#include <unistd.h> //usleep
#include <netinet/in.h> //
#include <fcntl.h> //fcntl

#define CLIENT_QUEUE_LEN 10 //max number of requests
#define SERVER_PORT 22011
#define CLIENT_PORT 80
#define CHUNK_SIZE 1024

int receive(int s);

int main(void){
	//parameters for server
	int listen_sock_fd, client_sock_fd;
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_addr_len;
	char str_addr[INET_ADDRSTRLEN];
	int ret, flag;
	char server_reply[50];
	
	//parameters from client
	int socket_desc; // takes the file descriptor returned by socket
	struct sockaddr_in6 addr;
	char *message;
	
	//create the socket for the client (client.c)
	socket_desc = socket(AF_INET6, SOCK_STREAM,0); //opens the socket and returns the file descriptor
	//SOCK_STREAM => TCP
	if(socket_desc < 0) {
		printf("Socket creation error! \n");
		return -1;
	} else printf("Socket Ok!\n");
	//end
	
	//connect() IPv6 client
	memset(&addr, '\0', sizeof(addr));
	addr.sin6_family = AF_INET6; //provides support for IPv6 128 bit address structures
	addr.sin6_port=htons(CLIENT_PORT);
	
	inet_pton(AF_INET6, "2001:1430:a:50::50", &addr.sin6_addr);
	if(connect(socket_desc, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		printf("Connection failed\n");
		return -1;
	} else printf("Connection OK!\n");

	//send data
	message = "GET / HTTP/1.0\r\nHost: www.axu.tm\r\n\r\n";
	if(send(socket_desc, message, strlen(message), 0) <0){
		printf("Send failed!\n");
		return -1;
	} else printf("Data sent!\n");

	receive(socket_desc);
       	close(socket_desc); //close socket	
	printf("Client socket closed\n");
	//end connect()
	
	//create socket for listening client requests (server.c)
	listen_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock_fd < 0) {
		perror("Socket still closed!\n");
		return -1;
	} else printf("Socket open!\n");
	
	//connect to server running on local host bind
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr= htonl(INADDR_ANY);
	server_addr.sin_port = htons(SERVER_PORT);

	//bind address and socket together
	ret = bind(listen_sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)); //attaches a new IP address and port number to the socket
	if (ret <0){
		printf("Bind unsuccessful!\n");
		exit(0);
	} else printf("Bind successful!\n");

	//create listening queue (client requests)
	ret = listen(listen_sock_fd, CLIENT_QUEUE_LEN); //checks the availability of the server to accept connections
	if (ret < 0) {
		printf("listen unsuccessful\n");
		exit(0);
	} else printf("Listen successful!\n");

	client_addr_len = sizeof(client_addr);
	
	// fp = fopen("index.html","r");

	while(1) {
		//TCP handshake with client
		client_sock_fd = accept(listen_sock_fd,
				(struct sockaddr*)&client_addr,
				&client_addr_len);  //blocks the waiting until a new connection 
		if (client_sock_fd < 0) {
			printf("Accept unsuccessful!\n");
			close(listen_sock_fd);
			return EXIT_FAILURE;
		} else printf("Accept successful!\n");
		
		//connect to server running on local host
		inet_ntop(AF_INET, &(client_addr.sin_addr),
				str_addr, sizeof(str_addr)); 
		printf("New connection from: %s:%d...\n",
				str_addr,
				ntohs(client_addr.sin_port)); 
		ret = recv(client_sock_fd, server_reply, 20, 0); //se receptioneaza date de la conexiunea stabilita anterior

		while(recv(client_sock_fd, server_reply, 20, 0) >0){
			//wait for data from client
			if (ret < 0) {
				printf( "Receive unsuccessful!\n");
				close(client_sock_fd);
				continue;
			} else{
				printf("Receive successful!\n", &server_reply);
			}		
			//send response to client
			ret = send(client_sock_fd, server_reply, 20, 0); //se transmit date prin conexiunea stabilita anterior	
			if (ret < 0){
				send(client_sock_fd, "Command not implemented!\n", 20, 0);
				close(client_sock_fd);
				continue;			
			} else {
				char *alert = "Command Implemented\n";
				send(client_sock_fd, alert, 20,0);
			}
		}//end while
				
	
		//TCP teardown
		ret = close(client_sock_fd);
		if (ret < 0) {
			printf("close unsuccessful\n");
			client_sock_fd = -1;
		}
		perror("Connection closed\n");
	} //end while(1)

	return EXIT_SUCCESS;
		
}

int receive(int s){
	int size_recv, total_size =0;
	char chunk[CHUNK_SIZE];
	FILE *fp; 
	fp = fopen("index.html", "w"); //create file
	
	fcntl(s, F_SETFL, O_NONBLOCK); //make socket non-blocking

	//receive a reply from the server
	while(1){
		memset(chunk, 0, CHUNK_SIZE); //clear the variable
		if((size_recv= recv(s, chunk, CHUNK_SIZE, 0) && fp == NULL) <0){
			usleep(10);
		} else {
			total_size += size_recv;
			printf("%s", chunk);
		//	fwrite(chunk, CHUNK_SIZE, sizeof(chunk), fp);
		
		}
	} //end while

	fclose(fp); // closes the file
} //end receive
