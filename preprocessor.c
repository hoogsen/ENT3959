#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <sys/stat.h> 
#include <math.h>

#define NUM_LEDS 50
#define NUM_SLICES 32
#define MAX_BRIGHTNESS 32
#define PI 3.14159

struct PixelRGB{
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t pixelCount;
};

int main(int argc, char *argv[]) {

    FILE* fp; //file pointer that will be the .bmp image being manipulated
    char buf[256]; //buffer used to read characters from the .bmp image
    
    if (argc < 2) { //checking user input to make sure a file was given
                    //checking could also be done to make sure it's a bmp
                    //we didn't do that
        printf("Please give us a filename buddy\n");
        return -1; //exit program because a file wasn't given as input
    }

    const char* pathname = argv[1]; //first function argument is filename
    if ((fp = fopen(pathname, "rb")) != NULL) {
        //need to seek through the header to get information about image size
        //getting the value of where in the file the pixel RGBs are given
        fseek(fp, 0x0A, SEEK_SET); //0xA contains the offset of where pixel RGB info is
                                   //this is known from how .bmp works
        bzero(&buf,sizeof(buf)); //always clear the buffer before use
        fgets(buf, 4, fp); //read the 4 bytes from the bmp file (fp) which
                           //contain the offset into our buffer (buf)
        
        //buffer currently contains four different bytes, it doesn't "know"
        //what they are yet, this casting tells us they form one cohesive
        //32 bit value which is the data offset of where the RGB values begin
        uint32_t dataOffset = (uint32_t)(*buf);
        
        //bmp stores pixel RGB data in an "array", we need to know width 
        //and height to know how long to read for
        //another reason this is important is because if width isn't divisble 
        //by 4 it adds padding 0s to the end of the rows, what a cool format
        
        //note the next block of code is a repeat of what was done with
        //dataOffset, but for the width and height of the image as well
        fseek(fp, 0x12, SEEK_SET); //0x12 is the offet of where the 
                                   //header lists width, then lists height
        bzero(&buf,sizeof(buf)); 
        fgets(buf, 4, fp);
        uint32_t dataWidth = (uint32_t)(*buf);
        fseek(fp, 0x16, SEEK_SET); //fseek is required to fix the currency 
                                   //indicator not working as expected
                                   //this is needed but I don't know why
        bzero(&buf,sizeof(buf)); 
        fgets(buf, 4, fp);
        uint32_t dataHeight = (uint32_t)(*buf);

        //(0,0) in the array will be the bottomleft most pixel. 
        //This is because of how bmp stores pixels 
        struct PixelRGB pixelRGB[dataHeight][dataWidth];
        fseek(fp, dataOffset, SEEK_SET); //seek to where the pixels are 
                                         //based on obtained pixel offset
        
        //Begin looping through pixel data to obtain RGB values
        //the loop will read the bottomleft most pixel first, then
        //read from left to right, then ascends up one row at a time
        for(uint32_t i = 0; i < dataHeight; i++){ 
            for(uint32_t j = 0; j < dataWidth; j++){
                //An important note on the below section
                //BMPs that we are choosing to read from are 24 bit color
                //this means each value, red, green, and blue, should have
                //one byte of value to it, so we should only need to read
                //one byte at a time, which would be a fgets with 1
                //However
                //for some reason this breaks everything and it was never
                //explored why after I got it working with reading 2 bytes
                //I believe it has to do with BMP being little endian but
                //in 16 bit sets, what a cool file format!

                bzero(&buf,sizeof(buf)); 
                fgets(buf, 2, fp); //get 2 bytes from the file, store into buf
                pixelRGB[i][j].blue = (uint8_t)(*buf); //cast away extra info
                                                       //whats left is blue value

                bzero(&buf,sizeof(buf)); //repeats for green
                fgets(buf, 2, fp); 
                pixelRGB[i][j].green = (uint8_t)(*buf);

                bzero(&buf,sizeof(buf)); //repeats for red
                fgets(buf, 2, fp); 
                pixelRGB[i][j].red = (uint8_t)(*buf);
            }
            if(dataWidth%4 > 0){  //as previously mentioned, bmp format adds 
                                  //padding to the end to make the width 
                                  //divisible by 4, this skip the padding
                fseek(fp, (dataWidth%4), SEEK_CUR); 
            }
        }
        
        //Now for the part to get a usuable format that we can give to our arduino, 
        //the previous matrix is in X,Y (cartesian) and we need it in R,S (polar)
        //if we wanted to be efficient, we could entirely cut out the X & Y 
        //caluclation section and figure it out on the fly, however for now 
        //we don't need to be efficient and it is nice to have the intermediate 
        //output for debugging purposes 
        
        //polarRGB will be what we use to export the usuable file format
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
        
        //transposing the values are important because of how  polar coordinates
        //work, to figure out what ring a pixel falls in, a radius from 0,0 is 
        //needed. However 0,0 in our arrays is the bottom left, in translating 
        //to polar 0,0 should be the center of the image
        //thus, the transposition
        int32_t translatedI = 0;
        int32_t translatedJ = 0;
        for(uint32_t i = 0; i < dataHeight; i++){    //these loops will go through
            for(uint32_t j = 0; j < dataWidth; j++){ //all pixel data
                //keep in mind, the BMP format starts with the bottom left hand
                //corner of the image, so we want 0,0 to become -49, -49
                translatedI = i - (dataHeight-1)/2; //example: (100-1)/2 = -49
                translatedJ = j - (dataWidth-1)/2;  //same thing but for width


                //Two different calculations need to happen, distance from center
                //in order to put the pixel into a ring/radius/LED
                //and an angle calculation to find the proper time slice

                //sqrt(pixelI^2 + pixelJ^2) gives the radius/length of a line
                //drawn from the center of the image to that pixel
                //(dataHeight/2)/NUM_LEDs gives a fraction such that the max
                //of sqrt(pixelI^2+pixelJ^2) multiplied by that fraction
                //will be NUM_LEDs, ring is an int because we want the value
                //to truncate down to a whole number, we do not have a fractional
                //number of LEDs
                ring = (sqrt(translatedI*translatedI + translatedJ*translatedJ)) / (((double)dataHeight/2)/NUM_LEDS);
                //A small aside on the ring calculation
                //The ring calculation used to do a full pythagorean theorem
                //calculation based on image height and width, however this
                //would make the corner of the image have the farthest
                //ring/radius/LED value instead of the side of the square
                //this would produce blank sections where the side of the image
                //was supposed to be, since the max radius was 144 but the side
                //of the square only went to 100, so now it basis it
                //off the height only. Ideally the .bmp should be a square,
                //so either width or height could have been chosen.
                
                //All right slice is a bit simpler i hope
                //atan2 is inverse tangent but instead of returning a value
                //90 degrees to -90 degrees, it can return from 180 to -180
                //it does this with magic, it isn't our problem how trig works.
                //the +pi is an offset because we don't negative angle, we want
                //from 0 to 360 the 2*pi/NUM_SLICES does what happened with ring,
                //it creates a fraction where the max will be NUM_SLICES
                slice = (atan2(translatedI,translatedJ)+PI) / ((2*PI)/NUM_SLICES);
                //Someone paying attention will notice that atan2 is supposed to be
                //y,x but we have done x,y as inputs instead
                //i think we got lucky in that I put them in backwards, but
                //the fan spins "backwards" so it worked out
                
                if(ring < NUM_LEDS){ //juuust to make sure we aren't accessing
                                     //anything out of bounds by accident,
                                     //we check that our ring calculation
                                     //didn't go larger than number of LEDs
                                     //before playing with it
                    
                    //Part of how this works is that multiple pixels can fall
                    //into the same ring or time slice, think of a web drawn
                    //over a 10000x10000 image where we only have 4 time slices
                    //and 4 LEDs, what we're going to do is count every pixel
                    //that falls within a time slice/radius and average
                    //out the color of all of the pixels.
                    polarRGB[ring][slice].red += pixelRGB[i][j].red;
                    polarRGB[ring][slice].green += pixelRGB[i][j].green;
                    polarRGB[ring][slice].blue += pixelRGB[i][j].blue;
                    polarRGB[ring][slice].pixelCount++; //averages need a count
                }
            }
        }
        
        //these loops calculate out that average
        for(uint32_t i = 0; i < NUM_LEDS; i++){
            for(uint32_t j = 0; j < NUM_SLICES; j++){ 
                if(polarRGB[i][j].pixelCount>0){
                    polarRGB[i][j].red = polarRGB[i][j].red/polarRGB[i][j].pixelCount;
                    polarRGB[i][j].green=polarRGB[i][j].green/polarRGB[i][j].pixelCount;
                    polarRGB[i][j].blue =polarRGB[i][j].blue/polarRGB[i][j].pixelCount;
                }
            }
        }
      

        //and finally... the output to file
        //since the fan will want to access an entire time slice of values
        //at a time, instead of an entire ring of values, the inner loop
        //is the leds, the outer loops is the slices.
        //The arduino code reads in NUM_LEDS*3 values at a time, aka
        //all the RGB values for one slice of time, and it updates them
        //to the next RGB values in time once enough time has passed based
        //on the hall effect sensor calculations.
        //Characters are output because we just want the raw bytes being output,
        //each byte is an R, G, or B value.
        //There is also a max brightness calculation, we gotta conserve
        //battery life somehow. we do it here because division is expensive,
        //makes more sense to preprocess it out instead of doing it live 
        //on the spinning fan
        for(unsigned int i = 0; i < NUM_SLICES; i++){
	        for(unsigned int j = 0; j < NUM_LEDS; j++){
           	    printf("%c%c%c", polarRGB[j][i].red * MAX_BRIGHTNESS / 255, polarRGB[j][i].green * MAX_BRIGHTNESS / 255 , polarRGB[j][i].blue * MAX_BRIGHTNESS / 255);
	        }
        }

    }else{
        perror("opening file failed: "); //oopsie 
    }
    fclose(fp); //can't have any memory leaks last moment now can we
    return 0;
}
