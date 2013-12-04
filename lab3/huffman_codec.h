#ifndef __HUFFMAN_CODEC_H__
#define __HUFFMAN_CODEC_H__

void huffman_encode(const unsigned char * message, unsigned char * buffer, int bufsize);
void huffman_decode(const unsigned char * message, unsigned char * buffer, int bufsize);

#endif // __HUFFMAN_CODEC_H__
