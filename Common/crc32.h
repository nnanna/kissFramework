/*
**  CRC.H - header file for SNIPPETS CRC and checksum functions
	Copyright (C) 1986 Gary S. Brown.  You may use this program, or
   code or tables extracted from it, as desired without restriction.*/

#ifndef CRC__H
#define CRC__H

//#include <stdlib.h>           /* For size_t                           */
#include <stdint.h>           /* For uint8_t, uint16_t, uint32_t      */

/*
**  File: ARCCRC16.C
*/

void init_crc_table();
uint16_t crc_calc(uint16_t crc, char *buf, unsigned nbytes);
void do_file(char *fn);

/*
**  File: CRC-16.C
*/

uint16_t crc16(char *data_p, uint16_t length);

/*
**  File: CRC-16F.C
*/

uint16_t updcrc(uint16_t icrc, uint8_t *icp, size_t icnt);

/*
**  File: CRC_32.C
*/

uint32_t updateCRC32(unsigned char ch, uint32_t crc);

//bool crc32file(char *name, uint32_t *crc, long *charcnt);

uint32_t crc32buf(const char *buf);

/*
**  File: CHECKSUM.C
*/

unsigned checksum(void *buffer, size_t len, unsigned int seed);

/*
**  File: CHECKEXE.C
*/

void checkexe(char *fname);


#define CRC32(x)	crc32buf(x)


#endif /* CRC__H */