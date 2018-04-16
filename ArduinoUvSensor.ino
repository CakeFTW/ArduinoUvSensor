// tasks left 
// make accelerometer woprk

#include <MedianFilter.h>

#include <SPI.h> //communicate with the shield
#include <SD.h> // SD read and write functions
// IMPORTANT STATE READ WHAT THEY DO
#define SSDSHIELD // If this is defined the code will assume the SSD SHIELD is attached if not it will not compile any logging into the code 
#define DEBUG // defining this makes the program print to the serial monitor, not defining it prevents the any printing functions from being complied into the code

#define userNr 3
#define windowSize 9// size to use for median filtering of the ir sensors
#define multiplexPin A0 // pin that the multiplexer is connected to 
int bitPin[4] = {4,5,6,7}; // pins used to which multiplexer channel to read from 
int pinTable[16][4]; // table to hold the states of bits need to acess a particulair channel
MedianFilter irFilter[3] = {MedianFilter(windowSize,0), MedianFilter(windowSize,0), MedianFilter(windowSize,0)};

//chanels for the different sensors
int accCh[3] ={0,1,2}; 
int irCh[3] ={3,4,5};
int preCh[3] ={6,7,8};

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



//sensor variables
#define IRBUFFER 120 // what reading mean that hands are in front of the sensor
#define ACCBUFFER 360 // what reading on the acc to count as flush

//flow varaibles
#define TIMEUNTILSTART 75; //how many milliseconds to wait before starting to meassure for the baseline


 

//user
User user[3] = { User(),User(),User()};

// meassurement variables
const byte filterWindowSize = 15; // how many readings to take median of
int reading[userNr]; //used to store all the readings
long baseline[userNr]; // Store baseLine distances
int baselineSamples[userNr];
bool baselineSet[userNr];
uint8_t currentSensor = 0; // Which sensor is active.

//person detection variables
unsigned long timeIn[userNr];

//declare sensors

//file stuff

//create a file 
File myFile;

void setup() {
  Serial.begin(9600);
  initMultiplex(); // prep the multiplexer stuff
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

void loop() {
// for each wired bathroom
  for (uint8_t i = 0; i < 1; i++) {
      // i is used to refer to the sensors attached to the current bathroom
      // start by checking if there is a flush, and if its been more than 12 seconds since the last flush

      if (isFlush(i) &&  millis() > user[i].flushStamp + 12000 ){
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
      
    #ifdef DEBUG //prints only the information if debug mode is declared
    Serial.println("for user " + String(i)+ " :  " + String(user[i].flushStamp)+ "  " + String(user[i].initHW) + "  " + String(user[i].lastHW) + "  " + String(user[i].soapStamp));
    #endif
  }  
}



int multiplexRead(int pin, bool usingPullUp){
  //setting the bit pins
  for(int i = 0; i < 4; i ++){
    if(pinTable[pin][i] != 0){
    digitalWrite(bitPin[i], HIGH);
    }else{
    digitalWrite(bitPin[i], LOW);
    }
  }
  // check if using pull up
  if(usingPullUp){
    pinMode(multiplexPin, INPUT_PULLUP);
  }else{
    pinMode(multiplexPin, INPUT);
  }
  delay(1); // give the arduino a msecond to account for the changeins in pull up
  return analogRead(multiplexPin); // read the pin an return the results
}

void initMultiplex(){
  for(int i = 0; i < 4; i++){
    pinMode(bitPin[i], OUTPUT);
  }
  for(int i = 0; i < 9; i++){
    int nr = i;
    if(nr - 8 >= 0){
      pinTable[i][3] = 1;
      nr -= 8;
    }
    if(nr - 4 >= 0){
      pinTable[i][2] = 1;
      nr -= 4;
    }
    if(nr - 2 >= 0){
      pinTable[i][1] = 1;
      nr -= 2;
    }
    if(nr - 1 >= 0){
      pinTable[i][0] = 1;
      nr -= 1;
    }
    //Serial.println("for " + String(i) + " : " + String(pinTable[i][3]) + " " + String(pinTable[i][2]) + " " + String(pinTable[i][1]) + " " + String(pinTable[i][0]));
  }
}


bool soapPressure(int nr)
{ 
  int sensorValue = multiplexRead(preCh[nr], true);
  if(sensorValue < 100){return true;}
  return false;
}

bool irDist(int nr){
  int sensorValue = multiplexRead(irCh[nr], false);
  irFilter[nr].in(sensorValue);
  sensorValue = irFilter[nr].out();
  if(sensorValue > IRBUFFER){return true;}
  return false;
}

bool isFlush(int nr){
  int sensorValue = multiplexRead(accCh[nr], false);
  if(sensorValue > ACCBUFFER){return true;}
  return false;
}



