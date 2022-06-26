#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCK_PATH "echo_socket"


void adminFunction(int sockfd){
	for(;;){
        printf("Choose operation:\n");
        printf("1.Total number of images process\n");
        printf("2.Number of images tranformed to gray\n");
        printf("3.Number of images inverted\n");
        printf("4.Number of images blured\n");
        printf("5.Number of images apllied sobel filter\n");
        printf("6.Disconnect\n");
        printf("7.Up time\n");
        int op;
        double time;
        scanf("%d",&op);
        send(sockfd, &op, sizeof(int), 0);
        int number;
        if(op == 6){
            printf("Disconnected\n");
            break;
        }
        if(op!=7){   
            while (recv(sockfd, &number, sizeof(int),0)<0)
            {
                usleep(100);
            }
            printf("%d\n",number);
        }else{
            while (recv(sockfd, &time, sizeof(double),0)<0)
            {
                usleep(100);
            }
            printf("%f\n",time);
        }
    }
}

int main(void)
{
    int s, t, len;
    struct sockaddr_un remote;
    char str[100];

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    printf("Trying to connect...\n");

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    printf("Connected.\n");

    adminFunction(s);
    close(s);

    return 0;
}