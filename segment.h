#ifndef SEGMENT_H
#define SEGMENT_H

#include <stdio.h>
#include <stdlib.h>
#include "segment.h"

typedef struct {
	char soh;
	int seqNum;
	char stx;
	char data;
	char etx;
	char checksum;
} segment;

typedef struct {
	char ack;
	int nextSeqNum;
	char windowSize;
	char checksum;
} packet_ack;

void segment_to_buffer(segment seg, char* buffer);

void to_segment(segment seg, char* buffer);

void ack_to_buffer(packet_ack ack_seg, char* buffer);

void to_ack(packet_ack ack_seg, char* buffer);

#endif SEGMENT_H