#ifndef PACKET_QUEUE_H
#define PACKET_QUEUE_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "hal.h"
#include "ch.h"


#define MAX_QUEUE_LENGTH  10      /* Number of packets in queue */
#define MAX_DIGIPEATERS   8       /* Maximum number of digipeaters */
#define ADDRESS_LENGTH    7       /* Number of characters in address field */
#define PAYLOAD_LENGTH    256     /* Maximum size of packet payload */
#define MAX_PACKET_BYTES  352     /* Maximum amount of bytes in a packet */
#define SD_TERMINAL_PORT  SD6     /* Serial Device to direct the output */

#define SD_TERMINAL       (BaseSequentialStream*)&SD_TERMINAL_PORT

/* Packet Data Structure */
typedef struct _Packet
{
  /* Source Callsign + SSID */
  char source_address[ADDRESS_LENGTH];

  /* Destination Callsign + SSID */
  char destination_address[ADDRESS_LENGTH];

  /* Digipeater Callsign + SSID */
  char digipeaters[MAX_DIGIPEATERS][ADDRESS_LENGTH];
  
  /* Number of digipeaters */
  int num_digipeaters;

  /* Control Field */
  char control_field;

  /* Protocol Identifier field */
  char pid;

  /* Packet Payload */
  char payload[PAYLOAD_LENGTH];
  
  /* Size of the stored payload in bytes */
  int payload_length;

} Packet;

/* Packet Queue Data Structure */
typedef struct _Queue
{
  /* Index of the start of queue */
  int front;

  /* Index of the end of queue */
  int rear;

  /* Array to hold packet queue */
  Packet packet_queue[MAX_QUEUE_LENGTH];
} Queue;

/* FUNCTION PROTOTYPES */
void init_packet(Packet*);
void init_queue(Queue*);
void get_packet_source(Packet*, char*, char*);
void get_packet_destination(Packet*, char*, char*);
int get_packet_digipeater(Packet*, int, char*, char*);
void set_packet_source(Packet*, char*, char);
void set_packet_destination(Packet*, char*, char);
int add_packet_digipeater(Packet*, char*, char);
void set_packet_payload(Packet*, char*);
int push_packet_to_queue(Queue*, Packet*);
int pop_packet_from_queue(Queue*, Packet*);
void display_packet(Packet*);
void display_packet_queue(Queue*);
int frame_packet_bytes(Packet*, char*);


#endif
