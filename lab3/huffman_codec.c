#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "huffman.h"
#include "huffman_codec.h"

#define BUFFER_SIZE 32

extern huffman_vlc_t CODEBOOK[];

#if 0 // TEST CODE
const char * message = "MUJTABA_SJSU_CMPE_FALL2013!!!!!";
int main(int argc, char ** argv)
{
    int i;
    unsigned char buffer[BUFFER_SIZE];
    unsigned char decoded[BUFFER_SIZE];
    huffman_encode(message, buffer, BUFFER_SIZE);
    printf("\n\n");
    for(i = 0; i < BUFFER_SIZE; i++)
    {
        printf("%02x ", buffer[i]);
    }
    printf("\n\n");

    printf("Decoding...\n");
    huffman_decode(buffer, decoded, BUFFER_SIZE);
    return 0;
}
#endif

void huffman_encode(const unsigned char * message, unsigned char * buffer, int bufsize)
{
    int i, j;
    int byteoff, bitoff;
    int n = strlen(message);
    bzero(buffer, bufsize);
    byteoff = 0;
    bitoff = 0;
    for(i = 0; i < n; i++)
    {
        unsigned char value = message[i];
        printf("%c  ", value);
        int len = CODEBOOK[value].length;
        int code = CODEBOOK[value].code;
        for(j = 0; j < len; j++)
        {
            int bit = (code >> j) & 0x01;
            printf("%d", bit);
            buffer[byteoff] |= (bit << (7-bitoff));
            if(bitoff == 7)
            {
                byteoff = (byteoff+1) % bufsize;
            }
            bitoff = (bitoff+1) % 8;
        }
        printf("\n");
    }   
}

void huffman_decode(const unsigned char * message, unsigned char * buffer, int bufsize)
{
    int i, j;
    int byteoff, bitoff;
    int bufindex;
    unsigned int value;
    unsigned int length;
    bzero(buffer, bufsize);

    byteoff = 0;
    bitoff = 0;
    value = 0;
    length = 0;
    bufindex = 0;
    while(byteoff < bufsize && bufindex < bufsize)
    {
        value |= (message[byteoff] >> (7-bitoff) & 0x01) << length;
        length++;
        // look for a match
        for(j = 0; j < 256; j++)
        {
            if(CODEBOOK[j].code == value && CODEBOOK[j].length == length)
            {
                // found it!
                printf("%c <-- code=%02x, len=%d\n", (unsigned char) j, value, length);
                buffer[bufindex++] = (unsigned char) j;
                value = 0;
                length = 0;
                break;
            }
        }

        if(bitoff == 7)
        {
            byteoff++;
        }
        bitoff = (bitoff+1) % 8;
    }
}

