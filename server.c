#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "util.h"

segment* recvbuffer;

void writeToFile() {
    FILE* fp;
    fp = fopen("output.txt", "a");
    fprintf(fp, "This is the sample output\n");
    fclose(fp);
}

int main(int argc, char *argv[]){
    recvbuffer = (segment*) malloc (256);

    writeToFile();
    // if(argc < 5){
    //     die("<filename> <windowsize> <buffersize> <port>");
    // }

    // // read from argument
    // char* FILE_NAME = argv[1];
    // int WS = char_to_int(argv[2]);
    // int BUFLEN = char_to_int(argv[3]);
    // int PORT = char_to_int(argv[4]);
    // char buf[BUFLEN];

    // int udpSocket, nBytes;
    // char buffer[BUFLEN];
    // struct sockaddr_in serverAddr, clientAddr;
    // struct sockaddr_storage serverStorage;
    // socklen_t addr_size, client_addr_size;
    // int i;

    // /*Create UDP socket*/
    // udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    // if(udpSocket < 0){
    //     die("Failed create UDP Socket");
    // }

    // /*Configure settings in address struct*/
    // serverAddr.sin_family = AF_INET;
    // serverAddr.sin_port = htons(PORT);
    // serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    // /*Bind socket with address struct*/
    // if(bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
    //     die("Couldn't bind socket");
    // }

    // /*Initialize size variable to be used later on*/
    // addr_size = sizeof serverStorage;

    // while(1){
    //     /* Try to receive any incoming UDP datagram. Address and port of 
    //       requesting client will be stored on serverStorage variable */
    //     nBytes = recvfrom(udpSocket,buffer,BUFLEN,0,(struct sockaddr *) &serverStorage, &addr_size);

    //     /*Convert message received to uppercase*/
    //     for(i=0;i<nBytes-1;i++)
    //       buffer[i] = toupper(buffer[i]);

    //     printf("%s\n", buffer);
    //     /*Send uppercase message back to client, using serverStorage as the address*/
    //     sendto(udpSocket,buffer,nBytes,0,(struct sockaddr *)&serverStorage,addr_size);
    // }

    return 0;
}