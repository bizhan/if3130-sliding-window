#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "util.h"
#include "segment.h"

#define SEGMENTSIZE 9

int main(int argc, char *argv[]){
    if(argc < 6){
        die("<filename> <windowsize> <buffersize> <destination_ip> <destination_port>");
    }

    // read from argument
    char* FILE_NAME = argv[1];
    int SWS = char_to_int(argv[2]);
    int BUFLEN = char_to_int(argv[3]);
    int DEST_IP = char_to_int(argv[4]);
    int DEST_PORT = char_to_int(argv[5]);
    char buf[BUFLEN];

    int clientSocket, portNum, nBytes;
    char buffer[BUFLEN];
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    /*Create UDP socket*/
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    /*Configure settings in address struct*/
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(DEST_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

    /*Initialize size variable to be used later on*/
    addr_size = sizeof serverAddr;

    FILE *fp;
    fp = fopen(FILE_NAME, "r");
    if(fp == NULL){
        die("Couldn't open file");
    }

    char *send_buff = malloc(BUFLEN * sizeof(char));
    int block;
    block = 0;
    int LAR = -1, LFS = LAR + SWS;
    int NEXT_SLIDE = 0;
    int NEXT_SEG_FROM_BUFFER = 0;
    while(1){
        int c;
        if(!block){
            int n=0;
            while(n < BUFLEN/sizeof(segment) && (c = fgetc(fp)) != EOF){
                segment seg;
                char* raw = (char*) malloc(SEGMENTSIZE*sizeof(char));
                seg.soh = 0x1;
                seg.seqNum = n;
                seg.stx = 0x2;
                seg.data = (char)c;
                seg.etx = 0x3;
                seg.checksum = 0x0;
                segment_to_raw(seg, raw);
                seg.checksum = checksum_str(raw, 8);
                raw[8] = seg.checksum;
                for(int i=0; i<SEGMENTSIZE; i++){
                    send_buff[n*SEGMENTSIZE+i] = raw[i];
                }
                n++;
            }
            NEXT_SEG_FROM_BUFFER = n;
            block = 1;
        } else {
            for(int i=LAR+1; i<=LFS; i++){
                char* segment_buff = (char*) malloc(sizeof(char)*9);
                for(int j=0; j<SEGMENTSIZE; j++){
                    segment_buff[j] = send_buff[i*SEGMENTSIZE+j];
                }
                printf("%d\n", segment_buff[1]);
                sendto(clientSocket,segment_buff,SEGMENTSIZE,0,(struct sockaddr *)&serverAddr,addr_size);
            }
            char* raw = (char*) malloc(SEGMENTSIZE*sizeof(char));
            nBytes = recvfrom(clientSocket,raw,SEGMENTSIZE,0,NULL, NULL);

            packet_ack ack;
            to_ack(&ack, raw);
            int next_seg = ack.nextSeqNum;
            LAR=next_seg-1;
            LFS=(LAR+SWS<NEXT_SEG_FROM_BUFFER)?LAR+SWS:NEXT_SEG_FROM_BUFFER-1;
            if(LAR==LFS){
                block=0;
                if(c==EOF){
                    exit(1);
                }
            }
        }
    }

    return 0;
}