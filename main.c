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

    *xp = *x ^ *y;
    *y = *x ^ *y;
    *x = *x ^ *y;
}

extern Pixel* upload_image(char* source_file_name)
{
    FILE *in;
    unsigned int width, height, padding;
    unsigned char pRGB[3];

    in = fopen(source_file_name, "rb");
    if(in == NULL)
    {
        printf("Error! Source file not found!");
        exit(1);
    }

    fseek(in, 18, SEEK_SET);
    fread(&width, sizeof(unsigned int), 1, in);
    fread(&height, sizeof(unsigned int), 1, in);

    if(width % 4 != 0)
        padding = 4 - (3 * width) % 4;
    else
        padding = 0;

    Pixel *plain_image = (Pixel*)malloc((width + padding)*height*sizeof(Pixel));
    if(plain_image == NULL)
    {
        printf("Memory allocation error!");
        exit(1);
    }

    fseek(in, 54, SEEK_SET);
    unsigned int i,j;
    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width; j++)
        {
            fread(pRGB, 3, 1, in);
            plain_image[i*width+j].B = pRGB[0];
            plain_image[i*width+j].G = pRGB[1];
            plain_image[i*width+j].R = pRGB[2];
        }
        fseek(in, padding, SEEK_CUR);
    }
    fclose(in);

    return plain_image;
}

int main()
{
    FILE *in, *key, *fin, *fout;
    char source_image[32], image_encrypted[32], image_decrypted[32];

    key = fopen("secret_key.txt", "rt");
    if(key == NULL)
    {
        printf("Secret key not found!");
        exit(1);
    }

    in = fopen("files.in", "r");
    if(in == NULL)
    {
        printf("Couldn't open date file!");
        exit(1);
    }

    fscanf(in, "%s", source_image);

    return 0;
}
