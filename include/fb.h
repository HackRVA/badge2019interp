#ifndef fb_h
#define fb_h

/* 
   Frame buffer
*/
enum {
    FB_UNUSED=0,		/* unused */
    FB_CLEAR,			/* reset LCD */
    FB_BACKGROUND_COLOR,	/* 16 bit background fill color */
    FB_COLOR,			/* 16 bit color */
    FB_TRANSPARENT_MASK,	/* 16 bit transparent/key/mask color */
    FB_MOVE, 			/* (x 8bit, y 8bit) where the next stuff goes */
};

struct vector8_t {
   unsigned char x;
   unsigned char y;
};

struct framebuffer_t {
   unsigned short *buffer;
   struct vector8_t pos;
   unsigned char font;
   unsigned char fontHeight;

   unsigned short color;
   unsigned short BGcolor;
   unsigned short transMask;
   unsigned short transIndex;
   unsigned short changed;
};

extern struct framebuffer_t G_Fb;

void FbInit() ;
int FbFrameDone();
void FbMove(unsigned char x, unsigned char y);
void FbMoveRelative(char x, char y);
void FbMoveX(unsigned char x);
void FbMoveY(unsigned char y);
void FbClear();
void FbColor(unsigned short color);
void FbBackgroundColor(unsigned short color);
// void FbPicture(unsigned char assetId, unsigned char seqNum);
void FbTransparency(unsigned short transparencyMask);
void FbSprite(unsigned char picId, unsigned char imageNo);
void FbCharacter(unsigned char charin);
void FbFilledRectangle(unsigned char width, unsigned char height);
void FbPoint(unsigned char x, unsigned char y);
void FbPrintChar(unsigned char charin, unsigned char x, unsigned char y);
void FbHorizontalLine(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
void FbVerticalLine(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2);
void FbLine(unsigned char x0, unsigned char y0, unsigned char x1, unsigned char y1);
void FbWriteLine(unsigned char *string);
void FbWriteString(unsigned char *string, unsigned char length);
void FbRectangle(unsigned char width, unsigned char height);

void FbImage(unsigned char assetId, unsigned char seqNum);
void FbImage8bit(unsigned char assetId, unsigned char seqNum);
void FbImage4bit(unsigned char assetId, unsigned char seqNum);
void FbImage2bit(unsigned char assetId, unsigned char seqNum);
void FbImage1bit(unsigned char assetId, unsigned char seqNum);

#endif
