#include <NewPing.h> // for sensors
#include <SPI.h> //communicate with the shield
#include <SD.h> // SD read and write functions

 
//sensor and pin stuff
#define TRIGGER_PIN0  1
#define ECHO_PIN0     3

#define TRIGGER_PIN1  2
#define ECHO_PIN1     5

#define TRIGGER_PIN2  8
#define ECHO_PIN2     9

//declare sensors
NewPing sonar[3] = {
NewPing(TRIGGER_PIN0, ECHO_PIN0),
NewPing(TRIGGER_PIN1, ECHO_PIN1),
NewPing(TRIGGER_PIN2, ECHO_PIN2)
};
int distance[3];
int lastDistance[3];
int counter = 0;

//file stuff
//create a file 
File myFile;



void setup() {
  Serial.begin(115200);
  
  //Initialize SD card and Check if the SD card and libary failed
  Serial.print("Initializing SD card...");
 
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }
  Serial.println("initialization done.");

  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist creating it.");
  }
  
  myFile = SD.open("example.txt", FILE_WRITE);
  myFile.close();

  // Check to see if the file exists:
  if (SD.exists("example.txt")) {
    Serial.println("example.txt exists.");
  } else {
    Serial.println("example.txt doesn't exist.");
  }

  // delete the file:
  //Serial.println("Removing example.txt...");
  //SD.remove("example.txt");
}

 
void loop() {
  
  //ping each sensor
   delay(500);
  for(int i = 0; i < 1; i++){
    Serial.print("Ping" + String(i) + ": ");
    int dist = sonar[i].ping_cm();
    distance[i] = dist;
    Serial.print(dist);
    Serial.println("cm");
    if(dist-lastDistance[i] < -150 ){
       myFile = SD.open("sensor"+String(i)+".txt", FILE_WRITE);
       counter++;
       myFile.println("Found " + String(counter));
       myFile.close();
    }
    lastDistance[i] = dist;
  }
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  Serial.println();
  
}
