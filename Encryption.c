#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    unsigned char B, G, R;
}Pixel;

unsigned int XORSHIFT32(unsigned int seed[static 1])
{
    unsigned int x = seed[0];
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    seed[0] = x;
    return x;
}

unsigned int* generate_permutation(unsigned int random_numbers[], unsigned int length)
{
    unsigned int *permutation = (unsigned int*)calloc(length+1,sizeof(unsigned int));
    if(permutation == NULL)
    {
        printf("Memory allocation error!");
        exit(1);
    }

    unsigned int i;
    for(i = 0; i < length; i++)
        permutation[i] = i;

    unsigned int indexRND = 1;
    for(i = length - 1; i>=1; i--)
    {
        unsigned int numRND = random_numbers[indexRND++];
        numRND = numRND % (i + 1);
        swap(permutation[i],numRND);
    }

    return permutation;
}

extern void encryption(char* source_image_name, char* destination_image_name, FILE *key)
{
    FILE *fin, *fout;
    unsigned int width, height, SV, R0[1];

    fin = fopen(source_image_name, "rb");
    if(fin == NULL)
    {
        printf("Couldn't find the source image!");
        exit(1);
    }

    fout = fopen(destination_image_name, "wb+");
    if(fout == NULL)
    {
        printf("Couldn't create the encrypted image!");
        exit(1);
    }

    fscanf(key, "%u %u", &R0[0], &SV);

    fseek(fin, 18, SEEK_SET);
    fread(&width, sizeof(unsigned int), 1, fin);
    fread(&height, sizeof(unsigned int), 1, fin);

    //linearize image
    Pixel* P = upload_image(source_image_name);

    unsigned int *R = (unsigned int*)calloc(2*width*height+1,sizeof(unsigned int));
    R[0] = R0[0];

    //generating random sequence of numbers using XORSHIFT
    unsigned int i;
    for(i = 1; i <= 2*width*height; i++)
        R[i] = XORSHIFT32(R0);

    //ciphered image
    Pixel *C = (Pixel*)calloc(width*height,sizeof(Pixel));
    if(C == NULL)
    {
        printf("Memory allocation error for the linear form of the ciphered image!");
        exit(1);
    }

    unsigned int *sigma = generate_permutation(R,width*height);

    for(i = 0; i < width*height; i++)
        C[sigma[i]] = P[i];


    free(R);
    free(sigma);
    free(C);
}
