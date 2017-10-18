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

typedef struct {
  segment* segments;
  int length;
} BufferArray;

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

void initBufferArray(BufferArray* a) {
    a->segments = malloc(BUFFERSIZE * sizeof(char));
    a->length = 0;
}

void freeArray(BufferArray *a) {
  free(a->segments);
  initBufferArray(a);
}

void drainBufferArray(BufferArray* a) {
    for (int i = 0; i < a->length; i++) {
        segment aSegment = *(a->segments + i * SEGMENTSIZE);
        printf(" %c",(char) aSegment.data);
        writeToFile("output.txt", (char) aSegment.data);
    }
    printf("\n");
    freeArray(a);
    BUFFERFULL = 0;
}

void insertBufferArray(BufferArray *a, segment aSegment) {
    int curr = a->length * SEGMENTSIZE;
    int memoryNeeded = curr + SEGMENTSIZE;
    int remainingMemoryAfterInsertion = BUFFERSIZE - memoryNeeded;

    // printf("CURR %d MEMNEED %d REMAINING %d\n", curr, memoryNeeded, remainingMemoryAfterInsertion);

    if (remainingMemoryAfterInsertion < SEGMENTSIZE) {
        BUFFERFULL = 1;
    }
    else {
        BUFFERFULL = 0;
        *(a->segments + curr) = aSegment;
        a->length = a->length + 1;
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
    // open connection
    init_socket(&udpSocket, PORT);

    // initial buffer
    BufferArray recv_buffer;
    initBufferArray(&recv_buffer);
    recv_buffer.segments = malloc(BUFLEN * sizeof(char));

    int max_segment = BUFLEN / SEGMENTSIZE;
    int LFR = -1;
    int LAF = LFR + RWS;
    int seqValid = 1;
    // int* has_ack = (int*) malloc(sizeof(int)*max_segment);
    int has_ack[max_segment];
    for(int i=0; i<max_segment; i++) has_ack[i]=0;
    // memset(has_ack,0,sizeof(has_ack));

    while(1){
        char* segment_buff = (char*) malloc(sizeof(char)*9);
        struct sockaddr_in client_addr;
        int client_size = sizeof(client_addr);

        // receive from client
        len = recvfrom(udpSocket,segment_buff,9,0,(struct sockaddr*) &client_addr, &client_size);
        segment seg;
        to_segment(&seg,segment_buff);
        printf("[%d] segment %d caught | have a data %c\n", (int) time(0), seg.seqNum, seg.data);
        fflush(stdout);

        // insert to buffer & accept segment
        insertBufferArray(&recv_buffer,seg);
        int next_seg;
        int pos = LFR+1;
        printf("%d\n", seg.seqNum);
        for(int i=0; i<max_segment; i++){
            printf("%d", has_ack[i]);
        }
        printf("\n");
        has_ack[seg.seqNum] = 1;
        int all_full = 1;
        while(pos <= LAF && pos < max_segment) {
            if (!has_ack[pos]) {
                next_seg = pos;
                LFR=pos-1;
                LAF=(LFR+RWS < max_segment)?LFR+RWS:max_segment-1;
                all_full = 0;
                break;
            }
            pos++;
        }

        // if next segment hit maximum allowed buffer
        if(all_full){
            drainBufferArray(&recv_buffer);
            for(int i=0; i<max_segment; i++) has_ack[i]=0;
            LFR = -1;
            pos = 0;
            LAF = (LFR+RWS < max_segment)?LFR+RWS:max_segment-1;
            next_seg = LFR+1;
        }

        printf("---------\n");
        printf(" LFR : %d \n", LFR);
        printf(" RWS : %d \n", RWS);
        printf(" LAF : %d \n", LAF);
        printf(" MAX : %d\n", max_segment);
        printf(" --------\n");
        printf(" Next segment number %d \n", next_seg);
        printf("---------\n\n");

        packet_ack send_ack;
        send_ack.ack = 0x6;
        send_ack.nextSeqNum = next_seg;
        send_ack.checksum = 0x0;

        char* raw = (char*) malloc(7*sizeof(char));

        ack_to_raw(send_ack,raw);
        send_ack.checksum = checksum_str(raw,6);
        raw[6] = send_ack.checksum;

        sendto(udpSocket,raw,7,0,(struct sockaddr*) &client_addr, client_size);
        printf("BERHAILSEND\n");
    }

    return 0;
}