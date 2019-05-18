#include <stdio.h>

/*

PEB 2018/05/18 
  reworked to key/value pair

PEB 2015/05/29
  original

*/

#ifndef MAIN
#include "plib.h"
#include "flash.h"
#else
#include <stdio.h>
#include "flash.h"
#endif

#define BLOCKSIG (unsigned int)0xFACEBEEF

#ifdef MAIN
/*
cc -m32 -DMAIN -O0 -g -I../include -o flash flash.c flash_addr.c
*/

void NVMErasePage(void *addr)
{
    int i;
    int *iaddr;

    iaddr = (int *)addr;
    for (i=0; i<1024; i++)
	iaddr[i] = 0xFFFFFFFF;
}

int NVMWriteWord(unsigned int *addr, unsigned int val)
{
    printf("write addr %X val %X\n", addr, val);
    *addr = val;
}

struct sysData_t G_sysData = { { 0 }, 42, { 0 }, { 0 } };
struct sysData_t G_sysDataRom;
int main(int argc, char** argv)
{
   char test[] = "tests";
   char test2[] = "test2";
   char derp[] = "derp";
   char sysData[] = "the rain in spain stays mainly";
   char sysOut[] =  "                              ";
   char value2[8];
   int i;

   flashWriteKeyValue(&G_sysData, &G_sysData, sizeof(struct sysData_t));
   flashReadKeyValue(&G_sysData, &G_sysDataRom, sizeof(struct sysData_t));
   printf("sysData %d\n", G_sysDataRom.badgeId);
/*
   flashWriteKeyValue(12345, test, 5);
   flashWriteKeyValue(9999, derp, 4);
   flashWriteKeyValue(12345, test2, 5);


   flashReadKeyValue(12345, value2, 5);
   value2[5] = 0;
   printf("value2 %s\n", value2);

   flashReadKeyValue(9999, value2, 4);
   value2[4] = 0;
   printf("value2 %s\n", value2);

   flashWriteKeyValue(sysData, sysData, sizeof(sysData));
   flashReadKeyValue(sysData, sysOut, sizeof(sysOut));
   printf("sysData %s\n", sysOut);

   for (i=0; i<sizeof(sysOut); i++) sysOut[i]=0;

   flashReadKeyValue(sysData, sysOut, 16);
   printf("sysData %s\n", sysOut);
*/
}
#endif

// 
// IMPORTANT NOTE FOR SELF WRITE FLASH
// 
// The flash can handle 10,000 writes and then it is worn out
// do not put an Erase OR WriteWord in any kind of loop
// 

void flashEraseBlock()
{
    int i;
    unsigned int *faddr;

    for (i=0; i<16; i++)
       NVMErasePage((void *)G_flashstart+i*1024); // pic32mx2XX has 1024 bute pages


    faddr = (unsigned int *)G_flashstart;
    NVMWriteWord((void *)faddr, BLOCKSIG); /* first key */
    faddr++;
    NVMWriteWord((void *)faddr, 0); /* len */
}

/*
     20190518 flash block organization:

     unsigned int key1
     unsigned int len1
     data of len1 rounded up to sizeof(unsigned int)
     unsigned int key2
     unsigned int len2
     data of len2 rounded up to sizeof(unsigned int)
*/
unsigned int *flashFindKey(unsigned int *faddr, unsigned int findkey)
{
    unsigned int *found=0;
    unsigned int key, len;

    //printf("flashFindKey faddr %X findkey %X\n", faddr, findkey);
    key = *faddr++;
    len = *faddr++;
    faddr += ((len+3) & 0XFFFFFFFC); // round up by 4's

    while (1) {
	//printf("flashFindKey faddr %X key %X len %x\n", faddr, key, len);

        found = faddr;
	key = *faddr++;
	len = *faddr++;

	//printf("flashFindKey AFTER faddr %X key %X len %x\n", faddr, key, len);
	if (len == 0xFFFFFFFF) { /* end of block */
	    found = 0;
	    break; /* not valid, at end */
	}
	if (key == findkey) break; 
	faddr += ((len+3) & 0XFFFFFFFC); // round up by 4's
    }
/*
    if (found == 0)
	printf("flashFindKey not faddr %X findkey %X\n", faddr, findkey);
    else
	printf("flashFindKey found key %X len %X\n", found[0], found[1]);
*/

    return found;
}

int flashWriteKeyValue( unsigned int valuekey, char *value, unsigned int valuelen)
{
    unsigned int *faddr, *end=0;
    unsigned int v=0;
    unsigned int d=0;
    unsigned int key, flen=0;

    faddr = (unsigned int *)G_flashstart;
    key = *faddr++;
    flen = *faddr++;

    //printf("faddr %X key %X len %X\n", faddr, key, flen);
    if (key != BLOCKSIG) flashEraseBlock();

    faddr = (unsigned int *)G_flashstart;
    key = *faddr++;
    flen = *faddr++;
    //printf("writeKey: faddr %X fkey %X flen %X\n", faddr, key, flen);

    /* 
       find end of block
    */
    while (1) {
        end = faddr;
	key = *faddr++;
	flen = *faddr++;
	if (flen == 0xFFFFFFFF) break; /* not valid, at end */
	faddr += ((flen+3) & 0XFFFFFFFC); // round up by 4's
    }
    if (faddr == 0) return 0; // wtf

    NVMWriteWord((void *)end++, valuekey);
    NVMWriteWord((void *)end++, valuelen);

    v=0;
    while (v < valuelen) {
	d = 0;
	d = value[v] << 24;
	if ((v+1) < valuelen) d |= (value[v+1] << 16);
	if ((v+2) < valuelen) d |= (value[v+2] <<  8);
	if ((v+3) < valuelen) d |= (value[v+3]      );

	NVMWriteWord((void *)end++, d);
	v += 4;
    }
    return 1;
}

int flashReadKeyValue(unsigned int valuekey, char *value, unsigned int valuelen)
{
    unsigned int *found, *next;
    unsigned int key, flen, v, d;

    //printf("ReadKey valuekey %X\n", valuekey);
    if (*G_flashstart != BLOCKSIG) {
//	avoid having this run when the badge first boots and reads user info
//	flashEraseBlock();
//	printf("not erased\n");
	return 0;
    }

    next = found = flashFindKey((unsigned int *)G_flashstart, valuekey);
    //printf("found %X valuekey %X\n", found, valuekey);
    while (next) {
	next = flashFindKey(found, valuekey);
	if (next != 0) found = next;
    }
    if (found == 0) return 0;

    key = *found++;
    flen = *found++;
    //printf("found %X %X len %X\n", found, key, flen);

    // check for short buffer
    flen = (flen > valuelen) ? valuelen : flen;
    v=0;
    while (v < flen) {
	d = *found++;
	value[v] = (d >> 24) & 0xFF;
	if ((v+1) < flen) value[v+1] = (d >> 16) & 0xFF;
	if ((v+2) < flen) value[v+2] = (d >>  8) & 0xFF;
	if ((v+3) < flen) value[v+3] = d & 0xFF;

	v += 4;
    }
    return (found != 0);
}
