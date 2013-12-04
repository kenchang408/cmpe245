/**
  Author:	Mujtaba Hassanpur
  			San Jose State University
			
  Course:	CMPE 245 - Embedded Wireless Architecture
  			Dr. Harry Li
			Fall 2013

  lisa.c	LISA algorithm implementation with test data.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lisa.h"

/**
  msg_buffer_alloc
  Allocates a 1KB buffer for incoming messages. Returns allocated memory area. Must be freed by caller.
 */
message_buffer_t * msg_buffer_alloc(void)
{
	message_buffer_t * result = (message_buffer_t *) malloc(sizeof(message_buffer_t));
	if(result != NULL)
	{
		bzero(result, sizeof(message_buffer_t));
		result->byte_offset = 0;
		result->bit_offset = 0;
	}
	return result;
}

/**
  msg_buffer_free
  Frees the memory for the given message buffer.
 */
void msg_buffer_free(message_buffer_t * buf)
{
	if(buf != NULL)
	{
		free(buf);
	}
}

/**
  msg_buffer_fill
  Fills the message buffer with random (garbage) data to simulate noise.
 */
void msg_buffer_fill(message_buffer_t * buf)
{
	int i;

	if(buf == NULL)
		return; // early bail

	// fill with random data
	srand(time(NULL));
	for(i = 0; i < MESSAGE_BUFFER_SIZE; i++)
	{
		char garbage = rand();
		buf->data[i] = garbage;
	}
}

/**
  msg_buffer_advance_bit
  Given a message buffer, advance the head by 1 bit.
 */
void msg_buffer_advance_bit(message_buffer_t * buf)
{
	if(buf == NULL)
		return; // early bail

	buf->bit_offset++;
	if(buf->bit_offset >= 8)
	{
		buf->byte_offset++;
		buf->bit_offset = 0;
	}
	if(buf->byte_offset >= MESSAGE_BUFFER_SIZE)
	{
		// wrap around
		buf->byte_offset = 0;
		buf->bit_offset = 0;
	}
}

/**
  msg_buffer_peek_packet
  Given a message buffer, get the top of the head as a message packet. Must be freed by caller.
 */
message_packet_t * msg_buffer_peek_packet(message_buffer_t * buf)
{
	int i, j;
	unsigned char * result;
	unsigned char temp;
	unsigned char curr;
	unsigned char next;

	if(buf == NULL)
		return; // early bail

	result = (unsigned char *) malloc(sizeof(message_packet_t));
	if(result != NULL)
	{
		for(i = 0; i < (sizeof(message_packet_t)); i++)
		{
			j = i + buf->byte_offset;
			if(j >= MESSAGE_BUFFER_SIZE)
			{
				result[i] = 0x00;
				continue;
			}

			curr = buf->data[j];
			next = (buf->bit_offset == 0)? 0x00 : buf->data[j+1];
			temp = (curr << buf->bit_offset) | (next >> (8 - buf->bit_offset));
			result[i] = temp;
		}
	}

	return (message_packet_t *) result;
}

/**
  buffer_compare
  Returns a value between 0 and 1 to indicate similarity. 1 means exactly same, 0 means no similarity whatsoever.
 */
float buffer_compare(unsigned char * buf1, unsigned char *buf2, size_t bufsz)
{
	int match_bits = 0;
	int total_bits = 0;
	int ibit;
	int ibyte;
	unsigned char temp_byte1;
	unsigned char temp_byte2;
	unsigned char temp_bit1;
	unsigned char temp_bit2;

	for(ibyte = 0; ibyte < bufsz; ibyte++)
	{
		temp_byte1 = buf1[ibyte];
		temp_byte2 = buf2[ibyte];

		for(ibit = 0; ibit < 8; ibit++)
		{
			temp_bit1 = temp_byte1 & (1<<(8-ibit));
			temp_bit2 = temp_byte2 & (1<<(8-ibit));
			if(temp_bit1 == temp_bit2)
			{
				match_bits++;
			}
			total_bits++;
		}
	}

	return ((float)match_bits/(float)total_bits);
}

/**
  inject_errors
  Injects random bit errors into the given buffer.
  */
void inject_errors(unsigned char * buf, size_t bufsz)
{
	int byteoff;
	int bitoff;
	unsigned char orig;
	unsigned char temp;
	
	byteoff = rand() % bufsz;
	bitoff = rand() % 8;
	orig = buf[byteoff];
	temp = buf[byteoff] | (byteoff << bitoff);
	buf[byteoff] = temp;

	printf("Injected error: byteoff=%d, bitoff=%d\n", byteoff, bitoff);
	printf("Replaced %02x with %02x\n", orig, temp);
}

/**
  print_hex
  Prints the given data in hex.
 */
void print_hex(void * data, size_t size)
{
	int i;
	unsigned char * bytes = (unsigned char *) data;

	if(data == NULL)
		return; //early bail

	if(size == 0)
		return; // no data

	printf("size=%d\n", (int)size);
	for(i = 0; i < size; i++)
	{
		if(i % 16 == 0)
		{
			printf("%04x: ", i);
		}

		unsigned char byte = bytes[i];
		printf(" %02x", byte);
		if((i+1) % 16 == 0)
		{
			printf("\n");
		}
	}
	printf("\n");
}

// dead code
#if 0

static unsigned char SYNC_FIELD_DATA[] = {
		0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
		0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf
	};
static unsigned char PAYLOAD_DATA[] = {
		'S', 'J', 'S', 'U', '_', 'C', 'M', 'P', 'E', '_', 'M', 'U', 'J', 'T', 'A', 'B',
		'A', ',', ' ', 'F', 'A', 'L', 'L', ' ', '2', '0', '1', '3', '.', '.', '.', '.'
	};

int main(int argc, char** argv)
{
	message_buffer_t * buffer;
	message_packet_t packet;
	unsigned int insert_offset;
	void * dest;
	int i, k;

	// set random seed
	srand(time(NULL));

	buffer = msg_buffer_alloc();
	if(buffer == NULL)
	{
		printf("Error on msg_buffer_alloc().");
		return -1;
	}
	
	//printf("Allocated message buffer:\n");
	print_hex(buffer->data, MESSAGE_BUFFER_SIZE);

	msg_buffer_fill(buffer);
	
	insert_offset = rand() % 32;
	dest = (void *)buffer->data + insert_offset;
	//insert_offset = 0;
	printf("Insert offset = %d, 0x%04x  Dest=0x%04x\n", insert_offset, insert_offset, dest);
	bzero(&packet, sizeof(message_packet_t));
	memcpy(packet.sync_field.data, SYNC_FIELD_DATA, sizeof(SYNC_FIELD_DATA));
	memcpy(packet.payload, PAYLOAD_DATA, sizeof(PAYLOAD_DATA));
	memcpy(dest, &packet, sizeof(message_packet_t));

	printf("Filled message buffer:\n");
	print_hex(buffer->data, MESSAGE_BUFFER_SIZE);

	// inject error
	for(i = 0; i < 4; i++)
	{
		inject_errors(dest, MESSAGE_PACKET_SIZE);
	}
	printf("Injected message with error:\n");
	print_hex(buffer->data, MESSAGE_BUFFER_SIZE);
}
#endif

int read_message_from_buffer(message_buffer_t * buffer, sync_field_t * sync_field, char * output, size_t outputsz)
{
    int k;

    if(buffer == NULL || sync_field == NULL || output == NULL)
    {
        printf("%s: got NULL pointer!\n", __func__);
        return -1;
    }
    
    buffer->byte_offset = 0;
    buffer->bit_offset = 0;

    for(k = 0; k < (1024-256); k++)
    {
        message_packet_t * test_packet = msg_buffer_peek_packet(buffer);
	float comp = buffer_compare((unsigned char *)&sync_field->data, (unsigned char *)test_packet, SYNC_FIELD_SIZE_BYTES);
	if(comp > 0.99)
	{
	    printf("Comparison: %.3f%% match (byteoff=%d, bitoff=%d)\n", comp, buffer->byte_offset, buffer->bit_offset);
	    printf("Found at k=%d\n", k);
	    print_hex(test_packet, sizeof(message_packet_t));

	    // print the message
	    memcpy(output, test_packet->payload, outputsz);
            output[outputsz-1] = 0x00; // null terminate
	    printf("Payload: %s\n", output);

            // free the test_packet
            free(test_packet);
            return 0;
	}
	free(test_packet);
	msg_buffer_advance_bit(buffer);
    }

    return -2;
}
