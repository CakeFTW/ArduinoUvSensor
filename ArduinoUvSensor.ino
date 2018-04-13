#include <MedianFilter.h>
#include <SharpDistSensor.h>
#include <SPI.h> //communicate with the shield
#include <SD.h> // SD read and write functions
// IMPORTANT STATE READ WHAT THEY DO
#define SSDSHIELD // If this is defined the code will assume the SSD SHIELD is attached if not it will not compile any logging into the code 
#define DEBUG // defining this makes the program print to the serial monitor, not defining it prevents the any printing functions from being complied into the code

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
#define IR_NUM     3 // Number or sensors.
#define MAX_DISTANCE 300 // Max distance in cm.
#define IRBUFFER 120 // what nr to read on the ir sensor to count as person, higher values makes the distance short exponentially

// baseline variables, (keep these high because the sensor is utter shite)
#define DYNAMICBASELINE false // wheather or not to adjust baseline on the fly(not implemented yet).
#define SAMPLESFORBASELINE 50

#define BUFFER 5 //How far away from baseline data does a result have to be to not snap to the baseline



//flow varaibles
#define TIMEUNTILSTART 75; //how many milliseconds to wait before starting to meassure for the baseline

 
//sensor and pin stuff
#define IR_PIN0  A0
#define IR_PIN1  A1
#define IR_PIN2  A2 

#define ACC_PIN0  0
#define ACC_PIN1  1
#define ACC_PIN2  2

#define PRE_PIN0 A3
#define PRE_PIN1 A4
#define PRE_PIN2 A5
 

//user
User user[3] = { User(),User(),User()};
int accPin[3] = {ACC_PIN0 , ACC_PIN1, ACC_PIN2};
int prePin[3] = {PRE_PIN0 , PRE_PIN1, PRE_PIN2};

// meassurement variables
const byte filterWindowSize = 15; // how many readings to take median of
int reading[IR_NUM]; //used to store all the readings
long baseline[IR_NUM]; // Store baseLine distances
int baselineSamples[IR_NUM];
bool baselineSet[IR_NUM];
uint8_t currentSensor = 0; // Which sensor is active.

//person detection variables
unsigned int peopleCounter[IR_NUM] = {0,0,0}; //counts nr hold people entering
unsigned int baselineCounter[IR_NUM] = {0,0,0};
unsigned long timeIn[IR_NUM];

//declare sensors
SharpDistSensor irSensor[IR_NUM] = {
SharpDistSensor(IR_PIN0, filterWindowSize),
SharpDistSensor(IR_PIN1, filterWindowSize),
SharpDistSensor(IR_PIN2, filterWindowSize)};

//file stuff

//create a file 
File myFile;




void setup() {
  Serial.begin(9600);
  
  // set the model of the ir sensors
  for(int i = 0; i < IR_NUM ; i++){
      irSensor[i].setModel(SharpDistSensor::GP2Y0A710K0F_5V_DS);
      baselineSet[i] = false; //set all baselines as unset
      baselineSamples[i] = 0;
      pinMode(prePin[i], INPUT);
  }
  delay(2000); //delay for 5 secs before starting to set the baseline
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
  // check for flush
  // if flushed save user data then clear it

  // ir distance reading
  for (uint8_t i = 0; i < 1; i++) {
      
      currentSensor = i;

      // start by checking if there is a flush, and if its been more than 12 seconds since the last flush
      // marc's code goes
      
      if (accReading >=  1 &&  millis() > user[i].flushStamp + 12000 ){
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
      reading[currentSensor] = irSensor[currentSensor].getDist();
      if(reading[i] > IRBUFFER){
        if(user[i].initHW == 0){
         user[i].initHW = millis();
        }
      user[i].lastHW = millis();
      }
      // Check the soap
      // kevin code goes 
      int presReading = analogRead(prePin[i]);
      if(presReading != 0 && user[i].soapStamp == 0){
        user[i].soapStamp = millis();
      }
      
    #ifdef DEBUG //prints only the information if debug mode is declared
    //Serial.println("for user " + String(i)+ " :  " + String(user[i].flushStamp)+ "  " + String(user[i].initHW) + "  " + String(user[i].lastHW) + "  " + String(user[i].soapStamp));
    Serial.println(digitalRead(0));
    if(digitalRead(0) == 1){
      delay(2000);
    }
    #endif
  }  
  // The rest of your code would go here.

}

