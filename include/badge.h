#ifndef BADGE_H
#define BADGE_H
extern const char hextab[];
void echoUSB(char *str);
void decDump(unsigned int value, char *out) ;
void hexDump(unsigned int value, char *out) ;
#endif
