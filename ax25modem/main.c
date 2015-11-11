#include <string.h>

#include "ch.h"
#include "hal.h"
#include "chtypes.h"
#include "chstreams.h"
#include "chprintf.h"
#include "packet_queue.h"

int main(void)
{

  static Queue my_queue;
  Packet pkt;
  char packet_payload[30];
  char digipeater_name[ADDRESS_LENGTH];
  int i;
  int j;
  int rv;

  /* HAL and RTOS Init Calls */
  halInit();
  chSysInit();
  
  /* Turn on UART6 */
  sdStart(&SD_TERMINAL_PORT , NULL );
  palSetPadMode(GPIOC, 6, PAL_MODE_ALTERNATE(8));     // UART TX
  palSetPadMode(GPIOC, 7, PAL_MODE_ALTERNATE(8));     // UART RX

  /* Initialize the Packet Queue */
  init_queue(&my_queue);

  for (i = 0; i <= MAX_QUEUE_LENGTH; i++)
  {
    init_packet(&pkt);
    set_packet_source(&pkt, "VU3EM", 1);  
    set_packet_destination(&pkt, "CQ", 0); 
    for (j = 0; j <= MAX_DIGIPEATERS; j++)
    {
      memset(digipeater_name, 0, ADDRESS_LENGTH * sizeof(char));
      strncpy(digipeater_name, "DIGI", 4 * sizeof(char));
      digipeater_name[4] = j + '1';
      rv = add_packet_digipeater(&pkt, digipeater_name, j);
      chprintf(SD_TERMINAL, "add_packet_digipeater returned %d for digi %d\n",
        rv, j);
    }
    memset(packet_payload, 0, 30 * sizeof(char));
    strncpy(packet_payload, "THIS IS PACKET ", 15 * sizeof(char));
    packet_payload[15] = i + '0';
    set_packet_payload(&pkt, packet_payload);

    chprintf(SD_TERMINAL, "Packet %d\n", i);
    display_packet(&pkt);

    rv = push_packet_to_queue(&my_queue, &pkt);
    chprintf(SD_TERMINAL, "push_packet_to_queue returned %d for pkt %d\n", 
      rv, i);
  }

  chprintf(SD_TERMINAL, "Displaying Packet Queue...\n");
  display_packet_queue(&my_queue);

  init_packet(&pkt);
  while(pop_packet_from_queue(&my_queue, &pkt) == 0)
  {
    chprintf(SD_TERMINAL, "Popped Packet...\n");
    display_packet(&pkt);
    init_packet(&pkt);
  }

  chprintf(SD_TERMINAL, "Pop packet queue complete....\n");

  chprintf(SD_TERMINAL, "Displaying Queue after push...\n");
  display_packet_queue(&my_queue);

  chprintf(SD_TERMINAL, "Queue Status: %d, %d\n", 
    my_queue.front,
    my_queue.rear);

  while(1)
  {
    chprintf(SD_TERMINAL, "Waiting....\n");
    chThdSleepMilliseconds(1000);
    palTogglePad(GPIOD, GPIOD_LED3);       /* Orange.  */
  }

  return 0;
}

