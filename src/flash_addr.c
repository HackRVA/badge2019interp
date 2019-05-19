/*
   16k block for flash writes
*/
#ifdef MAIN
unsigned int G_flashstart[0x1000]  = {0x0, 0x0};
#else
__attribute__ ((section (".badge"))) 
const unsigned int G_flashstart[0x1000]  = {0x0, 0x0};
#endif
