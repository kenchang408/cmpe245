#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

#include "lisa.h"
#include "huffman.h"
#include "huffman_codec.h"

#define CMD_RX	1
#define CMD_TX  0
// modes
#define RXMODE  1
#define TXMODE  0

// RX states
#define RX_STATE_SYNC   0
#define RX_STATE_DATA   1

// static variables
static const unsigned char SYNC_FIELD_DATA[] =
{
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf
};

// declared in huffman_codebook.c
extern huffman_vlc_t CODEBOOK[];

int debug_mode = 0;

// prototypes
void    print_usage_exit(void);
int     open_board(void);
int     enter_rx_mode(int bps);
int     enter_tx_mode(int bps);
int     xor(int a, int b);

int xor(int a, int b)
{
    if(a == b)
    {
        return 0;
    }
    else if(a == 1)
    {
        return 1;
    }
    else if(b == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int main(int argc, char **argv)
{
    int result = 0;
    int bps = 100; // default at 100bps
    int mode = RXMODE; // default RX mode
    int i;

    for(i = 1; i < argc; i++)
    {
        if(i+1 >= argc)
        {
            print_usage_exit();
        }

        if(strcmp("-m", argv[i]) == 0)
        {
            if(strcmp("rx", argv[i+1]) == 0)
            {
                printf("RXMODE\n");
                mode = RXMODE;
                i++;
            }
            else if(strcmp("tx", argv[i+1]) == 0)
            {
                printf("TXMODE\n");
                mode = TXMODE;
                i++;
            }
            else
            {
                print_usage_exit();
            }
        }
        else if(strcmp("-s", argv[i]) == 0)
        {
            sscanf(argv[i+1], "%d", &bps);
            printf("BPS = %d\n", bps);
            i++;
        }
        else if(strcmp("-d", argv[i]) == 0)
        {
            if(strcmp("on", argv[i+1]) == 0)
            {
                // debug mode
                debug_mode = 1;
                printf("DEBUG ON");
            }
            else
            {
                // debug mode off
                debug_mode = 0;
                printf("DEBUG OFF");
            }
            i++;
        }
        else
        {
            // if we get here then something is wrong
            print_usage_exit();
        }
    }

    switch(mode)
    {
        case RXMODE:
            result = enter_rx_mode(bps);
            break;

        case TXMODE:
            result = enter_tx_mode(bps);
            break;

        default:
            // shouldn't happen but just in case
            print_usage_exit();
            break;
    }

    return result;
}

void print_usage_exit(void)
{
    printf("Usage: txceiver \n\t-m (tx|rx)\tOperation mode (TX or RX).");
    printf("\n\t-s <bps>\tOperation speed (in bps).\n\n");
    exit(0);
}

int open_board(void)
{
    int fd = open("/dev/txceiver0", 0);
    if(fd < 0)
    {
        fd = open("/dev/txceiver", 0);
    }
    if(fd < 0)
    {
        perror("open device txceiver");
        exit(1);
    }

    return fd;
}

int enter_rx_mode(int bps)
{
    int state = RX_STATE_SYNC;
    int value;
    unsigned char byte = 0;
    int bitoff = 0;
    int byteoff = 0;
    // prevent div by 0, default = 100bps
    int usdelay = (bps != 0)? (1000000/bps) : 10000;
    sync_field_t sync_field;
    char payload[PAYLOAD_SIZE_BYTES];
    struct timeval tv;
    unsigned long ustime;
    unsigned long elapsed;
    unsigned char * read_bits;
    unsigned char * sync_bits;
    int index = 0;
    int match_count = 0;
    int i = 0;
    int bitArraySize = SYNC_FIELD_SIZE_BYTES * 8;
    int d1, d2, d3, d4, d5, d6, d7, r;
    int lastSyncIndex;

    // allocate memory
    read_bits = (unsigned char *) malloc(SYNC_FIELD_SIZE_BYTES * 8);
    sync_bits = (unsigned char *) malloc(SYNC_FIELD_SIZE_BYTES * 8);
    if(read_bits == NULL || sync_bits == NULL)
    {
        printf("Error allocating memory for bit buffer\n");
        return -2;
    }

    // init the sync field
    bzero(read_bits, SYNC_FIELD_SIZE_BYTES * 8);
    bzero(sync_bits, SYNC_FIELD_SIZE_BYTES * 8);
    bzero(payload, PAYLOAD_SIZE_BYTES);
    bzero(sync_field.data, SYNC_FIELD_SIZE_BYTES);
    memcpy(sync_field.data, &SYNC_FIELD_DATA, SYNC_FIELD_SIZE_BYTES);

    // grab the sync bits
    for(index = 0; index < (SYNC_FIELD_SIZE_BYTES*8); index++)
    {
        int byteoffset = index / 8;
        int bitoffset = index % 8;
        sync_bits[index] = (sync_field.data[byteoffset] >> (7-bitoffset)) & 0x01;
    }

    index = 0;
    lastSyncIndex = 0;
    printf("Receiving data (RX):\n ");
    while(1) // loop indefinitely
    {
        int pos = -1;
        unsigned char sync_byte = 0x00;
        // processing could take a while so measure time
        gettimeofday(&tv, NULL);
        ustime = 1000000 * tv.tv_sec + tv.tv_usec;

        int fd = open_board();
        ioctl(fd, CMD_RX, &value);
        close(fd);

        if(state == RX_STATE_SYNC)
        {
            float comp;
            read_bits[index] = value;
            match_count = 0;
            for(i = 0; i < bitArraySize; i++)
            {
                unsigned char d = read_bits[(i+index+1)%bitArraySize];
                unsigned char s = sync_bits[i];
                if(d == s)
                {
                    match_count++;
                }
            }
            
            sync_byte = 0x00;
            for(i = 0; i < 8; i++)
            {
                sync_byte |= (read_bits[(index-7+i)%bitArraySize] << (7-i));
            }
            if(((sync_byte & 0xf0) == 0x50) || ((sync_byte & 0xf0) == 0xa0))
            {
                bitoff = index % 8;
                if((sync_byte & 0xf0) == 0x50)
                {
                    pos = 0;
                    pos += (sync_byte & 0x0f);
                }
                else if((sync_byte & 0xf0) == 0xa0)
                {
                    pos = 16;
                    pos += (sync_byte & 0x0f);
                }
                    //printf("Sync pos=%d, byte=0x%02x\n", pos, sync_byte);
            }

            // make sure we get sync bits and good enough signal
            comp = (float)match_count / (float)bitArraySize;
            if(debug_mode == 1)
                printf("Comp: %.3f, Read %d\n", comp, value);
            if(comp >= 0.700 && sync_byte == 0xaf)
            {
                printf("Got sync field! (%.2f%%)\n", (comp * 100));
                //printf("Got sync field!\n");
                state = RX_STATE_DATA;
                bitoff = 0; // was 1
                byteoff = 0;
                byte = 0x00;
                pos = 0;
                sync_byte = 0x00;

                // reset scrambler
                d1 = 0;
                d2 = 0;
                d3 = 0;
                d4 = 0;
                d5 = 0;
                d6 = 0;
                d7 = 0;
            }
            else
            {
                index = (index+1) % bitArraySize;
            }
        }
        else if(state == RX_STATE_DATA)
        {
            // de-scramble
            r = value;
            value = xor(xor(d4, d7), r);
            d7 = d6;
            d6 = d5;
            d5 = d4;
            d4 = d3;
            d3 = d2;
            d2 = d1;
            d1 = r;

            //printf("r = %d, value = %d\n", r, value);

            byte = byte | (value<<(7-bitoff));
            if(bitoff == 7)
            {
                // last bit so print it
                printf(" %02x", byte);
                payload[byteoff] = byte;
                byteoff++;
                if(byteoff % 16 == 0)
                {
                    printf("\n");
                }
                if(byteoff >= PAYLOAD_SIZE_BYTES)
                {
                    //payload[PAYLOAD_SIZE_BYTES-1] = 0x00; // null terminate
                    //printf("Raw Message: %s\n\n", payload);

                    // huffman decode
                    unsigned char * decoded = (unsigned char *) malloc(PAYLOAD_SIZE_BYTES);
                    if(decoded == NULL)
                    {
                        printf("%s: Failed on malloc!\n", __func__);
                        exit(-1);
                    }
                    huffman_decode(payload, decoded, PAYLOAD_SIZE_BYTES);
                    decoded[PAYLOAD_SIZE_BYTES-1] = 0x00; // null terminate
                    printf("Decoded Message: %s\n", decoded);

                    state = RX_STATE_SYNC;
                    index = 0;
                    bzero(read_bits, bitArraySize);
                }
                else
                {
                    byteoff = byteoff % MESSAGE_BUFFER_SIZE;
                    fflush(stdout);
                    byte = 0;
                }
            }

            bitoff = (bitoff+1) % 8;
        }

        /*
        // try to read message (we get 0 on success)
        if(read_message_from_buffer(buffer, &sync_field, payload, PAYLOAD_SIZE_BYTES) == 0)
        {
            if(bitoff != 1) printf("%02x ", byte); // remaining data (just in case)

            printf("Got Message: %s\n\n", payload);
            // clear the buffer now
            bzero(buffer->data, MESSAGE_BUFFER_SIZE);
            buffer->byte_offset = 0;
            buffer->bit_offset = 0;
            byte = 0;
            bitoff = 0;
            byteoff = 0;
        }
        */

        // determine how long to sleep
        gettimeofday(&tv, NULL);
        elapsed = 1000000 * tv.tv_sec + tv.tv_usec;
        elapsed -= ustime;

        if(elapsed < usdelay)
        {
            usleep(usdelay-elapsed);
        }
        else
        {
            printf("Warning: unable to keep up with bitrate!\n");
        }
        //usleep(usdelay);
    }

    free(read_bits);
    free(sync_bits);

    return 0;
}

int enter_tx_mode(int bps)
{
    int value = 0;
    int usdelay = (bps != 0)? (1000000/bps) : 10000;
    //char msg[] = "HELLO!";
    message_packet_t * packet;
    unsigned char * packet_raw;
    int bitoff = 0;
    int byteoff = 0;
    int byte = 0;
    struct timeval tv;
    unsigned long ustime;
    unsigned long elapsed;
    int d1, d2, d3, d4, d5, d6, d7, t;
    
    packet = (message_packet_t *) malloc(sizeof(message_packet_t));
    if(packet == NULL)
    {
        printf("Error allocating buffer for message!\n");
        return -1;
    }

    bzero(packet, sizeof(message_packet_t));
    memcpy((void *)packet->sync_field.data, (void *)&SYNC_FIELD_DATA, SYNC_FIELD_SIZE_BYTES);
    //strncpy((char *)packet->payload, (char *)msg, PAYLOAD_SIZE_BYTES);
    packet_raw = (unsigned char *) packet;

    while(1) // loop indefinitely
    {
        // prompt for message
        if(byteoff == 0 && bitoff == 0)
        {
            unsigned char message[PAYLOAD_SIZE_BYTES];
            printf("Enter message (32 bytes max): ");
            bzero(message, PAYLOAD_SIZE_BYTES);
            fgets(message, PAYLOAD_SIZE_BYTES, stdin);

            // Huffman encode
            bzero(packet->payload, PAYLOAD_SIZE_BYTES);
            huffman_encode(message, packet->payload, PAYLOAD_SIZE_BYTES);

            // reset scrambler
            d1 = 0;
            d2 = 0;
            d3 = 0;
            d4 = 0;
            d5 = 0;
            d6 = 0;
            d7 = 0;
        }

        // processing could take a while so measure time
        gettimeofday(&tv, NULL);
        ustime = 1000000 * tv.tv_sec + tv.tv_usec;

        // get byte to write
        byte = packet_raw[byteoff];
        value = (byte >> (7-bitoff)) & 0x01;

        if(byteoff >= SYNC_FIELD_SIZE_BYTES)
        {
            // scramble payload
            t = xor(xor(d4, d7), value);
            d7 = d6;
            d6 = d5;
            d5 = d4;
            d4 = d3;
            d3 = d2;
            d2 = d1;
            d1 = t;
            value = t;
            //printf("value = %d, t = %d\n", value, t);
        }

        int fd = open_board();
        ioctl(fd, CMD_TX, value);
        close(fd);
        if(debug_mode == 1) printf("Write TX: %d\n", value);
        
        if(bitoff == 7)
        {
            byteoff = (byteoff + 1) % sizeof(message_packet_t);
        }
        bitoff = (bitoff + 1) % 8;
        
        // determine how long to sleep
        gettimeofday(&tv, NULL);
        elapsed = 1000000 * tv.tv_sec + tv.tv_usec;
        elapsed -= ustime;

        if(elapsed < usdelay)
        {
            usleep(usdelay-elapsed);
        }
        else
        {
            printf("Warning: unable to keep up with bitrate!\n");
        }
        
        //usleep(usdelay);
    }

    return 0;
}
