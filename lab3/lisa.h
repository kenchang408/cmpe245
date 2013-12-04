/**
  Author:	Mujtaba Hassanpur
  			San Jose State University
			
  Course:	CMPE 245 - Embedded Wireless Architecture
  			Dr. Harry Li
			Fall 2013

  lisa.h	LISA algorithm implementation with test data.
 */

#include <stdlib.h>

#define SYNC_FIELD_SIZE_BYTES	32
#define PAYLOAD_SIZE_BYTES		32
#define MESSAGE_BUFFER_SIZE		1024
#define MESSAGE_PACKET_SIZE		64

typedef struct {
	unsigned char data[SYNC_FIELD_SIZE_BYTES];
} sync_field_t;

typedef struct {
	sync_field_t sync_field;
	unsigned char payload[PAYLOAD_SIZE_BYTES];
} message_packet_t;

typedef struct {
	unsigned short byte_offset;
	unsigned short bit_offset;
	unsigned char data[MESSAGE_BUFFER_SIZE];
} message_buffer_t;

/**
  msg_buffer_alloc
  Allocates a 1KB buffer for incoming messages. Returns allocated memory area. Must be freed by caller.
 */
message_buffer_t * msg_buffer_alloc();

/**
  msg_buffer_free
  Frees the memory for the given message buffer.
 */
void msg_buffer_free(message_buffer_t * buf);

/**
  msg_buffer_fill
  Fills the message buffer with random (garbage) data to simulate noise.
 */
void msg_buffer_fill(message_buffer_t * buf);

/**
  msg_buffer_advance_bit
  Given a message buffer, advance the head by 1 bit.
 */
void msg_buffer_advance_bit(message_buffer_t * buf);

/**
  msg_buffer_peek_packet
  Given a message buffer, get the top of the head as a message packet. Must be freed by caller.
 */
message_packet_t * msg_buffer_peek_packet(message_buffer_t * buf);

/**
  buffer_compare
  Returns a float between 0 and 1 to indicate similarity between the given buffers. 0 means no matches
  at all. 1 means exactly identical.
 */
float buffer_compare(unsigned char * buf1, unsigned char * buf2, size_t bufsz);

/**
  read_message_from_buffer
  Attempts to read a message from the given buffer data by using the specified sync field data.
*/
int read_message_from_buffer(message_buffer_t * buffer, sync_field_t * sync_field, char * output, size_t outputsz);

/**
  print_hex
  Prints the given data in hex.
 */
void print_hex(void * data, size_t size);
