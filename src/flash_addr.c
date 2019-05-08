#ifdef BADGE_FLASH_SECTION
/*
   leave 256 bytes at the end of 16k/0x4000 block
*/
__attribute__ ((section (".badge"))) 
const unsigned char G_flashstart[0x4000]  = {0x0, 0x0};

#else

#endif
