#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define theoretical_threshold_value 293.25
#define theoretically_estimated_frequency(m,n) (m*n)/256.0

typedef struct{
    unsigned char B, G, R;
}Pixel;

unsigned int XORSHIFT32(unsigned int seed[static 1])
{
    unsigned int x = seed[0];
    x = x ^ x << 13;
    x = x ^ x >> 17;
    x = x ^ x << 5;
    seed[0] = x;
    return x;
}

unsigned int* generate_permutation(unsigned int random_numbers[], unsigned int length)
{
    unsigned int* permutation = (unsigned int*)calloc(length,sizeof(unsigned int));
    if(permutation == NULL)
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
    return permutation;
}

extern void encryption(char* source_file, char* destination_file, FILE *key)
{
    FILE *fin, *fout;
    unsigned int width, height, padding, SV, R0[1];
    unsigned char c;

    fin = fopen(source_file, "rb");
    if(fin == NULL)
    {
        printf("Couldn't find the source image!");
        exit(1);
    }

    fout = fopen(destination_file, "wb+");
    if(fout == NULL)
    {
        printf("Couldn't create the ciphered image!");
        exit(1);
    }

    fscanf(key, "%u %u", &R0[0], &SV);

    //copy byte by byte the source image into the new one(encrypted image)
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

    //linearize image
    Pixel* P = upload_image(source_file);

    //generating a sequence of random numbers of length 2*width*height using shift-register generator
    unsigned int *R = (unsigned int*)calloc(2*width*height+1,sizeof(unsigned int));
    if(R == NULL)
    {
        printf("Memory allocation error for the random sequence of numbers!");
        exit(1);
    }

    R[0] = R0[0];

    for(unsigned int i = 1; i <= 2*width*height; i++)
        R[i] = XORSHIFT32(R0);

    //generating a random permutation using Durstenfeld's algorithm
    unsigned int *sigma = generate_permutation(R,width*height);

    //ciphered image
    Pixel* C = (Pixel*)calloc(width*height,sizeof(Pixel));
    if(C == NULL)
    {
        printf("Memory allocation error for the ciphered image!");
        exit(1);
    }

    //pixel permutation
    unsigned int k = 0;
    for(unsigned int i = 0; i < height; i++)
    {
        for(unsigned int j = 0; j < width; j++)
        {
            C[sigma[k]] = P[k];
            k++;
        }
    }

    k = 0;
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
            k++;
        }
    }

    fseek(fout, 54, SEEK_SET);
    for(int i = height - 1; i >= 0; i--)
    {
        for(int j = 0; j < width; j++)
        {
            fwrite(C + (i * width + j), sizeof(Pixel), 1, fout);
            fflush(fout);
        }
        fseek(fout, padding, SEEK_CUR);
    }

    fclose(fin);
    fclose(fout);
    free(P);
    free(C);
    free(R);
    free(sigma);
}

extern void Chi_Squared_Distribution(char* source_file)
{
    printf("Chi_Squared distribution test for %s: \n", source_file);

    FILE *fin;
    unsigned int width, height, padding;
    unsigned char pRGB[3];
    float chi_squared, frequency_blue[256], frequency_green[256], frequency_red[256], tef;

    fin = fopen(source_file, "rb+");
    if(fin == NULL)
    {
        printf("Couldn't find the source image!");
        return;
    }

    fseek(fin, 18, SEEK_SET);
    fread(&width, sizeof(unsigned int), 1, fin);
    fread(&height, sizeof(unsigned int), 1, fin);

    tef = theoretically_estimated_frequency(width,height);

    if(width % 4 != 0)
        padding = 4 - (3 * width) % 4;
    else
        padding = 0;

    fseek(fin, 54, SEEK_SET);
    for(int i = 0; i < height; i++)
    {
        for(int j = 0; j < width; j++)
        {
            fread(pRGB, 3, 1, fin);
            frequency_blue[pRGB[0]] += pRGB[0];
            frequency_green[pRGB[1]] += pRGB[1];
            frequency_red[pRGB[2]] += pRGB[2];
        }
    }


    fclose(fin);
}
