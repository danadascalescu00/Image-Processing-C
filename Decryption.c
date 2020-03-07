#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    unsigned char B, G, R;
}Pixel;

unsigned int xorshift32(unsigned int seed[static 1])
{
    unsigned int x = seed[0];
    x = x ^ x << 13;
    x = x ^ x >> 17;
    x = x ^ x << 5;
    seed[0] = x;
    return x;
}

unsigned int* generate_invers_permutation(unsigned int random_numbers[], unsigned int length)
{
    unsigned int* permutation = (unsigned int*)calloc(length,sizeof(unsigned int));
    unsigned int* invers_permutation = (unsigned int*)calloc(length,sizeof(unsigned int));
    if(permutation == NULL || invers_permutation == NULL)
    {
        printf("Memory allocation error for the random permutation!");
        exit(1);
    }

    for(unsigned int i = 0; i < length; i++)
        permutation[i] = i;

    unsigned int index = 1;
    for(unsigned int i = length - 1; i >= 1; i--)
    {
        unsigned int numRND = random_numbers[index++] % (i + 1);
        swap(&permutation[i],&permutation[numRND]);
    }

    for(unsigned int i = 0; i < length; i++)
        invers_permutation[permutation[i]] = i;

    free(permutation);

    return invers_permutation;
}

extern void decryption(char* source_file, char* destination_file, FILE *key)
{
    FILE *fin, *fout;
    unsigned int width, height, padding, SV, R0[1];
    unsigned char c;

    fin = fopen(source_file, "rb+");
    if(fin == NULL)
    {
        printf("Couldn't find the source image!");
        exit(1);
    }

    fout = fopen(destination_file, "wb+");
    if(fout == NULL)
    {
        printf("Couldn't create the deciphered image!");
        exit(1);
    }

    fseek(key, 0, SEEK_SET);
    fscanf(key, "%u %u", &R0[0], &SV);

    //copy byte by byte the source image into the new one(dechiphered image)
    while(fread(&c,1,1,fin))
    {
        fwrite(&c,1,1,fout);
        fflush(fout);
    }

    fseek(fin, 18, SEEK_SET);
    fread(&width, sizeof(unsigned int), 1, fin);
    fread(&height, sizeof(unsigned int), 1, fin);

    if(width % 4 != 0)
        padding = 4 - (3 * width) % 4;
    else
        padding = 0;

    //linearize the ciphered image
    Pixel* C = upload_image(source_file);

    //generating a sequence of random numbers of length 2*width*height using shift-register generator
    unsigned int *R = (unsigned int*)calloc(2*width*height+1,sizeof(unsigned int));
    if(R == NULL)
    {
        printf("Memory allocation error for the random sequence of numbers!");
        exit(1);
    }

    R[0] = R0[0];

    for(unsigned int i = 1; i <= 2*width*height; i++)
        R[i] = xorshift32(R0);

    unsigned int k = width*height-1;
    unsigned int X;
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            X = R[width*height + k];
            if(k == 0)
            {
                c = (SV << 24) >> 24;
                C[k].B = c ^ C[k].B;
                c = (SV << 16) >> 24;
                C[k].G = c ^ C[k].G;
                c = (SV << 8) >> 24;
                C[k].R = c ^ C[k].R;

                c = (X << 24) >> 24;
                C[k].B = C[k].B ^ c;
                c = (X << 16) >> 24;
                C[k].G = C[k].G ^ c;
                c = (X << 8) >> 24;
                C[k].R = C[k].R ^ c;

            }else{

                Pixel aux = C[k-1];
                C[k].B = aux.B ^ C[k].B;
                C[k].G = aux.G ^ C[k].G;
                C[k].R = aux.R ^ C[k].R;

                c = (X << 24) >> 24;
                C[k].B = C[k].B ^ c;
                c = (X << 16) >> 24;
                C[k].G = C[k].G ^ c;
                c = (X << 8) >> 24;
                C[k].R = C[k].R ^ c;
            }
            k--;
        }
    }

    //generating the invers of a random permutation using Durstenfeld's algorithm
    unsigned int *sigma_invers = generate_invers_permutation(R,width*height);

    //deciphered image
    Pixel* D = (Pixel*)calloc(width*height,sizeof(Pixel));
    if(D == NULL)
    {
        printf("Memory allocation error for the deciphered image!");
        exit(1);
    }

    //pixel permutation
    k = 0;
    for(unsigned int i = 0; i < height; i++)
    {
        for(unsigned int j = 0; j < width; j++)
        {
            D[sigma_invers[k]] = C[k];
            k++;
        }
    }

    fseek(fout, 54, SEEK_SET);
    for(int i = height - 1; i >= 0; i--)
    {
        for(int j = 0; j < width; j++)
        {
            fwrite(D + (i * width + j), sizeof(Pixel), 1, fout);
            fflush(fout);
        }
        fseek(fout, padding, SEEK_CUR);
    }

    fclose(fin);
    fclose(fout);
    free(D);
    free(C);
    free(sigma_invers);
    free(R);
}
