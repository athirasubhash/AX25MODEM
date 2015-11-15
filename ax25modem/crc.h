#ifndef CRC_H
#define CRC_H

extern const unsigned short crc_table[];

unsigned short calculate_packet_crc(char*, int);

#endif
