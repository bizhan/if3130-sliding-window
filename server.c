#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "segment.h"
#include "util.h"

#define WINDOWSIZE 4
#define BUFFERSIZE 256
#define SEGMENTSIZE 9

int BUFFERFULL = 0;

typedef struct {
  segment* array;
  int length;
} BufferArray;

BufferArray recvbuffer;

void initBufferArray(BufferArray* a) {
    a->array = (segment*) malloc (BUFFERSIZE);
    a->length = 0;
}

void freeArray(BufferArray *a) {
  free(a->array);
  a->array = NULL;
  a->length = 0;
}

void drainBufferArray(BufferArray* a) {
    for (int i = 0; i < a->length; i++) {
        segment aSegment = *(a->array + i * SEGMENTSIZE);
        printf("%d - %c\n", aSegment.seqNum, aSegment.data);
    }
    freeArray(a);
    BUFFERFULL = 0;
}

void insertBufferArray(BufferArray *a, segment aSegment) {
    int currentMemoryUsed = a->length * SEGMENTSIZE;
    int memoryNeeded = currentMemoryUsed + SEGMENTSIZE;
    int remainingMemoryAfterInsertion = BUFFERSIZE - memoryNeeded;
    
    *(a->array + a->length * SEGMENTSIZE) = aSegment;
    a->length = a->length + 1;
    
    if (remainingMemoryAfterInsertion < SEGMENTSIZE) {
        printf("Buffer is already full at length %d \n", a->length);
        BUFFERFULL = 1;
    }
    else {
        BUFFERFULL = 0;    
    }
}

// void writeToFile(char* filename, char* message) {
//     FILE* fp;
//     fp = fopen(filename, "a");
//     fprintf(fp, message);
//     fclose(fp);
// }

segment generateSegment(int seqNum, char data, char checksum) {
    segment aSegment;
    aSegment.soh = 0x1;
    aSegment.seqNum = seqNum;
    aSegment.stx = 0x2;
    aSegment.data = data;
    aSegment.etx = 0x3;
    aSegment.checksum = checksum;
    return aSegment;
}

int main(int argc, char *argv[]){

    initBufferArray(&recvbuffer);

    int totalLength = 10;

    for (int i = 0; i < totalLength; i++) {
        char cc = 'x';
        segment aSegment = generateSegment(i, cc, i - 1);
        insertBufferArray(&recvbuffer, aSegment);

        if (BUFFERFULL == 1 || i == totalLength - 1) {
            drainBufferArray(&recvbuffer);
        }
    }
    // printf("%d\n", recvbuffer.length);

    // for (int i = 0; i < recvbuffer.length; i++) {
    //     segment aSegment = *(recvbuffer.array + i * SEGMENTSIZE);
    //     printf("%c", aSegment.data);
    // }


    // for (int i = 0; i < BUFFERSIZE; i += SEGMENTSIZE) {
    //     for (int j = 0; j < SEGMENTSIZE; j++) {
    //         recvbuffer
    //     }
    // }

    // recvbuffer = (segment*) malloc (28 * sizeof(segment));
    // writeToFile();
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