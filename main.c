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
    printf("readinf type of operation ");
    int type;
    recv(connfd, &type, sizeof(int),0);
    printf("%d \n",type);

    printf("Reading Picture Size\n");
    int size;
    recv(connfd, &size, sizeof(int),0);

    //Read Picture Byte Array
    printf("Reading Picture Byte Array\n");
    char p_array[100];
    FILE *image = fopen("c1.png", "w");
    int nb = recv(connfd, p_array, 100, 0);
    while (nb > 0) {
        fwrite(p_array, 1, nb, image);
        nb = recv(connfd, p_array, 100, 0);
    }

    fclose(image);

    VipsImage *in;

    if( !(in = vips_image_new_from_file( "c1.png", NULL )) )
        vips_error_exit( NULL );

    VipsImage *out;
    switch (type)
    {
    case 0:
        out = grayscale(in);
        break;
    case 1:
        out = invert(in);
        break;
    case 2:
        out = gaussianblur(in);
        break;
    case 3:
        out = sobel(in);
        break;
    default:
        break;
    }
    
    if( vips_image_write_to_file( out, "serverOut/image.png", NULL ) )
        vips_error_exit( NULL );

    g_object_unref( in );
    g_object_unref( out );

    
}

void sendImg(int connfd){

    printf("Getting Picture Size\n");
    FILE *picture;
    picture = fopen("serverOut/image.png", "r");
    int sizePic;
    fseek(picture, 0, SEEK_END);
    sizePic = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    //Send Picture Size
    printf("Sending Picture Size\n");
    send(connfd, &sizePic, sizeof(sizePic), 0);

    printf("Sending Picture as Byte Array\n");
    char send_buffer[100]; // no link between BUFSIZE and the file size
    int nb2 = fread(send_buffer, 1, sizeof(send_buffer), picture);
    while(!feof(picture)) {
        send(connfd, send_buffer, nb2, 0);
        nb2 = fread(send_buffer, 1, sizeof(send_buffer), picture);
    }

    fclose(picture);

}

int main( int argc, char **argv )
{
    int sockfd, connfd, len, nready;
    struct sockaddr_in servaddr, cli;
    fd_set rset;

    if( VIPS_INIT( argv[0] ) )
        vips_error_exit( NULL );
   
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

    while (TRUE)
    {
    // Now server is ready to listen and verification
    if ((listen(sockfd, 50)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    FD_SET(sockfd, &rset);


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
        // sendImg(connfd);
        //close(sockfd);
    }
   
    // After chatting close the socket
    close(sockfd);


    return( 0 );
}
