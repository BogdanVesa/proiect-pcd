#include <stdio.h>
#include <vips/vips.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#define SOCK_PATH "echo_socket"

#define PORT 5555
#define SA struct sockaddr
#define m 80
#define BACKLOG 5

clock_t t;

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
void * func(void* pconnfd)
{
    int connfd = *(int*)pconnfd;
    free(pconnfd);
    char folder[255] = "serverIn/";
    char file[255]= "";
    char buffer[20];
    printf("reading type of operation ");
    int type;
    recv(connfd, &type, sizeof(int),0);
    printf("%d \n",type);
    sprintf(buffer, "%d", type);
    printf("%s\n",buffer);
    strcat(file, buffer);
    printf("file type %s\n",file);

    printf("Reading Picture Size\n");
    int size;
    recv(connfd, &size, sizeof(int),0);
    sprintf(buffer,"%d",size);
    strcat(file, buffer);
    printf("file size %s\n",file);

    int number = rand();
    sprintf(buffer, "%d", number);
    strcat(file, buffer);

    strcat(file,".png");
    strcat(folder,file);
    //Read Picture Byte Array
    printf("Reading Picture Byte Array\n");
    char p_array[100];
    FILE *image = fopen(folder, "w");
    int nb;
    while (size>0) {
        nb = recv(connfd, p_array, 100, 0);
        
        if(nb<0)
            continue;
        size= size-nb;
        fwrite(p_array, 1, nb, image);
    }

    fclose(image);

    VipsImage *in;

    sleep(20);

    printf("Image proccesing started\n");
    if( !(in = vips_image_new_from_file( folder, NULL )) )
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


    char folderOut[255] = "serverOut/";
    strcat(folderOut,file);
    if( vips_image_write_to_file( out, folderOut, NULL ) )
        vips_error_exit( NULL );

    printf("Image proccesing endeed\n");

    g_object_unref( in );
    g_object_unref( out );

    printf("Getting Picture Size\n");
    FILE *picture;
    picture = fopen(folderOut, "r");
    int sizePic;
    fseek(picture, 0, SEEK_END);
    sizePic = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    //Send Picture Size
    printf("Sending Picture Size\n");
    send(connfd, &sizePic, sizeof(sizePic), 0);

    printf("Sending Picture as Byte Array\n");
    char send_buffer[100]; // no link between BUFSIZE and the file size
	do{
        int nb2 = fread(send_buffer, 1, sizeof(send_buffer), picture);
        send(connfd, send_buffer, nb2, 0);
	}while(!feof(picture));

    fclose(picture);

    
}
int numberOfPictures(int type){
    DIR *d;
    struct dirent *dir;
    int total =0;
    char c;
    int num;
    d = opendir("./serverOut");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            c= dir->d_name[0];
            num = c-'0';
            if(num==type){
                total++;
            }
        }
        closedir(d);
    }
    return total;
}

int totalNumber(){
    DIR *d;
    struct dirent *dir;
    int total =0;
    d = opendir("./serverOut");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
                total++;
        }
        closedir(d);
    }
    total = total -2;
    return total;
}

void adminConnection(int connfd){
    for(;;){    
        int op;
        while(recv(connfd,&op, sizeof(int),0)<0){
            usleep(100);
        }
        int number;
        clock_t end;
        double time;
        switch (op)
        {
        case 1:
            number = totalNumber();
            break;
        case 2:
            number = numberOfPictures(0);
            break;
        case 3:
            number = numberOfPictures(1);
            break;
        case 4:
            number = numberOfPictures(2);
            break;
        case 5:
            number = numberOfPictures(3);
            break;
        case 7:
            end = clock();
            time = (double)(end-t)/CLOCKS_PER_SEC*1000;
        default:
            break;
        }
        if(op == 6){
            break;
        }
        if(op!=7){
        send(connfd, &number, sizeof(int),0);
        }
        else{
            send(connfd,&time, sizeof(double),0);
        }
    }
}


void *ClientAdmin(){
    int s, s2, t, len;
    struct sockaddr_un local, remote;

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    local.sun_family = AF_UNIX;
    strcpy(local.sun_path, SOCK_PATH);
    unlink(local.sun_path);
    len = strlen(local.sun_path) + sizeof(local.sun_family);
    if (bind(s, (struct sockaddr *)&local, len) == -1) {
        perror("bind");
        exit(1);
    }
        if (listen(s, 1) == -1) {
            perror("listen");
            exit(1);
        }

    while (TRUE)
    {
    int done, n;
    printf("Waiting for a connection...\n");
    
        t = sizeof(remote);
        if ((s2 = accept(s, (struct sockaddr *)&remote, &t)) == -1) {
            perror("accept");
            exit(1);
        }

        printf("Connected.\n");

        adminConnection(s2);
    }

    close(s2);
}

void *ClientConnection(){

    int sockfd, connfd, len, nready;
    struct sockaddr_in servaddr, cli;

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
        if ((listen(sockfd, 50)) != 0) {
            printf("Listen failed...\n");
            exit(0);
        }
        else
            printf("Server listening..\n");
    
    while (TRUE)
    {
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
        pthread_t t;
        int * pconnfd = malloc(sizeof(int));
        *pconnfd = connfd; 
        pthread_create(&t, NULL, func, pconnfd);
    
        //close(sockfd);
    }
   
    // After chatting close the socket
    close(sockfd);

}

int main( int argc, char **argv )
{
    pthread_t thread_id[2];
    t = clock();
    pthread_create(&thread_id[0], NULL, ClientAdmin, NULL);
    pthread_create(&thread_id[1],NULL, ClientConnection, NULL);

    for(int i=0;i<2;i++)
        pthread_join(thread_id[i],NULL);
    if( VIPS_INIT( argv[0] ) )
        vips_error_exit( NULL );
       
    return( 0 );
}
