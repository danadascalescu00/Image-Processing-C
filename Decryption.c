#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    unsigned char B, G, R;
}Pixel;

unsigned int xorshift32(unsigned int seed[static 1])
{
    unsigned int r = seed[0];
    r ^= r << 13;
    r ^= r >> 17;
    r ^= r << 5;
    seed[0] = r;
    return r;
}

extern void decryption(char* source_image_name, char* destination_image_name, FILE *key)
{

}
