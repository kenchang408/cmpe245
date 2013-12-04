/*
 * Author: Mujtaba Hassanpur
 * CMPE 245 - Embedded Wireless Systems
 * Department of Computer Engineering
 * San Jose State University
 * San Jose, CA
 *
 * Fall 2013
 */

#ifndef __HAMMING_H__
#define __HAMMING_H__

// Generator Matrix
const int mG[7][4] = {
    {1, 1, 0, 1},
    {1, 0, 1, 1},
    {1, 0, 0, 0},
    {0, 1, 1, 1},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1}
};

// Parity-check Matrix
const int mH[3][7] = {
    {1, 0, 1, 0, 1, 0, 1},
    {0, 1, 1, 0, 0, 1, 1},
    {0, 0, 0, 1, 1, 1, 1}
};

// Receiver Matrix
const int mR[4][7] = {
    {0, 0, 1, 0, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0},
    {0, 0, 0, 0, 0, 1, 0},
    {0, 0, 0, 0, 0, 0, 1},
};

typedef struct
{
    int value;
    int bits[4];
} hamming_value4_t;

typedef struct
{
    int value;
    int bits[7];
} hamming_value7_t;

typedef struct
{
    unsigned char * data;
    unsigned int totalsz;
    unsigned int valsz;
    unsigned int byteoff;
    unsigned int bitoff;
} hamming_value_data_t;


void hamming_decode(unsigned char * input, unsigned char * output, size_t bufsize);
void hamming_encode(unsigned char * input, unsigned char * output, size_t bufsize);
hamming_value4_t * hamming_value4_data_get(hamming_value_data_t * buffer);
void			   hamming_value4_data_put(hamming_value_data_t * buffer, hamming_value4_t * value);
hamming_value7_t * hamming_value7_data_get(hamming_value_data_t * buffer);
void			   hamming_value7_data_put(hamming_value_data_t * buffer, hamming_value7_t * value);
void hamming_channel_encode(hamming_value4_t * value, hamming_value7_t * codeword);
void hamming_channel_decode(hamming_value7_t *received, hamming_value4_t * value);
int  hamming_parity_check(hamming_value7_t * received);

#endif //__HAMMING_H__

