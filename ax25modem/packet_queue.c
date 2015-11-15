#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "chtypes.h"
#include "chstreams.h"
#include "chprintf.h"
#include "packet_queue.h"
#include "crc.h"

/* DEFINITION OF FUNCTIONS */

/**
 *  init_packet(Packet* pkt)
 *  
 *  Initializes a packet datastructure in the address pointed by pkt.
 */
void init_packet(Packet* pkt)
{
  int i;

  memset(pkt->source_address, 32, (ADDRESS_LENGTH - 1) * sizeof(char));
  pkt->source_address[(ADDRESS_LENGTH - 1)] = 0x00;

  memset(pkt->destination_address, 32, (ADDRESS_LENGTH - 1) * sizeof(char));
  pkt->destination_address[(ADDRESS_LENGTH - 1)] = 0x00;
  
  for (i = 0; i < MAX_DIGIPEATERS; i++)
  {  
    memset(pkt->digipeaters[i], 32, (ADDRESS_LENGTH - 1) * sizeof(char));
    pkt->digipeaters[i][(ADDRESS_LENGTH - 1)] = 0x00;
  }
  pkt->num_digipeaters = 0;

  pkt->control_field = 0x30;
  pkt->pid = 0xF0;
  
  memset(pkt->payload, 0, PAYLOAD_LENGTH * sizeof(char));
  pkt->payload_length = 0;
}

/**
 *  init_queue(Queue* queue)
 *  
 *  Initializes a packet queue data structure in the address pointed by queue.
 */
void init_queue(Queue* queue)
{
  queue->front = -1;
  queue->rear = -1;
  memset(queue->packet_queue, 0, MAX_QUEUE_LENGTH * sizeof(Packet));  
}

/**
 *  get_packet_source(Packet* pkt, char* callsign, char* ssid)
 *  
 *  Get the address of the source for the given packet.
 */
void get_packet_source(Packet* pkt, char* callsign, char* ssid)
{
  int i;

  for (i = 0; i < (ADDRESS_LENGTH - 1); i++)
    callsign[i] = (pkt->source_address[i] >> 1) & 0x7f;

  *ssid = (pkt->source_address[ADDRESS_LENGTH - 1] >> 1) & 0x0f;
}

/**
 *  get_packet_destination(Packet* pkt, char* callsign, char* ssid)
 *  
 *  Get the address of the destination for the given packet.
 */
void get_packet_destination(Packet* pkt, char* callsign, char* ssid)
{
  int i;

  for (i = 0; i < (ADDRESS_LENGTH - 1); i++)
    callsign[i] = (pkt->destination_address[i] >> 1) & 0x7f;

  *ssid = (pkt->destination_address[ADDRESS_LENGTH - 1] >> 1) & 0x0f;
}

/**
 *  get_packet_digipeater(Packet* pkt, int n, char* callsign, char* ssid)
 *  
 *  Get the address of the n-th digipeater of the given packet.
 *  
 *  Returns 1 for the last digipeater, 0 for all others.
 *  If n is greater than the number of digipeaters available -1 is returned.
 */
int get_packet_digipeater(Packet* pkt, int n, char* callsign, char* ssid)
{
  int i;

  if (n > pkt->num_digipeaters) return -1;

  for (i = 0; i < (ADDRESS_LENGTH - 1); i++)
    callsign[i] = (pkt->digipeaters[n][i] >> 1) & 0x7f;

  *ssid = (pkt->digipeaters[n][ADDRESS_LENGTH - 1] >> 1) & 0x0f;

  return (pkt->digipeaters[n][ADDRESS_LENGTH - 1] & 0x01);
}

/**
 *  set_packet_source(Packet* pkt, char* callsign, char ssid)
 *
 *  Set the packet source address to the callsign and ssid
 */
void set_packet_source(Packet* pkt, char* callsign, char ssid)
{
  int n; 
  int i;

  if (callsign == NULL) return;

  n = strlen(callsign);
  for (i = 0; i < ADDRESS_LENGTH - 1; i++)
    pkt->source_address[i] = (((i < n) ? callsign[i] : 32) << 1) & 0xFE;
  pkt->source_address[ADDRESS_LENGTH - 1] = (0x60 | ((ssid & 0x0F) << 1)) \
                                                  & 0xFE;
}

/**
 *  set_packet_destination(Packet* pkt, char* callsign, char ssid)
 *
 *  Set the packet destination address to the callsign and ssid
 */
void set_packet_destination(Packet* pkt, char* callsign, char ssid)
{
  int n; 
  int i;

  if (callsign == NULL) return;

  n = strlen(callsign);
  for (i = 0; i < ADDRESS_LENGTH - 1; i++)
    pkt->destination_address[i] = (((i < n) ? callsign[i] : 32) << 1) & 0xFE;
  pkt->destination_address[ADDRESS_LENGTH - 1] = (0x60 | ((ssid & 0x0F) << 1)) \
                                                       & 0xFE;
}

/**
 *  int add_packet_digipeater(Packet* pkt, char* callsign, char ssid)
 *
 *  Add a digipeater to the packet structure.  
 *  
 *  Returns -1 if the callsign is NULL and -2 if the number of digipeaters for
 *  the packet has reached the maximum limit set in MAX_DIGIPEATERS.
 *
 *  Function Returns a positive number equal to the number of digipeaters
 *  including the added one, assigned for the packet.
 */
int add_packet_digipeater(Packet* pkt, char* callsign, char ssid)
{
  int n; 
  int i;

  if (callsign == NULL) return -1;
  if (pkt->num_digipeaters >= MAX_DIGIPEATERS) return -2;

  /* The previous digipeater entry is not the last one - Turn last bit to 0 */
  if (pkt->num_digipeaters > 0 )
    pkt->digipeaters[pkt->num_digipeaters - 1][ADDRESS_LENGTH - 1] &= 0xFE;

  n = strlen(callsign);
  for (i = 0; i < ADDRESS_LENGTH - 1; i++)
    pkt->digipeaters[pkt->num_digipeaters][i] = (((i < n) ? callsign[i] : 32) \
                                                          << 1) & 0xFE;

  pkt->digipeaters[pkt->num_digipeaters++][ADDRESS_LENGTH - 1] = \
                        (0x60 | ((ssid & 0x0F) << 1)) | 0x01;

  return pkt->num_digipeaters;
}

/**
 *  void set_packet_payload(Packet* pkt, char* payload)
 *
 *  Set the packet's payload to the string in payload
 */
void set_packet_payload(Packet* pkt, char* payload)
{   
  int n;

  n = strlen(payload);
  n = (n > 256) ? 256 : n;
  strncpy(pkt->payload, payload, n * sizeof(char));
  pkt->payload_length = n;
}

/**
 *  int push_packet_to_queue(Queue* queue, Packet* pkt)
 * 
 *  Add packet pkt to queue and modify the front and rear pointers accordingly.
 *
 *  Returns 0 if the packet is successfully added to the queue, -1 if the queue
 *  was full and the add failed.
 *
 */
int push_packet_to_queue(Queue* queue, Packet* pkt)
{
    if ((queue->front == queue->rear + 1) || \
       ((queue->front == 0) && (queue->rear == (MAX_QUEUE_LENGTH - 1))))
    {
      return -1;
    }
    else
    {
      queue->front = (queue->front == -1) ? 0 : queue->front;
      (queue->rear) = ((queue->rear) + 1) % MAX_QUEUE_LENGTH;
      memcpy(&(queue->packet_queue[(queue->rear)]), pkt, 1 * sizeof(Packet));
    }
    return 0;
}

/**
 *  int pop_packet_from_queue(Queue* queue, Packet* pkt)
 *
 *  Pop a packet from the packet queue and store in pkt
 *
 *  Return -1 if the queue is empty, 0 if the pop was successful.
 */
int pop_packet_from_queue(Queue* queue, Packet* pkt)
{
  if (queue->front == -1)
  {
    return -1;
  }
  else
  {
    memset(pkt, 0, 1 * sizeof(Packet)); 
    memcpy(pkt, &(queue->packet_queue[queue->front]), 1 * sizeof(Packet));
    queue->front = ((queue->front) + 1) % MAX_QUEUE_LENGTH;

    if ((queue->front == queue->rear + 1) || \
       ((queue->front == 0) && (queue->rear == (MAX_QUEUE_LENGTH - 1))))
    {
      queue->front = -1;
      queue->rear = -1;
    }
  }
  return 0;
}

/**
 *  void display_packet(Packet* pkt)
 *
 *  Display the fields of the packet pkt in a user-friendly fashion 
 *  on the terminal.
 */
void display_packet(Packet* pkt)
{
  int n;
  int i;
  char address_string[ADDRESS_LENGTH + 2];
  
  if (pkt == NULL) return;

  chprintf(SD_TERMINAL, "-------------------------------\n");
  memset(address_string, 0, (ADDRESS_LENGTH + 2) * sizeof(char));
  get_packet_source(pkt, address_string, address_string + ADDRESS_LENGTH);
  address_string[ADDRESS_LENGTH - 1] = '-';
  address_string[ADDRESS_LENGTH] += '0';
  chprintf(SD_TERMINAL, "  Source : %s\n",address_string);
  
  memset(address_string, 0, (ADDRESS_LENGTH + 2) * sizeof(char));
  get_packet_destination(pkt, address_string, address_string + ADDRESS_LENGTH);
  address_string[ADDRESS_LENGTH - 1] = '-';
  address_string[ADDRESS_LENGTH] += '0';
  chprintf(SD_TERMINAL, "  Destination : %s\n",address_string);

  for (i = 0; i < pkt->num_digipeaters; i++)
  {
    memset(address_string, 0, (ADDRESS_LENGTH + 2) * sizeof(char));
    n = get_packet_digipeater(pkt, 
                          i, 
                          address_string, 
                          address_string + ADDRESS_LENGTH);
    if (n < 0)
    {
      chprintf(SD_TERMINAL, \
        "  Error in retrieving digipeater data, exitting display_packet....\n");
      return;
    }
    address_string[ADDRESS_LENGTH - 1] = '-';
    address_string[ADDRESS_LENGTH] += '0';
    chprintf(SD_TERMINAL, 
      "  Digipeater %d : %s (Last Digipeater : %s)\n", 
      i, 
      address_string,
      (n == 1) ? "YES" : "NO");
  }

  chprintf(SD_TERMINAL, "  Control Field : 0x%x\n",
            pkt->control_field & 0xff);
  chprintf(SD_TERMINAL, "  PID : 0x%x\n", 
            pkt->pid & 0xff);
  chprintf(SD_TERMINAL, "  Payload : %s\n", 
            pkt->payload);
  chprintf(SD_TERMINAL, "  Payload Length : %d\n", 
            pkt->payload_length);
  chprintf(SD_TERMINAL, "-------------------------------\n");
}

/**
 *  void display_packet_queue(Queue* queue)
 *
 *  Display the fields of the packets in the queue.  The packets are not popped
 *  out of the queue.
 */
void display_packet_queue(Queue* queue)
{  
  int k;

  if( (queue->front) != -1 )
  {    
    if(queue->front == queue->rear)
      display_packet(&(queue->packet_queue[queue->front]));
    else
    {
      for (k = queue->front; 
           k != queue->rear; 
           k = ((k + 1) % MAX_QUEUE_LENGTH))
      {
        display_packet(&(queue->packet_queue[k]));
      }
      display_packet(&(queue->packet_queue[k]));
    }
  }
  else 
    chprintf(SD_TERMINAL, "Packet queue empty...\n");
}

/**
 *  int frame_packet_bytes(Packet* pkt, char* bytes)
 *
 *  Frame the bytes in packet pkt into the character array - bytes and return
 *  the total number of bytes framed into the bytes array.
 *
 *  Please note that bytes must have allocated atleast MAX_PACKET_BYTES.
 *
 *  Return -1 if there is an error, otherwise the number of bytes populated.
 *
 */

int frame_packet_bytes(Packet* pkt, char* bytes)
{
  int retval = 0;
  int n = 0;
  unsigned short crc = 0x0000;

  if (pkt == NULL) return -1;
  memset(bytes, 0, MAX_PACKET_BYTES * sizeof(char));

  /* Copy the destination field */
  memcpy(bytes, pkt->destination_address, ADDRESS_LENGTH * sizeof(char));

  /* Initialize return value */
  retval = ADDRESS_LENGTH;

  /* Copy the source field */
  memcpy(bytes + retval, pkt->source_address, ADDRESS_LENGTH * sizeof(char));

  /* Increment retval */
  retval += ADDRESS_LENGTH;

  /* If there are no digipeaters, then address extension bit should be set */
  bytes[retval - 1] |= (pkt->num_digipeaters == 0) ? 0x01 : 0x00;

  /* Populate digipeater fields */
  for (n = 0; n < pkt->num_digipeaters; n++)
  {
    memcpy(bytes + retval , pkt->digipeaters[n], ADDRESS_LENGTH * sizeof(char));
    retval += ADDRESS_LENGTH;
    bytes[retval - 1] |= (n == (pkt->num_digipeaters - 1)) ? 0x01 : 0x00;
  }

  /* Copy the packet data payload */
  if (pkt->payload_length > 0)
  {
    memcpy(bytes + retval, pkt->payload, pkt->payload_length * sizeof(char));
    retval += pkt->payload_length;
  }
  
  /* Compute Cyclic Redundancy Check (CRC) */
  crc = calculate_packet_crc(bytes, retval);

  /* Embed the CRC low byte first */
  bytes[retval++] = (crc & 0xff);
  bytes[retval++] = ((crc >> 8) & 0xff);

  return retval;
}

