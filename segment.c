#include "segment.h"

void segment_to_raw(segment seg, char* raw) {
	raw[0] = seg.soh;
	char* x = (char*) &seg.seqNum;
	raw[1] = *x;
	raw[2] = *(x+1);
	raw[3] = *(x+2);
	raw[4] = *(x+3);
	raw[5] = seg.stx;
	raw[6] = seg.data;
	raw[7] = seg.etx;
	raw[8] = seg.checksum;
}

void to_segment(segment* seg, char* raw) {
	seg->soh = *raw;
	seg->seqNum = ((int) *(raw+1)) + ((int) *(raw+2)<<8) + ((int) *(raw+3)<<16) + ((int) *(raw+4)<<24);
	seg->stx =  *(raw+5);
	seg->data = *(raw+6);
	seg->etx = *(raw+7);
	seg->checksum = *(raw+8);
}

void ack_to_raw(packet_ack ack_seg, char* raw) {
	raw[0] = ack_seg.ack;
	char* x = (char*) &ack_seg.nextSeqNum;
	raw[1] = *x;
	raw[2] = *(x+1);
	raw[3] = *(x+2);
	raw[4] = *(x+3);
	raw[5] = ack_seg.windowSize;
	raw[6] = ack_seg.checksum;
}

void to_ack(packet_ack* ack_seg, char* raw) {
	ack_seg->ack = *raw;
	ack_seg->nextSeqNum = ((int) *(raw+1)) + ((int) *(raw+2)<<8) + ((int) *(raw+3)<<16) + ((int) *(raw+4)<<24);
	ack_seg->windowSize = *(raw+5);
	ack_seg->checksum = *(raw+6);	
}