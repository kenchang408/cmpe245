
#define END_OF_TRANSMISSION 0x04
#define AGGREGATE_NODE      0

typedef struct huffman_node_t
{
    char symbol;
    float freq;
    struct huffman_node_t * left;
    struct huffman_node_t * right;
} huffman_node_t;

typedef struct huffman_vlc_t
{
    unsigned int length;
    unsigned int code;
} huffman_vlc_t;
