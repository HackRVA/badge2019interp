#ifndef flash_h
#define flash_h

#ifdef MAIN
extern unsigned char G_flashstart[];
#else
extern const unsigned char G_flashstart[];
#endif

/*
   one per badge
*/
struct sysData_t {
   char name[32];
   unsigned short badgeId; /* 2 bytes == our badge Id */
   char sekrits[8];
   char achievements[8];

   /*
      prefs
   */
   char ledBrightness;  /* 1 byte */
   char backlight;      /* 1 byte */
};

extern struct sysData_t G_sysData;
extern const unsigned short flashedBadgeId; /* overrides what is stored in sysData*/
extern unsigned short G_peerBadgeId; /* who we are talking to */
extern unsigned int *G_flashAddr; /* starter point w/in G_flashStart array */

#define FLASHSIZE 1024

void flashErasePage();
unsigned char sysDataRead(struct sysData_t *fdata);
unsigned char sysDataWrite(struct sysData_t *fdata);

unsigned char NVwrite(unsigned char appId, unsigned char dataId, unsigned char *data, unsigned char datasize);
unsigned char NVread(unsigned char appId, unsigned char dataId, unsigned char *data, unsigned char datasize);

/*
    format of a record in the flash block:
*/
struct flashHeader_t {
    unsigned char appId;
    unsigned char dataId;
    unsigned char dataSize;
    unsigned char pad; /* to fit into 32 bits */
};

/*
   union for easier access
*/
union flashWord_u {
    struct flashHeader_t h;
    char b[4];		/* byte access */
    unsigned int l;	/* int word access */
};


#endif

