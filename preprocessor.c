//README
//only run with smaller sized bmps (like 100x100), there is a testBMP.bmp that is recommended to test with
//send output to a text file to get usuable arduino matrix (./a.out testBMP.bmp > array.txt)
//100x100 is because we are making a struct of size NxM based on how big the file is
//this is an ape way to do it, and we don't have the system memory to be able to do more than that, so we segfault or bus error
//this has to be fixed to do it piece by piece so that we don't flood the system

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <sys/stat.h> 
#include <math.h>

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

struct PixelRGB{
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t pixelCount;
};



//TODO:
//Get C struct working in arduino. i.e. CRGB from FastLED
//Extract the RGB data into a convenient struct
int main(int argc, char *argv[]) {
    FILE* fp;
    char buf[256];
    
    if (argc < 2) {
        printf("Please give us a filename buddy\n");
        return -1;
    }

    const char* pathname = argv[1];
    if ((fp = fopen(pathname, "rb")) != NULL) {
        //we need to seek through the header to get a bunch of information before we begin working
        //getting the value of where in the file the pixel RGBs are given
        fseek(fp, 0x0A, SEEK_SET); //from bmp file format, 0xA is where the 4 bytes that give the offset of where the pixel RGB information is
        bzero(&buf,sizeof(buf)); //always clear buf before use
        fgets(buf, 4, fp); //read 4 bytes containing the offset, they are stored as 8 bit characters
        uint32_t dataOffset = (uint32_t)(*buf); //convert characters to an int that will be used later to read RGB values from pixels

        
        //bmp stores pixel RGB data in an "array", we need to know width and height to know how long to read for
        //another reason this is important is because if width isn't divisble by 4 it adds padding 0's to the end of the rows
        
        fseek(fp, 0x12, SEEK_SET); //0x12 is where the header lists width, then lists height
        bzero(&buf,sizeof(buf)); 
        fgets(buf, 4, fp);  
        uint32_t dataWidth = (uint32_t)(*buf);
        fseek(fp, 0x16, SEEK_SET); //fseek required to fix currency indicator not working as expected, works correctly but not understood TODO
        bzero(&buf,sizeof(buf)); 
        fgets(buf, 4, fp);
        uint32_t dataHeight = (uint32_t)(*buf);

        
        struct PixelRGB pixelRGB[dataHeight][dataWidth]; //(0,0) in the array will be the bottomleft most pixel. This is also how bmp stores pixels 
        fseek(fp, dataOffset, SEEK_SET); //seek to where the pixels are based on obtained pixel offset
        
        //Begin looping through pixel data to obtain RGB values
        for(uint32_t i = 0; i < dataHeight; i++){ //BMP stores bottom left first, this is ideal for our case as explained later
            for(uint32_t j = 0; j < dataWidth; j++){ //still reads left to right, so this loop is unchanged 
                bzero(&buf,sizeof(buf)); 
                fgets(buf, 2, fp); //get 2 bytes from the file, store into buf
                pixelRGB[i][j].blue = (uint8_t)(*buf); //cast away extra information, whats left is blue value

                bzero(&buf,sizeof(buf)); //repeats for green
                fgets(buf, 2, fp); 
                pixelRGB[i][j].green = (uint8_t)(*buf);

                bzero(&buf,sizeof(buf)); //repeats for blue
                fgets(buf, 2, fp); 
                pixelRGB[i][j].red = (uint8_t)(*buf);
                //An important note on the above stuff:
                //I still don't fully understand how the currency indicator is working with fgets and fseek, fgets(buf,1,fp) <stuff> followed
                //by fseek(fp,1,SEEK_CUR) has the indicator in a different spot (or at least messes up the code) in a way that fgets(buf,2,fp)
                //does not. I think this has to do with BMP being little endian in 16 bit sets. another works correctly but not understood TODO
            }
            if(dataWidth%4 > 0){  //as previously mentioned, bmp format adds padding to the end to make the width divisible by 4, skip the padding
                fseek(fp, (dataWidth%4), SEEK_CUR); 
            }
        }
        
        //Display to standard out, this is purely for debugging and will likely be removed (TODO)
        /*
        for(unsigned int i = 0; i < dataHeight; i++){
            printf("{{%x, %x, %x}", pixelRGB[i][0].red ,pixelRGB[i][0].green ,pixelRGB[i][0].blue);
            for(unsigned int j = 1; j < dataWidth; j++){
                printf(", {%x, %x, %x}", pixelRGB[i][j].red ,pixelRGB[i][j].green ,pixelRGB[i][j].blue);
            }
            printf("},\n");
        }
        */

        //Now for the part to get a usuable display matrix that we can give to our arduino, the previous matrix is in X,Y and we need it in R,S
        //if we wanted to be efficient, we could entirely cut out the X & Y caluclation section and figure it out on the fly, however for now
        //we don't need to be efficient and it is nice to have the intermediate output for debugging purposes 
        //Another TODO to make things better is to have this functioned out better, whats a main doing doing all these steps? 
        
        
#define NUM_LEDS 49
#define NUM_SLICES 32
#define PI 3.14159
#define MAX_BRIGHTNESS 32
        //polarRGB will be what we use to export the usuable (text) matrix
        struct PixelRGB polarRGB[NUM_LEDS][NUM_SLICES];

        //creating default values
        for(uint32_t i = 0; i < NUM_LEDS; i++){
            for(uint32_t j = 0; j < NUM_SLICES; j++){
                polarRGB[i][j].red = 0;
                polarRGB[i][j].green = 0;
                polarRGB[i][j].blue = 0;
                polarRGB[i][j].pixelCount = 0;
            }
        }

        int32_t ring = 0;
        int32_t slice = 0;
        
        //translated values are important because of the way the pizza model works
        //to figure out what ring a pixel falls in, a radius from 0,0 is needed
        //0,0 in our arrays is the bottom left, but in the model 0,0 is in the center
        //thus, the translation
        int32_t translatedI = 0;
        int32_t translatedJ = 0;
        for(uint32_t i = 0; i < dataHeight; i++){
            for(uint32_t j = 0; j < dataWidth; j++){ //loop through all pixel data we have
                //keep in mind, the BMP format starts with the bottom left hand corner of the image, so 0,0 should be -49, -49
                translatedI = i - (dataHeight-1)/2; //for a 100x100 image the translation moves index [0][0] to position -49,-49
                translatedJ = j - (dataWidth-1)/2;
                //this looks like a lot of gibberish but is simple and TODO will be broken down more
                //basically its a distance from center calculation and then putting the data in a bucket
                //ring = (sqrt(translatedI*translatedI + translatedJ*translatedJ)) / (sqrt((dataHeight/2)*(dataHeight/2) + (dataWidth/2)*(dataWidth/2))/NUM_LEDS);
                ring = (sqrt(translatedI*translatedI + translatedJ*translatedJ)) / (((double)dataHeight/2)/NUM_LEDS);
                slice = (atan2(translatedI,translatedJ)+PI) / ((2*PI)/NUM_SLICES);  //same thing with slice as with ring
                //DEBUG
                //printf("%.3f\t", (atan2(translatedI,translatedJ)+PI));
                if(ring < NUM_LEDS){ //since our circle of translated values is within the square of the picture, those outside
                    //the circle don't have anywhere to go, and we do not count them
                    polarRGB[ring][slice].red += pixelRGB[i][j].red;
                    polarRGB[ring][slice].green += pixelRGB[i][j].green;
                    polarRGB[ring][slice].blue += pixelRGB[i][j].blue;
                    polarRGB[ring][slice].pixelCount++; //pixel count is needed because we're going to take the average of all values in our bucket later
                    //DEBUG
                    //printf("translatedI: %d, translatedJ: %d\n",translatedI,translatedJ);
                    //printf(" ring: %d slice %d \n", ring, slice);
                }
            }
                //DEBUG
                //printf("\n");
        }
        
        //averaging out the values
        for(uint32_t i = 0; i < NUM_LEDS; i++){
            for(uint32_t j = 0; j < NUM_SLICES; j++){ 
                if(polarRGB[i][j].pixelCount>0){
                    polarRGB[i][j].red = polarRGB[i][j].red/polarRGB[i][j].pixelCount;
                    polarRGB[i][j].green = polarRGB[i][j].green/polarRGB[i][j].pixelCount;
                    polarRGB[i][j].blue = polarRGB[i][j].blue/polarRGB[i][j].pixelCount;
                }
            }
        }
      
        /*
        //TODO remove this //DEBUG output
        for(unsigned int i = 0; i < NUM_SLICES; i++){
	        for(unsigned int j = 0; j < NUM_LEDS; j++){
           	    printf("%2x %2x %2x\t", polarRGB[j][i].red, polarRGB[j][i].green, polarRGB[j][i].blue);
	        }
            printf("\n"); //DEBUG
        }
        */

        //real output
        //first one is LEDs, second one is slices
        for(unsigned int i = 0; i < NUM_SLICES; i++){
	        for(unsigned int j = 0; j < NUM_LEDS; j++){
           	    printf("%c%c%c", polarRGB[j][i].red * MAX_BRIGHTNESS / 255, polarRGB[j][i].green * MAX_BRIGHTNESS / 255 , polarRGB[j][i].blue * MAX_BRIGHTNESS / 255);
	        }
        }


    //aa DEBUG this is all debug
     
        /*

    int32_t debugI = -49;
    int32_t debugJ = -1;
    int32_t debugRing = 0;
    int32_t debugSlice = 0;

    printf("\n%f", (atan2(debugJ,debugI) +PI) / ((2*PI)/NUM_SLICES)  );

    debugSlice = (atan2(debugJ,debugI) +PI) / ((2*PI)/NUM_SLICES);
    debugRing = (sqrt(debugI*debugI + debugJ*debugJ)) / ((double)(dataHeight/2)/NUM_LEDS);
    printf("\nring: %d\tslice: %d",debugRing,debugSlice);
    printf("\nring: %f",  (sqrt(debugI*debugI + debugJ*debugJ)) / (((double)(dataHeight-1)/2)/NUM_LEDS) );

    */


    }else{
        perror("opening file failed: ");
    }
    fclose(fp);
    return 0;
}
