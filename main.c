#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    unsigned char B, G, R;
}Pixel;

extern void swap(unsigned int *xp, unsigned int *yp)
{
    if(xp == yp)
        return;

    *xp = *xp ^ *yp;
    *yp = *xp ^ *yp;
    *xp = *xp ^ *yp;
}

unsigned int sizeFile(FILE *f)
{
    unsigned int pos, size;

    pos = ftell(f);
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fseek(f, pos, SEEK_SET);

    return size;
}

extern Pixel* upload_image(char* source_image)
{
    FILE *fin;
    unsigned int width, height, padding;
    unsigned char pRGB[3];

    fin = fopen(source_image, "rb");
    if(fin == NULL)
    {
        printf("Error! Source image not found!");
        return NULL;
    }

    fseek(fin, 18, SEEK_SET);
    fread(&width, sizeof(unsigned int), 1, fin);
    fread(&height, sizeof(unsigned int), 1, fin);

    if(width % 4 != 0)
        padding = 4 - (3 * width) % 4;
    else
        padding = 0;

    Pixel *plain_image = (Pixel*)malloc((width + padding) * height * sizeof(Pixel));

    fseek(fin, 54, SEEK_SET);
    for(int i = height - 1; i >= 0; i--)
    {
        for(int j = 0; j < width; j++)
        {
            fread(pRGB, 3, 1, fin);
            plain_image[i * width + j].B = pRGB[0];
            plain_image[i * width + j].G = pRGB[1];
            plain_image[i * width + j].R = pRGB[2];
        }
        fseek(fin, padding, SEEK_CUR);
    }
    fclose(fin);

    return plain_image;
}

void encryption_verification(char* source_image, char* compare_source)
{
    FILE *ftest, *fok;

    unsigned int width, height, padding, i, j;
    unsigned char pOK[3], pTest[3], byteTest, byteOK, h;

    ftest = fopen(source_image, "rb");
    if(ftest == NULL)
    {
        printf("Couldn't find the image for testing!");
        exit(1);
    }

    fok = fopen(compare_source, "rb");
    if(fok == NULL)
    {
        printf("Couldn't find the image for comparison!");
        exit(1);
    }

    printf("Ciphered image tested: %s \n", source_image);

    if(sizeFile(fok) != sizeFile(ftest))
    {
        printf("Ciphered image tested (%s) doesn't have the right size!", source_image);
        fclose(fok);
        fclose(ftest);
        exit(1);
    }

    for(h = 0; h < 54; h++)
    {
        fread(&byteOK,1,1,fok);
        fread(&byteTest,1,1,ftest);

        if(byteOK != byteTest)
        {
            printf("Ciphered image tested (%s) has the header with a different byte at the position %u!", source_image, h);
            fclose(fok);
            fclose(ftest);
            exit(1);
        }
    }

    fseek(fok, 18, SEEK_SET);
    fread(&width, sizeof(unsigned int), 1, fok);
    fread(&height, sizeof(unsigned int), 1, fok);

    if(width % 4 != 0)
        padding = 4 - (3 * width) % 4;
    else
        padding = 0;

    fseek(fok, 54, SEEK_SET);
    fseek(ftest, 54, SEEK_SET);

    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width; j++)
        {
            fread(pOK, 3, 1, fok);
            fread(pTest, 3, 1, ftest);

            if(memcmp(pOK, pTest, 3) != 0)
            {
                printf("Ciphered image tested (%s) has a different value at the line %u and column %u!", source_image, height - 1 - i, j);
                fclose(fok);
                fclose(ftest);
                exit(1);
            }
        }

        fseek(fok, padding, SEEK_CUR);
        fseek(ftest, padding, SEEK_CUR);
    }

    printf("Ciphered imagine tested (%s) is correctly encrypted!", source_image);
}

int main()
{
    FILE *key, *fin, *fout, *in;
    char source_image[32], encrypted_image[32], decrypted_image[32];

    in = fopen("files.in", "r");
    if(in == NULL)
    {
        printf("Couldn't open data file!");
        exit(1);
    }

    key = fopen("secret_key.txt", "r");
    if(key == NULL)
    {
        printf("Couldn't open secret_key file!");
        exit(1);
    }

    fscanf(in, "%s",  source_image);
    fscanf(in, "%s", encrypted_image);
    fscanf(in, "%s", decrypted_image);

    encryption(source_image,encrypted_image,key);
    fclose(key);






    key = fopen("secret_key.txt", "r");
    if(key == NULL)
    {
        printf("Couldn't open secret_key file!");
        exit(1);
    }
    decryption(encrypted_image,decrypted_image,key);
    fclose(key);

    return 0;
}
