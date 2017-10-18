#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "util.h"
#include "segment.h"

#define SEGMENTSIZE 9
#define ACKSIZE 7

typedef struct {
  segment* segments;
  int length;
} BufferArray;

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

int init_buffer(BufferArray* buffer_array){
}

int main(int argc, char *argv[]){
    if(argc < 6){
        die("<filename> <windowsize> <buffersize> <destination_ip> <destination_port>");
    }

    // read from argument
    char* FILE_NAME = argv[1];
    int SWS = char_to_int(argv[2]);
    int BUFLEN = char_to_int(argv[3]);
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
    BufferArray send_buffer;

    int block;
    block = 0;
    int max_segment = BUFLEN / SEGMENTSIZE;
    int LAR = -1, LFS = LAR + SWS;
    int NEXT_SLIDE = 0;
    int pos = 0;
    char* segment_buff;
    char* raw;
    int n;
    while(1){
        int c;
        if(!block){
            n=0;
            printf("[%d] prepare %d segment to buffer\n", time(0), BUFLEN/SEGMENTSIZE);
            while(n < max_segment && (c = fgetc(fp)) != EOF){
                segment seg = create_segment(n,c);
                char* raw = (char*) malloc(SEGMENTSIZE*sizeof(char));
                segment_to_raw(seg, raw);
                seg.checksum = checksum_str(raw, 8);
                raw[8] = seg.checksum;
                for(int i=0; i<SEGMENTSIZE; i++){
                    buf[n*SEGMENTSIZE+i] = raw[i];
                }
                n++;
            }
            max_segment = n;
            printf("---------\n");
            printf(" buffer has %d maximum segment\n", max_segment);
            printf(" buffer data:\n");
            for(int i=0; i<max_segment; i++) printf(" %c", buf[i*SEGMENTSIZE+6]);
            printf("\n---------\n");
            block = 1;
        } else {
            // prepare buffer segment to send
            segment_buff = (char*) malloc(sizeof(char)*SEGMENTSIZE);
            for(int j=0; j<SEGMENTSIZE; j++){
                segment_buff[j] = buf[pos*SEGMENTSIZE+j];
            }
            sendto(clientSocket,segment_buff,SEGMENTSIZE,0,(struct sockaddr *)&client_addr,sizeof(client_addr));
            printf("[%d] segment %d with data %c was send\n", (int) time(0), segment_buff[1], segment_buff[6]);
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
            printf("---------\n");
            printf(" RECEIVE ACK : %d\n", next_seg-1);
            printf(" LAR : %d\n", LAR);
            printf(" SWS : %d\n", SWS);
            printf(" LFS : %d\n", LFS);
            printf(" MAX : %d\n", max_segment);
            printf("---------\n\n");

            printf("POS %d MAX %d\n", pos, max_segment);
            if(pos>=max_segment || pos==0){
                // finish segment
                printf("DONE\n");
                block=0;
                pos=0;
                if(c==EOF){
                    printf("FINISH\n");
                    break;
                }
                continue;
            }
        }
    }
    close(clientSocket);

    return 0;
}