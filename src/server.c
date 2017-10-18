#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include "segment.h"
#include "util.h"

#define SEGMENTSIZE 9

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

    // setting timeout
    struct timeval tv;
    tv.tv_sec = 0.5;
    tv.tv_usec = 0;  // Not init'ing this can cause strange errors
    setsockopt(*udpSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));

    /*Bind socket with address struct*/
    if(bind(*udpSocket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0){
        die("Couldn't bind socket");
    }

    printf("[%d] socket success on port %d\n", (int) time(0), port);
    fflush(stdout);
}

void initBufferArray(BufferArray* a, int max_segment) {
    // free(a->segments);
    a->segments = (segment*) malloc(max_segment * sizeof(segment));
    a->length = 0;
}

void writeToFile(char* filename, char* message, int n) {
    for(int i=0; i<n; i++){
        printf("WRITE : %c\n", message[i]);
    }
    FILE *fp;
    fp=fopen(filename, "a+");
    fwrite(message, sizeof(message[0]), n, fp);
    fclose(fp);
}

void drainBufferArray(BufferArray* a, char* filename, int max_segment) {
    char temp[a->length];
    for (int i = 0; i < a->length; i++) {
        segment aSegment = *(a->segments + i * SEGMENTSIZE);
        // printf(" %c",(char) aSegment.data);
        temp[i] = (char) aSegment.data;
    }
    // printf("\n");
    writeToFile(filename, temp, a->length);
    free(a->segments);
    initBufferArray(a,max_segment);
}

void insertBufferArray(BufferArray *a, segment aSegment, int buffersize) {
    int curr = a->length * SEGMENTSIZE;
    int last_mem = curr + SEGMENTSIZE;
    // int remainingMemoryAfterInsertion = buffersize - memoryNeeded;

    // printf("CURR %d MEMNEED %d REMAINING %d\n", curr, memoryNeeded, remainingMemoryAfterInsertion);

    if (last_mem >= buffersize){
    } else {
        *(a->segments + curr) = aSegment;
        a->length = a->length + 1;
    }
}

void initFile(char* filename){
    FILE *fp;
    fp=fopen(filename, "w");
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
    // char buf[BUFLEN];

    int udpSocket, len;
    // open connection
    init_socket(&udpSocket, PORT);

    // initial buffer
    int max_segment = BUFLEN / SEGMENTSIZE;
    BufferArray recv_buffer;
    initBufferArray(&recv_buffer,max_segment);

    // initial file
    initFile(FILE_NAME);

    int LFR = -1;
    int LAF = LFR + RWS;
    int seqValid = 1;
    int has_ack[max_segment];
    for(int i=0; i<max_segment; i++) has_ack[i]=0;
    int next_seg = 0;
    int pos = 0;

    while(1){
        char* segment_buff;
        struct sockaddr_in client_addr;
        int client_size = sizeof(client_addr);

        // timeout setting
        fd_set select_fds;
        struct timeval timeout;

        FD_ZERO(&select_fds);
        FD_SET(udpSocket, &select_fds);

        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        segment seg;
        int reload = 0;
        if ( select(32, &select_fds, NULL, NULL, &timeout) == 0 ){  
            printf("Select has timed out...\n");
        } else{
            // receive from client
            segment_buff = (char*) malloc(sizeof(char)*SEGMENTSIZE);
            len = recvfrom(udpSocket,segment_buff,SEGMENTSIZE,0,(struct sockaddr*) &client_addr, &client_size);
            to_segment(&seg,segment_buff);
            free(segment_buff);

            printf("[%d] segment %d caught | have a data %c\n", (int) time(0), seg.seqNum, seg.data);
            fflush(stdout);
            reload = 1;
        }
        // write on file
        // char* c_in = (char *) malloc(sizeof(char));
        // c_in[0] = seg.data;
        // writeToFile(FILE_NAME,c_in,1);
        // free(c_in);

        if(reload){
        
            if(seg.data == 0){
                drainBufferArray(&recv_buffer,FILE_NAME, max_segment);
                printf("FINISH ALL MSG\n");
                break;
            }
            // insert to buffer & accept segment
            insertBufferArray(&recv_buffer,seg,BUFLEN);
            pos = LFR+1;
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
                drainBufferArray(&recv_buffer,FILE_NAME, max_segment);
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
            printf("BERHASIL SEND\n");
            free(raw);
        }
    }
    free(recv_buffer.segments);
    close(udpSocket);

    return 0;
}