/**
 * Author: Mujtaba Hassanpur
 * CMPE 245 - Embedded Wireless Systems
 * Department of Computer Engineering
 * San Jose State University
 * San Jose, CA
 *
 * Fall 2013
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hamming.h"

#if 0
int main(int argc, char ** argv)
{
    int i;
    int parity;
    hamming_value4_t value;
    hamming_value7_t result;
    unsigned char stream[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef, 0x11, 0x33, 0x55, 0x77, 0x99, 0xbb, 0xdd, 0xff};

    printf("Channel Encode --\n");
    value.value = 0x3;
    value.bits[0] = 0;
    value.bits[1] = 0;
    value.bits[2] = 1;
    value.bits[3] = 1;
    hamming_channel_encode(&value, &result);

    printf("Value: %d %d %d %d\n", value.bits[0], value.bits[1], value.bits[2], value.bits[3]);
    printf("Result: ");
    for(i = 0; i < 7; i++)
    {
        printf("%d ", result.bits[i]);
    }
    printf("\n\n");

    printf("Parity Check --\n");
    result.bits[4] = !result.bits[4];
    printf("Received: ");
    for(i = 0; i < 7; i++)
    {
        printf("%d ", result.bits[i]);
    }
    printf("\n");
    parity = hamming_parity_check(&result);
    printf("Parity: %d\n", parity);
    if(parity != 0)
    {
        result.bits[parity-1] = !result.bits[parity-1];
        printf("Corrected: ");
        for(i = 0; i < 7; i++)
        {
            printf("%d ", result.bits[i]);
        }
        printf("\n");
    }
    printf("\n");

    printf("Channel Decode --\n");
    bzero(&value, sizeof(hamming_value4_t));
    hamming_channel_decode(&result, &value);
    printf("Received: ");
    for(i = 0; i < 7; i++)
    {
        printf("%d ", result.bits[i]);
    }
    printf("\n");
    printf("Value: %d %d %d %d\n\n", value.bits[0], value.bits[1], value.bits[2], value.bits[3]);

    printf("Hamming Value4 Data --\n");
    hamming_value_data_t valueData;
    bzero(&valueData, sizeof(hamming_value_data_t));
    valueData.data = stream;
    valueData.totalsz = sizeof(stream);
 	
	hamming_value_data_t valueDataWrite;
	bzero(&valueDataWrite, sizeof(hamming_value_data_t));
	valueDataWrite.data = (unsigned char *) malloc(sizeof(stream));
	bzero(valueDataWrite.data, sizeof(stream));
	valueDataWrite.totalsz = sizeof(stream);
	
    hamming_value4_t * more = hamming_value4_data_get(&valueData);
    while(more)
    {
        printf("  %02x -> %d%d%d%d\n", more->value, more->bits[0], more->bits[1], more->bits[2], more->bits[3]);
        hamming_value4_data_put(&valueDataWrite, more);
		free(more);
        more = hamming_value4_data_get(&valueData);
    }
	printf("Write Value4 Data:\n  ");
	for(i = 0; i < valueDataWrite.totalsz; i++)
	{
		printf("%02x ", valueDataWrite.data[i]);
	}
	printf("\n\n");

    printf("Hamming Value7 Data --\n");
    bzero(&valueData, sizeof(hamming_value_data_t));
    valueData.data = stream;
    valueData.totalsz = sizeof(stream);
    
	bzero(&valueDataWrite, sizeof(hamming_value_data_t));
	valueDataWrite.data = (unsigned char *) malloc(sizeof(stream));
	bzero(valueDataWrite.data, sizeof(stream));
	valueDataWrite.totalsz = sizeof(stream);
	
	hamming_value7_t * more7 = hamming_value7_data_get(&valueData);
    while(more7)
    {
        printf("  %02x -> %d%d%d%d%d%d%d\n", more7->value, more7->bits[0], more7->bits[1], more7->bits[2], more7->bits[3], more7->bits[4], more7->bits[5], more7->bits[6]);
        hamming_value7_data_put(&valueDataWrite, more7);
		free(more7);
        more7 = hamming_value7_data_get(&valueData);
    }

	printf("Write Value7 Data:\n  ");
	for(i = 0; i < valueDataWrite.totalsz; i++)
	{
		printf("%02x ", valueDataWrite.data[i]);
	}
	printf("\n\n");

	printf("Hamming Encode --\n");
	unsigned char * buf = (unsigned char *) malloc(sizeof(stream));
	bzero(buf, sizeof(stream));
	hamming_encode(stream, buf, sizeof(stream));
	for(i = 0; i < sizeof(stream); i++)
	{
		printf(" %02x", buf[i]);
	}
	printf("\n\n");

	printf("Hamming Decode --\n");
	unsigned char * decbuf = (unsigned char *) malloc(sizeof(stream));
	bzero(decbuf, sizeof(stream));
	hamming_decode(buf, decbuf, sizeof(stream));
	for(i = 0; i < sizeof(stream); i++)
	{
		printf(" %02x", decbuf[i]);
	}
	printf("\n\n");

    return 0;
}
#endif

void hamming_encode(unsigned char * input, unsigned char * output, size_t bufsize)
{
	if(input == NULL || output == NULL)
	{
		printf("%s: Got NULL pointer!\n", __func__);
		exit(-100);
	}

	hamming_value4_t * value;
	hamming_value7_t encoded;
	bzero(&encoded, sizeof(hamming_value7_t));
	hamming_value_data_t indata;
	hamming_value_data_t outdata;
	bzero(&indata, sizeof(hamming_value_data_t));
	bzero(&outdata, sizeof(hamming_value_data_t));
	indata.data = input;
	indata.totalsz = bufsize;
	outdata.data = output;
	outdata.totalsz = bufsize;

	value = hamming_value4_data_get(&indata);
	while(value)
	{
		hamming_channel_encode(value, &encoded);
		printf(" - %d%d%d%d -> %d%d%d%d%d%d%d\n",
				value->bits[0], value->bits[1], value->bits[2], value->bits[3],
				encoded.bits[0], encoded.bits[1], encoded.bits[2], encoded.bits[3],
				encoded.bits[4], encoded.bits[5], encoded.bits[6]);
		hamming_value7_data_put(&outdata, &encoded);
		free(value);
		value = hamming_value4_data_get(&indata);
	}
}

void hamming_decode(unsigned char * input, unsigned char * output, size_t bufsize)
{
	if(input == NULL || output == NULL)
	{
		printf("%s: Got NULL pointer!\n", __func__);
		exit(-100);
	}

	hamming_value7_t * value;
	hamming_value4_t decoded;
	int parity;
	bzero(&decoded, sizeof(hamming_value7_t));
	hamming_value_data_t indata;
	hamming_value_data_t outdata;
	bzero(&indata, sizeof(hamming_value_data_t));
	bzero(&outdata, sizeof(hamming_value_data_t));
	indata.data = input;
	indata.totalsz = bufsize;
	outdata.data = output;
	outdata.totalsz = bufsize;

	value = hamming_value7_data_get(&indata);
	while(value)
	{
		parity = hamming_parity_check(value);
		if(parity != 0)
		{
			printf(" ----> Found parity error on bit %d!\n", parity);
			value->bits[parity-1] = !value->bits[parity-1];
		}
		hamming_channel_decode(value, &decoded);
		printf(" - %d%d%d%d%d%d%d -> %d%d%d%d\n",
				value->bits[0], value->bits[1], value->bits[2], value->bits[3],
				value->bits[4], value->bits[5], value->bits[6],
				decoded.bits[0], decoded.bits[1], decoded.bits[2], decoded.bits[3]);
		hamming_value4_data_put(&outdata, &decoded);
		free(value);
		value = hamming_value7_data_get(&indata);
	}
}

hamming_value4_t * hamming_value4_data_get(hamming_value_data_t * buffer)
{
    int i;
    int tbits;
    int rbits;
    hamming_value4_t * result = NULL;
    if(buffer != NULL)
    {
        tbits = buffer->totalsz * 8;
        rbits = tbits - ((buffer->byteoff * 8) + buffer->bitoff);
        if(rbits < 4)
        {
            // not enough bits
            return NULL;
        }

        result = (hamming_value4_t *) malloc(sizeof(hamming_value4_t));
        if(result == NULL)
        {
            printf("%s: Failed on malloc!\n", __func__);
            exit(-101);
        }

        bzero(result, sizeof(hamming_value4_t));
        for(i = 0; i < 4; i++)
        {
            result->bits[i] = (buffer->data[buffer->byteoff] >> (7-buffer->bitoff)) & 0x01;
            result->value |= (result->bits[i] << (3-i));
            buffer->bitoff = (buffer->bitoff + 1) % 8;
            if(buffer->bitoff == 0)
            {
                buffer->byteoff++;
            }
        }
    }

    return result;
}

void hamming_value4_data_put(hamming_value_data_t * buffer, hamming_value4_t * value)
{
	int i;
	int tbits;
	int rbits;
	if(buffer == NULL || value == NULL)
	{
		printf("%s: Got NULL pointer!\n", __func__);
		exit(-101);
	}

	tbits = buffer->totalsz * 8;
	rbits = tbits - ((buffer->byteoff * 8) + buffer->bitoff);
	if(rbits < 4)
	{
		//printf("%s: Not enough room for data in buffer!\n", __func__);
		//exit(-102);
		return;
	}

	for(i = 0; i < 4; i++)
	{
		buffer->data[buffer->byteoff] |= (value->bits[i] << (7-buffer->bitoff));
		buffer->bitoff = (buffer->bitoff + 1) % 8;
		if(buffer->bitoff == 0)
		{
			buffer->byteoff++;
		}
	}
}

hamming_value7_t * hamming_value7_data_get(hamming_value_data_t * buffer)
{
    int i;
    int tbits;
    int rbits;
    hamming_value7_t * result = NULL;
    if(buffer != NULL)
    {
        tbits = buffer->totalsz * 8;
        rbits = tbits - ((buffer->byteoff * 8) + buffer->bitoff);
        if(rbits < 7)
        {
            // not enough bits
            return NULL;
        }

        result = (hamming_value7_t *) malloc(sizeof(hamming_value7_t));
        if(result == NULL)
        {
            printf("%s: Failed on malloc!\n", __func__);
            exit(-101);
        }

        bzero(result, sizeof(hamming_value7_t));
        for(i = 0; i < 7; i++)
        {
            result->bits[i] = (buffer->data[buffer->byteoff] >> (7-buffer->bitoff)) & 0x01;
            result->value |= (result->bits[i] << (6-i));
            buffer->bitoff = (buffer->bitoff + 1) % 8;
            if(buffer->bitoff == 0)
            {
                buffer->byteoff++;
            }
        }
    }

    return result;
}

void hamming_value7_data_put(hamming_value_data_t * buffer, hamming_value7_t * value)
{
    int i;
	int tbits;
	int rbits;
	if(buffer == NULL || value == NULL)
	{
		printf("%s: Got NULL pointer!\n", __func__);
		exit(-101);
	}

	tbits = buffer->totalsz * 8;
	rbits = tbits - ((buffer->byteoff * 8) + buffer->bitoff);
	if(rbits < 7)
	{
		//printf("%s: Not enough room for data in buffer!\n", __func__);
		//exit(-102);
		return;
	}

	for(i = 0; i < 7; i++)
	{
		buffer->data[buffer->byteoff] |= (value->bits[i] << (7-buffer->bitoff));
		buffer->bitoff = (buffer->bitoff + 1) % 8;
		if(buffer->bitoff == 0)
		{
			buffer->byteoff++;
		}
	}
}

/**
 * hamming_channel_encode
 *
 * value - Value to be encoded.
 * codeword - Result of encoding.
 */
void hamming_channel_encode(hamming_value4_t * value, hamming_value7_t * codeword)
{
    int i, j;

    if(value == NULL || codeword == NULL)
    {
        printf("%s: Got NULL pointer!\n", __func__);
        exit(-100);
    }

    bzero(codeword, sizeof(hamming_value7_t));
	for(i = 0; i < 7; i++)
    {
        codeword->bits[i] = 0;
        for(j = 0; j < 4; j++)
        {
            codeword->bits[i] += (mG[i][j] * value->bits[j]);
        }
        codeword->bits[i] %= 2;
        codeword->value |= (codeword->bits[i] << (6-i));
    }
}

/**
 * hamming_channel_decode
 *
 * received - Value to be decoded.
 * value - Result of decode.
 */
void hamming_channel_decode(hamming_value7_t *received, hamming_value4_t * value)
{
    int i, j;

    if(received == NULL || value == NULL)
    {
        printf("%s: Got NULL pointer!\n", __func__);
        exit(-100);
    }

	bzero(value, sizeof(hamming_value4_t));
    for(i = 0; i < 4; i++)
    {
        for(j = 0; j < 7; j++)
        {
            value->bits[i] += (mR[i][j] * received->bits[j]);
        }
        value->bits[i] %= 2;
        value->value |= (value->bits[i] << (3-i));
    }
}

/**
 * hamming_parity_check
 *
 * received - 7 bit codeword to check.
 */
int  hamming_parity_check(hamming_value7_t * received)
{
    int i, j;
    int z[3] = {0};
    int result;

    if(received == NULL)
    {
        printf("%s: Got NULL pointer!\n", __func__);
        exit(-100);
    }

    result = 0;
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 7; j++)
        {
            z[i] += (mH[i][j] * received->bits[j]);
        }
        z[i] %= 2;
        result |= (z[i] << (2-i));
    }

    return result;
}
