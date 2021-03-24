#include <SPI.h>
#include <SD.h>

//Test File for SD card reader
void setup() {
    Serial.begin(9600);
    
    while (!Serial) {}

    if (!SD.begin())
        return;

    Serial.println("Boutta Start");

    //Should be a file loaded onto SD card named test.txt
    File f = SD.open("test.txt");
    
    if (!SDFile)
        Serial.println("error opening test.txt");

    else {
        SD_write(f, "hey boys ;)");        
        SD_read(f);
        SDFile.close();
    }
}

void loop() {}

void SD_write(File SDFile, char* msg) {
    //Read from file and print data to terminal
    SDFile.print(msg);
    SDFile.println();
}

void SD_read(File SDFile) {
    //Read from file and print data to terminal
    while (SDFile.available()) {
        Serial.write(SDFile.read());
    }
}
