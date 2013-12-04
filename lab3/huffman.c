
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "huffman.h"

#define CODEBOOK_OUTPUT_FILE "huffman_codebook.c"

huffman_vlc_t codebook[256];

void sort_huffman_nodes(huffman_node_t ** list, int size);
void print_huffman_tree(huffman_node_t * tree, unsigned int depth, unsigned int code);
void free_huffman_tree(huffman_node_t * tree);
void print_codebook(huffman_vlc_t * codebook, int size);

int main(int argc, char **argv)
{
    int n = 0;
    int i = 0;
    int j = 0;
    //huffman_node_t * list[256] = {NULL};
    int listsz = 0;
    huffman_node_t ** list = NULL;
    
    // get the (symbol,freq) pairs from args
    if(argc % 2 != 1)
    {
        printf("Expected even number of args for (symbol,freq) pairs.\n");
        return -1;
    }

    // zero the codebook
    bzero(codebook, sizeof(codebook));

    n = (argc-1)/2;
    listsz = n;
    list = (huffman_node_t **) malloc(listsz * sizeof(huffman_node_t *));
    if(list == NULL)
    {
        printf("Failed on malloc!\n");
        return -2;
    }

    // create nodes for all values
    for(i = 0; i < n; i++)
    {
        char symbol;
        float freq;
        sscanf(argv[2*i+1], "%c", &symbol);
        sscanf(argv[2*i+2], "%f", &freq);
        list[i] = (huffman_node_t *) malloc(sizeof(huffman_node_t));
        if(list[i] == NULL)
        {
            printf("Error allocating memory for huffman_node_t!\n");
            return -2;
        }
        list[i]->left = NULL;
        list[i]->right = NULL;
        list[i]->symbol = symbol;
        list[i]->freq = freq;
    }

    // sort the nodes
    sort_huffman_nodes(list, listsz);

    // print for debug
    printf("\nSorted:\n");
    for(i = 0; i < n; i++)
    {
        printf("%c %f\n", list[i]->symbol, list[i]->freq);
    }

    // build the huffman tree
    while(listsz > 1)
    {
        huffman_node_t ** newlist = (huffman_node_t **) malloc(listsz * sizeof(huffman_node_t *));
        if(newlist == NULL)
        {
            printf("Failed on malloc!\n");
            return -2;
        }

        // sort the existing list
        sort_huffman_nodes(list, listsz);

        // now find the two lowest nodes and create a parent node
        huffman_node_t * node = (huffman_node_t *) malloc(sizeof(huffman_node_t));
        if(node == NULL)
        {
            printf("Failed on malloc!\n");
            return -2;
        }
        node->left = list[0];
        node->right = list[1];
        node->symbol = '\0';
        node->freq = list[0]->freq + list[1]->freq;
        newlist[0] = node;
        for(i = 2; i < listsz; i++)
        {
            newlist[i-1] = list[i];
        }

        free(list); // free old list
        list = newlist;
        listsz--;
    }

    // print the tree
    print_huffman_tree(list[0], 0, 0);

    // print the codebook
    print_codebook(codebook, 256);

    // free the memory
    free_huffman_tree(list[0]);
    free(list);

    return 0;
}

/**
 * Sorts a list of huffman_node_t structs in ascending freq order.
 */
void sort_huffman_nodes(huffman_node_t ** list, int size)
{
    int i, j;
    // using bubble sort for now, since we don't expect too many symbols (max 256)
    for(i = 0; i < size; i++)
    {
        for(j = 0; j < (size-1); j++)
        {
            if(list[j]->freq > list[j+1]->freq)
            {
                huffman_node_t * temp;
                temp = list[j+1];
                list[j+1] = list[j];
                list[j] = temp;
            }
        }
    }
}

/**
 * Recursively walks the tree and prints out the node values.
 */
void print_huffman_tree(huffman_node_t * tree, unsigned int depth, unsigned int code)
{
    int i = 0;
    if(depth == 0)
    {
        printf(" Symbol     Codeword\n");
        printf("==========  ======================\n");
    }
    if(tree->symbol != '\0')
    {
        codebook[tree->symbol].length = depth;
        codebook[tree->symbol].code = code;
        printf(" %c (0x%02x)    ", tree->symbol, tree->symbol);
        for(i = 0; i < depth; i++)
        {
            int bit = (code >> i) & 0x01;
            printf("%d", bit);
        }
        printf("\n");
    }
    if(tree->left != NULL)
    {
        int lcode = code | (0 << depth);
        print_huffman_tree(tree->left, depth+1, lcode);
    }
    if(tree->right != NULL)
    {
        int rcode = code | (1 << depth);
        print_huffman_tree(tree->right, depth+1, rcode);
    }
}

/**
 * Recursively walks the tree to free memory.
 */
void free_huffman_tree(huffman_node_t * tree)
{
    if(tree->left != NULL)
    {
        free_huffman_tree(tree->left);
    }
    if(tree->right != NULL)
    {
        free_huffman_tree(tree->right);
    }

    free(tree);
}

/**
 * Prints the code book to a file.
 */
void print_codebook(huffman_vlc_t * codebook, int size)
{
    FILE * file;
    int i = 0;

    file = fopen(CODEBOOK_OUTPUT_FILE, "w");
    if(file == NULL)
    {
        printf("Error on fopen!\n");
        exit(-3);
    }

    fprintf(file, "/** Automatically generated file from huffman program! */\n");
    fprintf(file, "#include \"huffman.h\"\n");
    fprintf(file, "huffman_vlc_t CODEBOOK[] = {\n");
    for(i = 0; i < size; i++)
    {
        fprintf(file, "    { %d, %d }", codebook[i].length, codebook[i].code);
        if(i != (size-1))
        {
            fprintf(file, ",");
        }
        fprintf(file, "\n");
    }
    fprintf(file, "};\n");
}
