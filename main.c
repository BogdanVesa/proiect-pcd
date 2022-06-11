#include <stdio.h>
#include <vips/vips.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#define PORT 5555
#define SA struct sockaddr
#define m 80


VipsImage* grayscale(VipsImage* img){

    VipsImage *scRGB;

    if(vips_sRGB2scRGB(img, &scRGB, NULL))
        vips_error_exit(NULL);

    VipsImage *gray;
    if( vips_scRGB2BW(scRGB, &gray, NULL))
        vips_error_exit( NULL );

    return gray;
}

VipsImage* invert(VipsImage* img){

    VipsImage *scInverted;
    VipsImage *copy;

    if(vips_copy(img, &copy, NULL))
        vips_error_exit(NULL);

    if( vips_invert(copy, &scInverted, NULL ) )
        vips_error_exit( NULL );

    return scInverted; 
}

VipsImage* gaussianblur(VipsImage* img){

    VipsImage *gauss;
    VipsImage *copy;

    if(vips_copy(img, &copy, NULL))
        vips_error_exit(NULL);

    if( vips_gaussblur( copy, &gauss, 5.00, NULL ) )
        vips_error_exit( NULL );

    return gauss;
}

VipsImage* sobel(VipsImage* img){

    VipsImage *scSobel;
    VipsImage *copy;

    if(vips_copy(img, &copy, NULL))
        vips_error_exit(NULL);

    VipsImage *gray = grayscale(copy);

    if( vips_sobel( gray, &scSobel, NULL) )
        vips_error_exit( NULL );

    return scSobel;

}
void func(int connfd)
{
    printf("Reading Picture Size\n");
    int size;
    read(connfd, &size, sizeof(int));

    //Read Picture Byte Array
    printf("Reading Picture Byte Array\n");
    char p_array[1024];
    FILE *image = fopen("c1.png", "w");
    int nb = read(connfd, p_array, 1024);
    while (nb > 0) {
        fwrite(p_array, 1, nb, image);
        nb = read(connfd, p_array, 1024);
    }
    fclose(image);
}

int main( int argc, char **argv )
{
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
   
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
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");
   
    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);
   
    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server accept failed...\n");
        exit(0);
    }
    else
        printf("server accept the client...\n");
   
    // Function for chatting between client and server
    func(connfd);
   
    // After chatting close the socket
    close(sockfd);

    return( 0 );
}
