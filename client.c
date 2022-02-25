#include <stdio.h> //printf
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr
#include <string.h>
#include <unistd.h> //usleep
#include<fcntl.h> //fcntl

#define CHUNK_SIZE 1024
#define PORT 80
int receive(int s);

int main(void){
	int socket_desc; //takes the file descriptor returned by socket
	struct sockaddr_in6 addr; //
	char *message;
	//create socket
	socket_desc= socket(AF_INET6, SOCK_STREAM, 0);//opens the socket and returns the file desciptor
	//SOCK_STREAM => TCP
	
	if (socket_desc < 0)
	{
		printf("\n  Socket creation error \n");
		return -1;
	} else printf("\n Socket OK!\n");
	
	memset(&addr, '\0', sizeof(addr));
	addr.sin6_family = AF_INET6; //provides support for IPv6 128 bit  address structures
	addr.sin6_port=htons(PORT); //converts an IP port number in host byte order to the IP port number in network bite order 
	
	inet_pton(AF_INET6, "2001:1430:a:50::50", &addr.sin6_addr);
	if(connect(socket_desc, (struct sockaddr *)&addr, sizeof(addr)) < 0) {	//connects to socket
		printf("\n Connection failed\n");
		return -1;
	} else printf ("\n Conncetion OK!\n");

	//Send some data	
	message = "GET / HTTP/1.0\r\nHost: www.axu.tm\r\n\r\n";

	if (send(socket_desc, message, strlen(message), 0)<0)
	{
		printf("Send failed");
		return -1;
	} else printf("\nData sent!\n");
	
	receive(socket_desc);
	
	return 0;

}
int receive(int s){ 
	int size_recv, total_size=0;
	char chunk[CHUNK_SIZE];
	//char *f;
	FILE *fp;
	fp=fopen("index.html", "w");
	//make socket non blocking 
	fcntl(s, F_SETFL, O_NONBLOCK);

	//receive a reply from the server
	while(1){ 
		
		memset(chunk, 0, CHUNK_SIZE); //clear the variable
		if((size_recv=recv(s, chunk, CHUNK_SIZE,0) && fp == NULL) <0){
			usleep(10);
		} else {  	
			total_size += size_recv;
			printf("%s", chunk);
	//		fwrite(chunk, CHUNK_SIZE, sizeof(chunk), fp);
		} //end else
	} // end while
	fclose(fp);

} // end receive

