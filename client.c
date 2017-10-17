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
    int WS = char_to_int(argv[2]);
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

    char *has_ack = (char *) malloc(WS * sizeof(char));
    char *msg_ws = (char *) malloc(WS * sizeof(char));

    FILE *fp;
    fp = fopen(FILE_NAME, "r");
    if(fp == NULL){
        die("Couldn't open file");
    }

    char *msg_buff = malloc(BUFLEN * sizeof(char));
    int n=0, c;
    while(1){
        while(n < BUFLEN/9 && (c = fgetc(fp)) != EOF){
            segment seg;
            char* raw;
            seg.soh = 0x1;
            seg.seqNum = n;
            seg.stx = 0x2;
            seg.data = (char)c;
            seg.etx = 0x3;
            segment_to_raw(seg, &raw);
            seg.checksum = checksum_str(raw,8);
            msg_buff[n*9] = raw;
            // for(int i=0; i<9; i++){
            //     msg_buff+(n*9+1) = raw+i;
            // }
            n++;
        }
        printf("%s", msg_buff[n*9]);
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