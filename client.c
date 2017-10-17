#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "util.h"
#include "segment.h"

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

    char *has_ack = (char *) malloc(SWS * sizeof(char));
    char *msg_ws = (char *) malloc(SWS * sizeof(char));

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
        if(!block){
            int n=0, c;
            while(n < BUFLEN/sizeof(segment) && (c = fgetc(fp)) != EOF){
                segment seg;
                char* raw = (char*) malloc(9*sizeof(char));
                seg.soh = 0x1;
                seg.seqNum = n;
                seg.stx = 0x2;
                seg.data = (char)c;
                seg.etx = 0x3;
                segment_to_raw(seg, raw);
                // printf("%c\n", raw[6]);
                for(int i=0; i<9; i++){
                    send_buff[n*9+i] = raw[i];
                }
                // printf("%d\n", send_buff[n*9+1]);
                n++;
            }
            NEXT_SEG_FROM_BUFFER = n;
            block = 1;
        } else {
            char* raw = (char*) malloc(9*sizeof(char));
            nBytes = recvfrom(clientSocket,raw,9,0,NULL, NULL);

            packet_ack ack;
            to_ack(&ack, raw);
            int cur_ack = ack.nextSeqNum-1;
            int first_seg = LAR+1;
            int pos = cur_ack-first_seg;
            has_ack[pos] = 1;

            if(pos==0){
                // SLIDE
                int n=0;
                for(int i=0; i<SWS; i++,n++){
                    if(!has_ack[i]){
                        break;
                    }
                }
                for(int i=0; i<SWS-n; i++){
                    has_ack[i] = has_ack[i+n];
                }
                for(int i=SWS-n; i<SWS; i++){
                    has_ack[i] = NULL;
                }

                LAR+=n;
                LFS=LAR+SWS; // sementara
                if(LFS>=NEXT_SEG_FROM_BUFFER){
                    block=0;
                }
            }
            for(int i=LAR+1; i<=LFS; i++){
                sendto(clientSocket,send_buff[i*9],9,0,(struct sockaddr *)&serverAddr,addr_size);
            }
        }
    }
    
        // printf("Type a sentence to send to server:\n");
        // fgets(buffer,BUFLEN,stdin);
        // printf("You typed: %s",buffer);
        // nBytes = strlen(buffer) + 1;

        // /*Send message to server*/
        // sendto(clientSocket,buffer,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);

        // /*Receive message from server*/
        // nBytes = recvfrom(clientSocket,buffer,BUFLEN,0,NULL, NULL);

        // printf("Received from server: %s\n",buffer);

    return 0;
}