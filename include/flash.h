#ifndef flash_h
#define flash_h

extern const unsigned int G_flashstart[];

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
extern unsigned short G_peerBadgeId; /* who we are talking to */
extern unsigned int *G_flashAddr; /* starter point w/in G_flashStart array */
#define FLASHSIZE 1024*4096

void flashErasePage();
unsigned char NVwrite(unsigned char appId, unsigned char dataId, unsigned char *data, unsigned char datasize);
unsigned char NVread(unsigned char appId, unsigned char dataId, unsigned char *data, unsigned char datasize);

#endif

