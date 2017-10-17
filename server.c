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

int windowSize = WINDOWSIZE;
int* arrayAck;
int BUFFERFULL = 0;
int SHOULDSTOP = 0;

typedef struct {
  segment* array;
  int length;
} BufferArray;

void initBufferArray(BufferArray* a) {
    a->array = malloc(BUFFERSIZE * sizeof(char));
    a->length = 0;
}

void freeArray(BufferArray *a) {
  free(a->array);
  a->array = NULL;
  a->length = 0;
}

void drainBufferArray(BufferArray* a) {
    printf("Buffer is being drained.\n");
    for (int i = 0; i < a->length; i++) {
        segment aSegment = *(a->array + i * SEGMENTSIZE);
        printf("%c", aSegment.data);
    }
    printf("\n");
    freeArray(a);
    BUFFERFULL = 0;
    printf("Buffer is already empty.\n");
}

int insertBufferArray(BufferArray *a, segment aSegment) {
    int usedMemory = a->length * SEGMENTSIZE;
    if (BUFFERSIZE - usedMemory > SEGMENTSIZE) {
        *(a->array + a->length * SEGMENTSIZE) = aSegment;
        a->length += 1;
        return 1;
    }
    else {
        BUFFERFULL = 1;
        printf("Buffer already full.\n");
        return 0;
    }
}

void writeToFile() {
    FILE* fp;
    fp = fopen("output.txt", "a");
    fprintf(fp, "This is the sample output\n");
    fclose(fp);
}

char* readFile() {
    FILE* fp;
    fp = fopen("input.txt", "r");
    char* string = (char*) malloc (sizeof(char));
    int charCount = 0;
    char cc;
    while ((cc = fgetc(fp)) != EOF) {
        charCount++;
        string = (char*) realloc (string, charCount * sizeof(char));
        *(string + ((charCount - 1) * sizeof(char))) = cc;
    }
    return string;
}

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

void readingFile(void *vargp) {
    // char* string = readFile();
    // int count = 0;
    // printf("%lu", strlen(string));
    // while (count < strlen(string)) {
    //     char cc = *(string + count * SEGMENTSIZE);
    //     segment aSegment = generateSegment(count, cc, count * -1);
    //     if (BUFFERFULL == 0) {
    //         insertBufferArray(&recvbuffer, aSegment);
    //         count++;
    //     }
    // }
    // SHOULDSTOP = 1;
}

void writingFile(void *vargp) {
    for (int i = 0; i < 100; i++) {
        printf("writingFile.\n");
    }
    // while(SHOULDSTOP == 0) {
    //     if (BUFFERFULL == 1) {
    //         drainBufferArray(&recvbuffer);
    //     }
    // }
}

int main(int argc, char *argv[]){

    // pthread_t thread1, thread2;
    // int  iret1, iret2;
    // pthread_create(&thread1, NULL, readingFile, NULL);
    // pthread_create( &thread2, NULL, writingFile, NULL);
    // /* Wait till threads are complete before main continues. Unless we  */
    // /* wait we run the risk of executing an exit which will terminate   */
    // /* the process and all threads before the threads have completed.   */
    // pthread_join( thread1, NULL);
    // pthread_join( thread2, NULL);
    // exit(EXIT_SUCCESS);

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
    if(argc < 5){
        die("<filename> <windowsize> <buffersize> <port>");
    }

    // read from argument
    char* FILE_NAME = argv[1];
    int WS = char_to_int(argv[2]);
    int BUFLEN = char_to_int(argv[3]);
    int PORT = char_to_int(argv[4]);
    char buf[BUFLEN];

    int udpSocket, nBytes;
    // char buffer[BUFLEN];
    char* buffer = (char*) malloc(sizeof(char)*9);
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size, client_addr_size;
    int i;

    /*Create UDP socket*/
    udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(udpSocket < 0){
        die("Failed create UDP Socket");
    }

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    /*Bind socket with address struct*/
    if(bind(udpSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0){
        die("Couldn't bind socket");
    }

    BufferArray recvbuffer;
    initBufferArray(&recvbuffer);
    // recvbuffer.array = malloc(BUFLEN * sizeof(char));

    while(1){
        char* segment_buff = (char*) malloc(sizeof(char)*9);
        nBytes = recvfrom(udpSocket,segment_buff,9,0,NULL, NULL);

        segment seg;
        to_segment(&seg,segment_buff);

        insertBufferArray(&recvbuffer,seg);

        sendto(udpSocket,buffer,nBytes,0,NULL,NULL);
    }

    return 0;
}