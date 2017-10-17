#include "segment.h"

void segment_to_buffer(segment seg, char* buffer) {
	buffer[0] = seg.soh;
	char* x = (char*) &seg.seqNum;
	buffer[1] = *x;
	buffer[2] = *(x+1);
	buffer[3] = *(x+2);
	buffer[4] = *(x+3);
	buffer[5] = seg.stx;
	buffer[6] = seg.data;
	buffer[7] = seg.etx;
	buffer[8] = seg.checksum;
}

void to_segment(segment* seg, char* buffer) {
	seg->soh = *buffer;
	seg->seqNum = ((int) *(buffer+1)) + ((int) *(buffer+2)<<8) + ((int) *(buffer+3)<<16) + ((int) *(buffer+4)<<24);
	seg->stx =  *(buffer+5);
	seg->data = *(buffer+6);
	seg->etx = *(buffer+7);
	seg->checksum = *(buffer+8);
}

void ack_to_buffer(packet_ack ack_seg, char* buffer) {
	buffer[0] = ack_seg.ack;
	char* x = (char*) &ack_seg.nextSeqNum;
	buffer[1] = *x;
	buffer[2] = *(x+1);
	buffer[3] = *(x+2);
	buffer[4] = *(x+3);
	buffer[5] = ack_seg.windowSize;
	buffer[6] = ack_seg.checksum;
}

void to_ack(packet_ack* ack_seg, char* buffer) {
	ack_seg->ack = *buffer;
	ack_seg->nextSeqNum = ((int) *(buffer+1)) + ((int) *(buffer+2)<<8) + ((int) *(buffer+3)<<16) + ((int) *(buffer+4)<<24);
	ack_seg->windowSize = *(buffer+5);
	ack_seg->checksum = *(buffer+6);	
}