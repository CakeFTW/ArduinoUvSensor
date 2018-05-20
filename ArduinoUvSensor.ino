#include <MedianFilter.h>
#include <SPI.h> //communicate with the shield
#include <SD.h> // SD read and write functions
// IMPORTANT STATE READ WHAT THEY DO
#define SSDSHIELD // If this is defined the code will assume the SSD SHIELD is attached if not it will not compile any logging into the code 
#define DEBUG // defining this makes the program print to the serial monitor, not defining it prevents the any printing functions from being complied into the code


//chanels for the different sensors
int irPins[3] = {A0,A1,A2};
int soapPins[3] = {A3,A4,A5};
int flushPins[3] = {A6,A7,A8};

//timer for debuging via serial
unsigned long timer = 0;

//class for holding the user data
class User{
  public:
  unsigned long flushStamp; // time stamp for when the flush is pressed
  bool hasFlushed;
  unsigned long initHw; //initial handwashing time
  bool hasHw;
  unsigned long lastHw; //last time the handwashing sensor is pressed
  bool hasHwSoap;
  unsigned long soapStamp; //time stamp when soap is pressed
  bool hasSoap;
  unsigned long lastActive;
  bool currentlyActive ; // 
  bool activity ; // check if any of the sensor show activity

  // for SD stuff

  File userFile;
  int userNumber = 0; 

  // thresholds

  const int irThreshold = 220;
  int soapThreshold = 50;
  const int flushThreshold = 650;

  //reset time 

  const int resetTime = 20000;

  // median filter for ir sensor
  MedianFilter filter = MedianFilter(9,0);
  
  User(int myNumber, int soapTHRESH){
    soapThreshold = soapTHRESH;
    userNumber = myNumber;
    flushStamp = 0;
    hasFlushed = false;
    initHw = 0;
    hasHw = false;
    lastHw = 0;
    soapStamp = 0;
    hasSoap = false;
    hasHwSoap = false;
    lastActive = 0;
    activity = false;

  };

  void readFromSensors(int flushPin, int handPin, int soapPin){
    activity = false;
    
    if(millis() > (lastActive + resetTime) && lastActive != 0){
      //reset user
      #ifdef SSDSHIELD
        userFile = SD.open("log" + String(userNumber) + ".txt", FILE_WRITE);
        if(userFile){// if opened sucessfully save data 
          userFile.println(String(flushStamp) + " " + String(initHw) + " " + String(lastHw) + " " + String(soapStamp));
        }
        userFile.close();
      #endif
      resetClass();
    }

    if((analogRead(flushPin) > flushThreshold) && !hasFlushed){
      hasFlushed = true;
      flushStamp = millis();
      activity = true;
    }

    if((analogRead(soapPin) < soapThreshold) && !hasSoap){
      hasSoap = true;
      soapStamp = millis();
      activity = true;
    }

    int distVal = analogRead(handPin);
    filter.in(distVal);
    distVal = filter.out();
    
    if(distVal > irThreshold){
      activity = true;
      if(!hasSoap && !hasHw ){
        hasHw = true;
        initHw = millis();
      }
      // if they used soap they are done
      if(hasSoap){
        //done complete
        hasHwSoap = true;
      }

      lastHw = millis();

    }else if(lastActive + 5000 < millis() && hasHwSoap){

      //save the users data 

      #ifdef SSDSHIELD
        userFile = SD.open("log" + String(userNumber) + ".txt", FILE_WRITE);
        if(userFile){// if opened sucessfully save data 
          userFile.println(String(flushStamp) + " " + String(initHw) + " " + String(lastHw) + " " + String(soapStamp));
        }
        userFile.close();
      #endif
      Serial.println("im resetting ");
      //reset user
      resetClass();
    }

    if(activity){
      lastActive = millis();
    }
    
  }

  void resetClass(){
    flushStamp = 0;
    hasFlushed = false;
    initHw = 0;
    hasHw = false;
    lastHw = 0;
    soapStamp = 0;
    hasSoap = false;
    hasHwSoap = false;
    lastActive = 0;
    activity = false;
    // also save shit here
  }
};


//user
User user[3] = { User(0,50),User(1,300),User(2,200)};


//class for doing SD shield related things 
File myFile;

void setup() {
  Serial.begin(9600);
  initInput();
  #ifdef SSDSHIELD
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
  #endif

  #ifdef DEBUG //prints only the information if debug mode is declared
  Serial.println("setup is done");
  #endif
}

void loop(){
// for each wired bathroom
  Serial.println(String(analogRead(A0)) + " " + String(analogRead(A1)) + " "  + String(analogRead(A2)) + "     " +String(analogRead(A3)) + " " + String(analogRead(A4)) + " " + String(analogRead(A5)) + "       " +String(analogRead(A6)) + " " + String(analogRead(A7)) + " "  + String(analogRead(A8)));
  for (uint8_t i = 0; i < 3; i++) {
    user[i].readFromSensors(flushPins[i],irPins[i],soapPins[i]);
  }  
  #ifdef DEBUG //prints only the information if debug mode is declared
  if(millis() > timer){
    for(int i = 0; i < 3; i++){
    Serial.print("for user " + String(i)+ " :  " + String(user[i].flushStamp)+ "  " + String(user[i].initHw) + "  " + String(user[i].lastHw) + "  " + String(user[i].soapStamp) + " "  + String(user[i].lastActive) +"             ");
    }
    Serial.println();
    timer = millis() + 2000;
  }
  #endif*/
}

void initInput()
{
  for(int i = 0; i < 3; i++){
    pinMode(soapPins[i], INPUT_PULLUP);
    pinMode(irPins[i], INPUT);
    pinMode(flushPins[i], INPUT_PULLUP);
  }
  
}




