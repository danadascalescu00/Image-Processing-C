#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern void grayscale(char* source_file_name, char* destination_file_name)
{
    FILE *fin, *fout;
    unsigned int size_img, width, height;
    unsigned char pRGB[3], aux;

    printf("Source file name: %s\n", source_file_name);

    fin = fopen(source_file_name, "rb");
    if(fin == NULL)
    {
        printf("Source file not found!");
        exit(1);
    }

    fout = fopen(destination_file_name, "wb+");
    if(fout == NULL)
    {
        printf("Couldn't create destination file!");
        exit(1);
    }

    fseek(fin, 2, SEEK_SET);
    fread(&size_img, sizeof(unsigned int), 1, fin);
    printf("Image size in bytes: %u\n", size_img);

    fseek(fin, 18, SEEK_SET);
    fread(&width, sizeof(unsigned int), 1, fin);
    fread(&height, sizeof(unsigned int), 1, fin);
    printf("Image size in pixels (width x height) : %u x %u \n", width, height);

    fseek(fin, 0, SEEK_SET);

    //copy byte by byte the source image in the new one
    unsigned char c;
    while(fread(&c,1,1,fin))
    {
        fwrite(&c,1,1,fout);
        fflush(fout);
    }
    fclose(fin);

    unsigned int padding;
    if(width % 4 != 0)
        padding = 4 - (3 * width) % 4;
    else
        padding = 0;

    printf("padding = %u \n", padding);

    fseek(fout, 54, SEEK_SET);

    unsigned int i, j;
    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width; j++)
        {
            //read the pixel colors
            fread(pRGB, 3, 1, fout);
            //conversion to gray pixel
            aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
            pRGB[0] = pRGB[1] = pRGB[2] = aux;
            fseek(fout, -3, SEEK_CUR);
            fwrite(pRGB, 3, 1, fout);
            fflush(fout);
        }
        fseek(fout, padding, SEEK_CUR);
    }

    fclose(fout);
}
