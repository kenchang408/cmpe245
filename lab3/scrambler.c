#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scrambler.h"

#if 1
#define BUFSIZE 16
static unsigned char msg[] = "HELLO!";

int xor(int a, int b);

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
    unsigned int i;
    unsigned int value;
    unsigned int scram[8] = {0};
    unsigned int x1, x2;
    unsigned int d1, d2, d3, d4, d5, d6, d7;
    unsigned int t;
    unsigned int r;

    d1 = 0;
    d2 = 0;
    d3 = 0;
    d4 = 0;
    d5 = 0;
    d6 = 0;
    d7 = 0;
    for(i = 0; i < 8; i++)
    {
        value = 1;
        t = xor(xor(d4, d7), value);
        //x2 = xor(d4, d7);
        //x1 = xor(x2, value);
        d7 = d6;
        d6 = d5;
        d5 = d4;
        d4 = d3;
        d3 = d2;
        d2 = d1;
        d1 = t;

        scram[i] = t;
        printf("value = %d, t = %d\n", value, t);
    }

    d1 = 0;
    d2 = 0;
    d3 = 0;
    d4 = 0;
    d5 = 0;
    d6 = 0;
    d7 = 0;
    for(i = 0; i < 8; i++)
    {
        r = scram[i];
        value = xor(xor(d4, d7), r);
        //x2 = xor(d4, d7);
        //x1 = xor(x2, r);
        d7 = d6;
        d6 = d5;
        d5 = d4;
        d4 = d3;
        d3 = d2;
        d2 = d1;
        d1 = r;

        printf("r = %d, value = %d\n", r, value);
    }
}
#endif
