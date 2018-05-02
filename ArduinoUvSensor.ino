#include <MedianFilter.h>
#include <SPI.h> //communicate with the shield
#include <SD.h> // SD read and write functions
// IMPORTANT STATE READ WHAT THEY DO
#define SSDSHIELD // If this is defined the code will assume the SSD SHIELD is attached if not it will not compile any logging into the code 
//#define DEBUG // defining this makes the program print to the serial monitor, not defining it prevents the any printing functions from being complied into the code

//sensor variables
#define userNr 3
#define windowSize 9 // size to use for median filtering of the ir sensors 
#define windowSizeAcc 3 // filter size to use for the accelerometer 

#define IRBUFFER 120 // what reading mean that hands are in front of the sensor
#define ACCBUFFER 360 // what reading on the acc to count as flush
#define FLUSHBUFFER 900

//filters for the noisy transducers Inferred and Accelerometer
MedianFilter irFilter[3] = {MedianFilter(windowSize,0), MedianFilter(windowSize,0), MedianFilter(windowSize,0)};
//MedianFilter accFilter[3] = {MedianFilter(windowSizeAcc,0), MedianFilter(windowSizeAcc,0), MedianFilter(windowSizeAcc,0)};

//chanels for the different sensors
int irCh[3] ={A0,A1,A2};
int preCh[3] ={A3,A4,A5};
int accCh[3] ={A6,A7,A8};

//accelerometer controls
int prevAccReading[3] = {300,300,300}; // holds the last accelerometer reading for comparison
int accBuf[3] = {60,60,60}; // how much difference between readings to count as lever movement, controls how sensitive the accelerometer is try to stay above 3 if possible


//class for holding the user data
class User{
  public:
  unsigned long flushStamp; // time stamp for when the flush is pressed
  unsigned long initHW; //initial handwashing time
  unsigned long lastHW; //last time the handwashing sensor is pressed
  unsigned long soapStamp; //time stamp when soap is pressed
  User::User(){
  flushStamp = 0;
  initHW = 0;
  lastHW = 0;
  soapStamp = 0;
  };
};


//user
User user[3] = { User(),User(),User()};
//person detection variables
unsigned long timeIn[userNr];
unsigned long timer = 0;
int sensorValue;


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
/*
for(int i = 0 ; i < 3 ; i++){
  Serial.print( user[i].flushStamp);
  Serial.print( "    ") ;
}
Serial.println();*/
  for (uint8_t i = 0; i < 3; i++) {
      // i is used to refer to the sensors attached to the current bathroom
      // start by checking if there is a flush, and if its been more than 12 seconds since the last flush
      //  
      if (isFlush(i) && millis() > user[i].flushStamp + 12000){
        //if it has it must be a new user in, open the correct file
        #ifdef SSDSHIELD
        myFile = SD.open("log" + String(i) + ".txt", FILE_WRITE);
        if(myFile){// if opened sucessfully save data 
          myFile.println(String(user[i].flushStamp) + " " + String(user[i].initHW) + " " + String(user[i].lastHW) + " " + String(user[i].soapStamp));
        }
        myFile.close();
        #endif
        user[i] = User();
        user[i].flushStamp = millis(); // and that user flushed just now
      }
      // check the proximity sensor on the tabs
      if(irDist(i)){
        if(user[i].initHW == 0){
         user[i].initHW = millis();
        }
        user[i].lastHW = millis();
      }
      // Check the soap 
      if(soapPressure(i) && user[i].soapStamp == 0){
        user[i].soapStamp = millis();
      }
  }  
  #ifdef DEBUG //prints only the information if debug mode is declared
  if(millis() > timer){
    for(int i = 0; i < 3; i++){
    Serial.print("for user " + String(i)+ " :  " + String(user[i].flushStamp)+ "  " + String(user[i].initHW) + "  " + String(user[i].lastHW) + "  " + String(user[i].soapStamp) + "             ");
    }
    Serial.println();
    timer = millis() + 2000;
  }
  #endif*/
}

void initInput()
{
  for(int i = 0; i < 3; i++){
    pinMode(preCh[i], INPUT_PULLUP);
    pinMode(irCh[i], INPUT);
    pinMode(accCh[i], INPUT);
  }
  
}

bool soapPressure(int nr)
{ 
  sensorValue = analogRead(preCh[nr]);
  if(sensorValue < 250){return true;}
  return false;
}

bool irDist(int nr)
{
  sensorValue = analogRead(irCh[nr]);
  irFilter[nr].in(sensorValue);
  sensorValue = irFilter[nr].out();
  if(sensorValue > IRBUFFER){return true;}
  return false;
}

bool isFlush(int nr)
{
  sensorValue = analogRead(accCh[nr]);
  if(sensorValue > FLUSHBUFFER){return true;}
  return false;
}




