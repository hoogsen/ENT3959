#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h> 


//BMP image format reference
//Will *probably* be used if we make conversion software
/*
typedef struct{
    uint16_t signature[2] = "BM";
    uint32_t filesize;
    uint32_t reserved;
    uint32_t dataOffset;
} bmp_fileheader;

typedef struct{
    uint32_t infoHeaderSize;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bitsperpixel;
    uint32_t compression;
    uint32_t imageSize;
    uint32_t yRes;
    uint32_t xRes;
    uint32_t colorsUsed;
    uint32_t MIC;
} bmp_infoheader;

struct bmp_image {
    bmp_fileheader header;
    bmp_infoheader infoHeader;
    //Rest of image is actual pixel data
}
*/

int main(int argc, char *argv[]) {

    FILE* fp;
    char *buffer = malloc(sizeof(char) * 256); 
    
    if (argc < 2) {
        printf("Please give us a filename buddy\n");
        return -1;
    }

    const char* pathname = argv[1];

    if ((fp = fopen(pathname, "rb") != NULL)) {

    }

    return 0;
}
