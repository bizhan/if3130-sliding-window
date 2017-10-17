 while(1){
        int LFR = 0;
        int RWS = WINDOWSIZE;
        int LAF = LFR + RWS;
        int num_ack;
        int i = 0;
        int seqValid = 1;
        while(seqValid send_&& i < LAF) {
            if (recvbuffer.array+i*SEGMENTSIZE+1 != i) {
                seqValid = 0;
                num_ack = i;
            } else
            i++;
        }
        num_ack = i;

        int diffWinSize = BUFFERSIZE - (recvbuffer.length*SEGMENTSIZE);
        packet_ack send_ack;
        send_ack.ack = 0x6;
        send_ack.nextSeqNum = num_ack;
        if (diffWinSize >= WINDOWSIZE ) {
            send_ack.windowSize = WINDOWSIZE;   
        } else
            send_ack.windowSize = diffWinSize;
        send_ack.checksum = 0x0;

        char* raw = (char*) malloc(9*sizeof(char));

        ack_to_raw(send_ack,raw);
        send_ack.checksum = checksum_str(raw,6);
        raw[6] = send_ack.checksum;

        sendto(udpSocket,raw,7,0,NULL,addr_size);

}