#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "segment.h"
#include "util.h"

#define BUFFERSIZE 256
#define SEGMENTSIZE 9
int BUFFERFULL = 0;

void init_socket(int* udpSocket, int port){
    /*Create UDP socket*/
    *udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(*udpSocket < 0){
        die("Failed create UDP Socket");
    }

    /*Configure settings in address struct*/
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));  
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*Bind socket with address struct*/
    if(bind(*udpSocket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        die("Couldn't bind socket");
    }

    printf("[%d] socket success on port %d\n", (int) time(0), port);
    fflush(stdout);
}


typedef struct {
  segment* recv_buffer;
  int seg_length;
} BufferArray;

void initBufferArray(BufferArray* a) {
    a->recv_buffer = malloc(BUFFERSIZE * sizeof(char));
    a->seg_length = 0;
}

void freeArray(BufferArray *a) {
  free(a->recv_buffer);
  a->recv_buffer = NULL;
  a->seg_length = 0;
}

void drainBufferArray(BufferArray* a) {
    for (int i = 0; i < a->seg_length; i++) {
        segment aSegment = *(a->recv_buffer + i * SEGMENTSIZE);
        writeToFile("output.txt", aSegment.data);
    }
    freeArray(a);
    BUFFERFULL = 0;
}

void insertBufferArray(BufferArray *a, segment aSegment) {
    int curr = a->seg_length * SEGMENTSIZE;
    int memoryNeeded = curr + SEGMENTSIZE;
    int remainingMemoryAfterInsertion = BUFFERSIZE - memoryNeeded;
    
    if (remainingMemoryAfterInsertion < SEGMENTSIZE) {
        printf("Buffer is already full at length %d \n", a->seg_length);
        BUFFERFULL = 1;
    }
    else {
        BUFFERFULL = 0;
        *(a->recv_buffer + curr) = aSegment;
        a->seg_length = a->seg_length + 1;
    }
}

void writeToFile(char* filename, char message) {
    FILE* fp;
    fp = fopen(filename, "a");
    fprintf(fp, "%c", message);
    fclose(fp);
}

int main(int argc, char *argv[]){
    
    if(argc < 5){
        die("<filename> <windowsize> <buffersize> <port>");
    }

    // read from argument
    char* FILE_NAME = argv[1];
    int RWS = char_to_int(argv[2]);
    int BUFLEN = char_to_int(argv[3]);
    int PORT = char_to_int(argv[4]);
    char buf[BUFLEN];

    int udpSocket, len;
    // char buffer[BUFLEN];
    // char* buffer = (char*) malloc(sizeof(char)*9);
    // socklen_t addr_size, client_addr_size;
    // int i;

    // open connection
    init_socket(&udpSocket, PORT);

    // initial buffer
    BufferArray buffer;
    initBufferArray(&buffer);
    buffer.recv_buffer = malloc(BUFLEN * sizeof(char));

    int max_segment = BUFLEN / SEGMENTSIZE;
    int LFR = -1;
    int LAF = LFR + RWS;
    int seqValid = 1;
    int has_ack[max_segment];
    memset(has_ack,0,sizeof(has_ack));

    while(1){
        char* segment_buff = (char*) malloc(sizeof(char)*9);
        struct sockaddr_in client_addr;
        int client_size = sizeof(client_addr);

        len = recvfrom(udpSocket,segment_buff,9,0,(struct sockaddr*) &client_addr, &client_size);

        segment seg;
        to_segment(&seg,segment_buff);

        printf("[%d] segment %d caught\n", (int) time(0), seg.seqNum);
        fflush(stdout);

        insertBufferArray(&buffer,seg);
        // printf("  | Buffer segment length : %d\n", buffer.seg_length);

        // sendto(udpSocket,buffer,len,0,NULL,NULL);

        int next_seg;
        int pos = LFR+1;
        has_ack[seg.seqNum] = 1;
        while(pos <= LAF && pos < max_segment) {
            if (!has_ack[pos]) {
                next_seg = pos;
                LFR=pos-1;
                LAF=(LFR+RWS < max_segment)?LFR+RWS:max_segment-1;
                break;
            }
            pos++;
        }
        printf("--------\n");
        printf("LFR : %d \n", LFR);
        printf("RWS : %d \n", RWS);
        printf("LAF : %d \n", LAF);
        printf("--------\n");
        printf("Next segment number %d \n", next_seg);
        printf("--------\n");

        // int diffWinSize = BUFFERSIZE - (buffer.length*SEGMENTSIZE);
        packet_ack send_ack;
        send_ack.ack = 0x6;
        send_ack.nextSeqNum = next_seg;
        // if (diffWinSize >= RWS ) {
        //     send_ack.windowSize = RWS;   
        // } else
        //     send_ack.windowSize = diffWinSize;
        send_ack.checksum = 0x0;

        char* raw = (char*) malloc(7*sizeof(char));

        // printf("%d\n", send_ack.nextSeqNum);
        ack_to_raw(send_ack,raw);
        send_ack.checksum = checksum_str(raw,6);
        raw[6] = send_ack.checksum;

        // printf("%d\n", raw[1]);

        sendto(udpSocket,raw,7,0,(struct sockaddr*) &client_addr, client_size);
    }

    return 0;
}