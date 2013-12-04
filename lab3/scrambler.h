#ifndef __SCRAMBLER_H__
#define __SCRAMBLER_H__

void scrambler_scram(unsigned char * message, unsigned char * buffer, int bufsize);
void scrambler_descram(unsigned char * message, unsigned char * buffer, int bufsize);

#endif //__SCRAMBLER_H__
