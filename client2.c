#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#define m 80
#define PORT 5555
#define SA struct sockaddr


void sendImg(int sockfd, int typeOperation)
{
	printf("Send Type Of Operation\n");
	//int typeOperation = 1;
	send(sockfd, &typeOperation, sizeof(int), 0);

    printf("Getting Picture Size\n");
    FILE *picture;
    picture = fopen("index.png", "r");
    int sizePic;
    fseek(picture, 0, SEEK_END);
    sizePic = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    //Send Picture Size
    printf("Sending Picture Size\n");
    send(sockfd, &sizePic, sizeof(sizePic), 0);


    //Send Picture as Byte Array
    printf("Sending Picture as Byte Array\n");
    char send_buffer[100]; // no link between BUFSIZE and the file size
	do{
		usleep(10000);
        int nb = fread(send_buffer, 1, sizeof(send_buffer), picture);
        send(sockfd, send_buffer, nb, 0);
		printf("Send %d bytes\n",nb);
	}while(!feof(picture));

    fclose(picture);
	
}

void receive(int sockfd){
	
	int size;
	while(recv(sockfd, &size, sizeof(int), 0)<=0){
		printf("I am sleeping\n");
		usleep(1000000);
	}
    
	printf("Reading Picture Byte Array\n");
    char p_array[100];
    FILE *image = fopen("client2/clientPicture.png", "w");
    while (size>0) {
        int nb = recv(sockfd, p_array, 100, 0);
        if(nb<0)
            continue;
        size= size-nb;
        printf("I read %d bytes and %d size\n",nb, size);
        fwrite(p_array, 1, nb, image);
    }
	fclose(image);
}


int main(int argc, char *argv[])
{
	int sockfd, connfd, nready;
	struct sockaddr_in servaddr, cli;
	fd_set rset;

	if(argc <2){
		perror("Insificient Arguments");
		exit(1);
	}


	char *type = argv[1];
	int tp;
	if (strcmp(type, "gray") == 0) 
		{
			tp = 0;
		} 
		else if (strcmp(type, "inverted") == 0)
		{
			tp = 1;
		}
		else if (strcmp(type, "gaussian") == 0)
		{
			tp = 2;
		}
		else if (strcmp(type, "sobel") == 0)
		{
			tp = 3;
		}
		else
		{
			perror("Wrong Type of operation");
			exit(2);
		}

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to the server..\n");

	
	sendImg(sockfd, tp);

	receive(sockfd);

	// close the socket
	close(sockfd);
}
