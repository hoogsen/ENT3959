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
    
        SD_write(f, "{{0, 0, 0}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {0, 0, 0}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {0, 0, 0}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {0, 0, 0}, {44, 55, 66}, {44, 55, 66}, {0, 0, 0}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {0, 0, 0}, {44, 55, 66}, {2a, 3b, 4c}, {11, 22, 33}, {0, 0, 0}, {33, 44, 55}, {11, 22, 33}, {11, 22, 33}, {0, 0, 0}, {2a, 3b, 4c}, {2a, 3b, 4c}, {44, 55, 66}, {0, 0, 0}}, {{44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {39, 4a, 5b}, {1b, 2c, 3d}, {19, 2a, 3b}, {11, 22, 33}, {1b, 2c, 3d}, {39, 4a, 5b}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {88, 99, aa}, {99, aa, bb}, {8b, 9c, ad}, {6c, 7d, 8e}, {aa, bb, cc}, {2a, 3b, 4c}, {11, 22, 33}, {11, 22, 33}, {11, 22, 33}, {11, 22, 33}, {62, 73, 84}, {b4, c5, d6}, {dd, ee, ff}}, {{99, aa, bb}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {44, 55, 66}, {3d, 4e, 5f}, {11, 22, 33}, {11, 22, 33}, {11, 22, 33}, {82, 93, a4}, {9f, b0, c1}, {44, 55, 66}, {44, 55, 66}, {6a, 7b, 8c}, {aa, bb, cc}, {aa, bb, cc}, {aa, bb, cc}, {aa, bb, cc}, {aa, bb, cc}, {aa, bb, cc}, {66, 77, 88}, {11, 22, 33}, {11, 22, 33}, {11, 22, 33}, {90, a1, b2}, {dd, ee, ff}, {dd, ee, ff}}");       
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
