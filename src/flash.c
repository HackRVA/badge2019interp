#include <stdio.h>
#include "flash.h"

const unsigned short flashedBadgeId = 0xefbe; /* MAGIC value for script. It will be replaced by awk script in final flashing */

/*

PEB 2015/05/29

simple usage case/POC:

cc -g -O0 -o flash flash.c -DMAIN
./flash

*/

#ifdef MAIN
unsigned char G_flashstart[2048] = {0xFF, 0xFF};
void NVMWriteWord(unsigned int *addr, unsigned int word) ;
void NVMErasePage(unsigned int *addr);
#else
#include "plib.h"
const unsigned char G_flashstart[2048] = {0x00, 0x00};
#endif
/*
    not really a NV routine- used mostly for POC/MAIN testing 
*/
unsigned int NVMReadWord(unsigned int *addr) ;

/*
    flashstart rounded to a 1024 boundary
*/
unsigned int *G_flashAddr = 0x0; 

// 
// IMPORTANT NOTE FOR SELF WRITE FLASH
// 
// The flash can handle 10,000 writes and then it is worn out
// do not put an Erase OR WriteWord in any kind of loop
// 
void flashInit()
{
   // align addr on a 1k boundary within the 2k block we allocated
   // erase page first, don't have to erase if writing an area already erased
   /* seems there should be a way to statically define this */
   G_flashAddr = (unsigned int *)(((unsigned long)(&G_flashstart)+1024) & 0b11111111111111111111110000000000); // 1k flash boundary

#ifdef MAIN
   {
        unsigned int *fAddr;
	int i;

	fAddr = G_flashAddr;
	for (i=0; i< FLASHSIZE/4; i++) *fAddr++ = 0xFF;
   }
#endif
}

void flashErasePage()
{
   if (!G_flashAddr) flashInit();

   NVMErasePage(G_flashAddr);
}

/*
   write datasize bytes with appId and dataId identifiers
   appId = unique < 255 Id => badge menu position
   dataId = unique app assigned id so it can access data for read/write
*/
unsigned char NVwrite(unsigned char appId, unsigned char dataId, unsigned char *data, unsigned char dataSize)
{
   unsigned short i;
   unsigned char *dAddr;
   unsigned int *fAddr;
   union flashWord_u hdr;
   union flashWord_u fw;
   unsigned char dataWordSize; /* datasize rounded up to closest 4 bytes */

   if (!G_flashAddr) flashInit();

   /*
	find last data in block so we can 
	write past it.
   */

   fAddr = G_flashAddr;
   while (1) {
	hdr.l = *fAddr;
	if (hdr.h.appId == 255) break; /* unwritten flash */
	fAddr++; /* incr the read that already happen */
        dataWordSize = (hdr.h.dataSize+3) & 0xFFFFFFFC; /* round datablock up to int/4bytes */

#ifdef MAIN
	printf("skipover appId %d\n", hdr.h.appId);
	printf("skipover dataId %d\n", hdr.h.dataId);
	printf("skipover dataSize %d\n", hdr.h.dataSize);
	printf("skipover dataWordSize %d\n", dataWordSize);
#endif

	fAddr += dataWordSize; /* skip over data block */

	/* ran out of space? */
	if (fAddr > (G_flashAddr+FLASHSIZE)) return 0;
   }

   hdr.h.appId = appId;
   hdr.h.dataId = dataId;
   hdr.h.dataSize = dataSize;
   hdr.h.pad = 0; /* pad: write has to be int aligned */
   NVMWriteWord(fAddr++, hdr.l);

#ifdef MAIN
	printf("write appId %d\n", hdr.h.appId);
	printf("write dataId %d\n", hdr.h.dataId);
	printf("write dataSize %d\n", hdr.h.dataSize);
#endif

   dAddr = data;
   for (i=0; i<hdr.h.dataSize; i+=4) {
	fw.b[0] = *dAddr++;
	fw.b[1] = *dAddr++;
	fw.b[2] = *dAddr++;
	fw.b[3] = *dAddr++;
	NVMWriteWord(fAddr++, fw.l);
   }
   return dataSize;
}

/*
   read at most datasize bytes the have appId and dataId identifiers
   appId = unique < 255 Id => badge menu position
   dataId = unique app assigned id so it can access data for read/write
*/
unsigned char NVread(unsigned char appId, unsigned char dataId, unsigned char *data, unsigned char dataSize)
{
   unsigned short i;
   unsigned int *fAddr;
   union flashWord_u hdr;
   union flashWord_u fw;
   unsigned char dataWordSize; /* datasize rounded up to closest 4 bytes */

   if (!G_flashAddr) flashInit();

   fAddr = G_flashAddr;
   while (1) {
	hdr.l = NVMReadWord(fAddr);
	fAddr++;

	if (hdr.h.appId == 255) break; /* unwritten flash, done */

#ifdef MAIN
	printf("appId %d\n", hdr.h.appId);
	printf("dataId %d\n", hdr.h.dataId);
	printf("dataSize %d\n", hdr.h.dataSize);
#endif

	if (hdr.h.appId == appId) {
	    if (hdr.h.dataId == dataId) {
		unsigned char calcDataSize;

		calcDataSize = (hdr.h.dataSize < dataSize) ? hdr.h.dataSize : dataSize;
		/* first do full 4 byte reads */
		//for (i=0; i<(hdr.h.dataSize/4); i++) {   -> this would not use user specfied data size it would use stored datasize
		for (i=0; i<(calcDataSize/4); i++) {
		    fw.l = NVMReadWord(fAddr++);
		    *data++ = fw.b[0];
		    *data++ = fw.b[1];
		    *data++ = fw.b[2];
		    *data++ = fw.b[3];
		}
		/* last partial read -> 0,1,2 bytes */
		//if (hdr.h.dataSize & 0x3) {              -> this would not use user specfied data size it would use stored datasize
		if (calcDataSize & 0x3) {
		    unsigned char b;

		    fw.l = NVMReadWord(fAddr++); /* partial read */
		    //for (b=0; b<(hdr.h.dataSize & 0x3); b++)
		    for (b=0; b<(calcDataSize & 0x3); b++)
		        *data++ = fw.b[b];
		}
		return calcDataSize;
		//return hdr.h.dataSize;
	    }
	}
        dataWordSize = (hdr.h.dataSize+3) & 0xFFFFFFFC; /* round datablock up to int/4bytes */
#ifdef MAIN
	printf("skip %d\n", dataWordSize);
#endif
	fAddr += dataWordSize;

	if (fAddr > (G_flashAddr+FLASHSIZE)) break;
   }
   return 0;
}

#define SYSID 0
#define DATAID 0
unsigned char sysDataRead(struct sysData_t *sdata)
{
    return NVread(SYSID, DATAID, (unsigned char *)sdata, sizeof(struct sysData_t));
}

unsigned char sysDataWrite(struct sysData_t *sdata)
{
    return NVwrite(SYSID, DATAID, (unsigned char *)sdata, sizeof(struct sysData_t));
}


#ifndef MAIN

unsigned int NVMReadWord(unsigned int *addr) 
{
    return *addr;
}

#else
#include <stdio.h>
#include <string.h>

char buffer[128];
FILE *flashRead_fd=0;
FILE *flashWrite_fd=0;

void dumpSys(struct sysData_t *sdata)
{
    printf("name %s \n", sdata->name);
    printf("sekrits %s \n", sdata->sekrits);
    printf("ach %s \n", sdata->achievements);
}

void NVMErasePage(unsigned int *addr)
{
    fprintf(flashWrite_fd, "erase addr %u\n", addr);
}

void NVMWriteWord(unsigned int *addr, unsigned int word) 
{
    fprintf(flashWrite_fd, "word addr %u, val %x\n", addr, word);
//    sprintf(buffer, "word addr %u, val %x\n", addr, word);

    *addr = word;

    //printf("WRITE word addr %u, val %x\n", addr, word);
}

unsigned int NVMReadWord(unsigned int *addr) 
{
    fprintf(flashRead_fd, "read addr %u, val %x\n", addr, *addr);

    return *addr;

//    sscanf(buffer, "word addr %u, val %x\n", addr, *addr);

    //printf("READ word addr %u, word %x val %x\n", addr, word, (unsigned int)*word);
}

void main(int argc, char **argv, char **envp)
{
    struct sysData_t sdata;
    unsigned char data[16];

    flashRead_fd = fopen("flashRead.txt", "w");
    flashWrite_fd = fopen("flashWrite.txt", "w");

    strcpy(sdata.name, "badge test 1");
    strcpy(sdata.sekrits, "0123456");
    strcpy(sdata.achievements, "0123456");

    sysDataWrite(&sdata);

    dumpSys(&sdata);

    sysDataRead(&sdata);

    strcpy(data, "D1TA1234X");
    printf("	NVwrite %d\n", NVwrite(1, 0, data, 8));

    strcpy(data, "D2TA123XX");
    printf("	NVwrite %d\n", NVwrite(1, 1, data, 7));

    strcpy(data, "D3TA1234X");
    printf("	NVwrite %d\n", NVwrite(1, 2, data, 8));

    strcpy(data, "4XXXXXXXX");
    printf("	NVwrite %d\n", NVwrite(1, 3, data, 1));

    strcpy(data, "D5TA1234X");
    printf("	NVwrite %d\n", NVwrite(2, 2, data, 8));

    bzero(data,16);
    printf("	NVread appid %d id %d res %d data %s\n", 1, 0, NVread(1, 0, data, 8), data);

    bzero(data,16);
    printf("	NVread appid %d id %d res %d data %s\n", 1, 1, NVread(1, 1, data, 8), data);

    bzero(data,16);
    printf("	NVread appid %d id %d res %d data %s\n", 1, 2, NVread(1, 2, data, 8), data);

    bzero(data,16);
    printf("	NVread appid %d id %d res %d data %s\n", 1, 3, NVread(1, 3, data, 8), data);

    bzero(data,16);
    printf("	NVread appid %d id %d res %d data %s\n", 2, 2, NVread(2, 2, data, 8), data);

    fclose(flashRead_fd);
    fclose(flashWrite_fd);
}
#endif
