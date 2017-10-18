#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "util.h"
#include "segment.h"

// GLOBAL
#define SEGMENTSIZE 9
#define ACKSIZE 7

int SWS;
int BUFLEN;


typedef struct {
  segment* segments;
  int length;
} BufferArray;

void initBufferArray(BufferArray* a, int max_segment) {
    free(a->segments);
    a->segments = (segment*) malloc(max_segment * sizeof(segment));
    a->length = 0;
}

int init_socket_address(int* clientSocket, struct sockaddr_in *address, char* ip, int port){
    /*Create UDP socket*/
    *clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(*clientSocket < 0){
        die("Failed create UDP Socket");
    }

    /*Configure settings in address struct*/
    memset(address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(port); 
    inet_aton(ip , &address->sin_addr);

    printf("[%d] success on address %s port %d\n", (int) time(0), ip, port);
    fflush(stdout);
    return 1;
}

int main(int argc, char *argv[]){
    if(argc < 6){
        die("<filename> <windowsize> <buffersize> <destination_ip> <destination_port>");
    }

    // read from argument
    char* FILE_NAME = argv[1];
    SWS = char_to_int(argv[2]);
    BUFLEN = char_to_int(argv[3]);
    char* DEST_IP = argv[4];
    int DEST_PORT = char_to_int(argv[5]);
    char buf[BUFLEN];

    int clientSocket, len;
    struct sockaddr_in client_addr;

    // open connection
    init_socket_address(&clientSocket, &client_addr, DEST_IP, DEST_PORT);

    // open file to read
    FILE *fp;
    fp = fopen(FILE_NAME, "r");
    if(fp == NULL){
        die("Couldn't open file");
    }
    // initial buffer
    int max_segment = BUFLEN / SEGMENTSIZE;
    BufferArray send_buffer;
    initBufferArray(&send_buffer,max_segment);

    int block;
    block = 0;
    int LAR = -1, LFS = LAR + SWS;
    int pos = 0;
    char* segment_buff;
    char* raw;
    int n;
    // srand(time(NULL));
    while(1){
        // int r = rand() % 1000;
        int c;
        if(!block){
            pos=0;
            n=0;
            initBufferArray(&send_buffer,max_segment);
            printf("[%d] prepare %d segment to buffer\n", time(0), max_segment);
            fflush(stdout);
            while(n < max_segment && (c = fgetc(fp)) != EOF){
                segment seg = create_segment(n,c);
                char* raw = (char*) malloc(SEGMENTSIZE*sizeof(char));
                segment_to_raw(seg, raw);
                seg.checksum = checksum_str(raw, 8);
                send_buffer.segments[n] = seg;
                // raw[8] = seg.checksum;
                // for(int i=0; i<SEGMENTSIZE; i++){
                //     buf[n*SEGMENTSIZE+i] = raw[i];
                // }
                n++;
            }
            max_segment = n;
            printf("---------\n"); fflush(stdout);
            printf(" buffer has %d maximum segment\n", max_segment); fflush(stdout);
            printf(" buffer data:\n"); fflush(stdout);
            for(int i=0; i<max_segment; i++) {
                printf(" %c", send_buffer.segments[i].data); fflush(stdout);
            }
            printf("\n---------\n"); fflush(stdout);
            block = 1;
        } else {
            // prepare buffer segment to send
            segment_buff = (char*) malloc(sizeof(char)*SEGMENTSIZE);
            segment_to_raw(send_buffer.segments[pos],segment_buff);
            // for(int j=0; j<SEGMENTSIZE; j++){
            //     segment_buff[j] = buf[pos*SEGMENTSIZE+j];
            // }
            // if(r<100){
                sendto(clientSocket,segment_buff,SEGMENTSIZE,0,(struct sockaddr *)&client_addr,sizeof(client_addr));
            // }
            printf("[%d] segment %d with data %c was send\n", (int) time(0), segment_buff[1], segment_buff[6]); fflush(stdout);
            free(segment_buff);

            // preapare raw to receive ACK
            raw = (char*) malloc(ACKSIZE*sizeof(char));
            len = recvfrom(clientSocket,raw,ACKSIZE,0,NULL, NULL);

            packet_ack ack;
            to_ack(&ack, raw);
            free(raw);

            int next_seg = ack.nextSeqNum;
            pos = next_seg;
            LAR=next_seg-1;
            LFS=(LAR+SWS<max_segment)?LAR+SWS:max_segment-1;
            printf("---------\n"); fflush(stdout);
            printf(" RECEIVE ACK : %d\n", next_seg-1); fflush(stdout);
            printf(" LAR : %d\n", LAR); fflush(stdout);
            printf(" SWS : %d\n", SWS); fflush(stdout);
            printf(" LFS : %d\n", LFS); fflush(stdout);
            printf(" MAX : %d\n", max_segment); fflush(stdout);
            printf("---------\n\n"); fflush(stdout);

            printf("POS %d MAX %d\n", pos, max_segment); fflush(stdout);
            if(pos>=max_segment || pos==0){
                // finish segment
                printf("DONE\n"); fflush(stdout);
                block=0;
                if(c==EOF){
                    // send last sentinel
                    segment s = create_sentinel();
                    segment_buff = (char*) malloc(sizeof(char)*SEGMENTSIZE);
                    segment_to_raw(s,segment_buff);
                    sendto(clientSocket,segment_buff,SEGMENTSIZE,0,(struct sockaddr *)&client_addr,sizeof(client_addr));
                    free(segment_buff);
                    printf("FINISH\n"); fflush(stdout);
                    break;
                }
                continue;
            }
        }
    }
    close(clientSocket);

    return 0;
}