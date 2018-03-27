// IMPORTANT STATE READ WHAT THEY DO
//#define SSDSHIELD // If this is defined the code will assume the SSD SHIELD is attached if not it will not compile any logging into the code 
#define DEBUG // defining this makes the program print to the serial monitor, not defining it prevents the any printing functions from being complied into the code


#include <NewPing.h> // for sensors
#include <SPI.h> //communicate with the shield
#include <SD.h> // SD read and write functions
//sensor variables
#define SONAR_NUM     3 // Number or sensors.
#define MAX_DISTANCE 400 // Max distance in cm.
#define PING_INTERVAL 70   // Milliseconds between pings.

// baseline variables, (keep these high because the sensor is utter shite)
#define DYNAMICBASELINE false // wheather or not to adjust baseline on the fly(not implemented yet).
#define MINDISTANCE 30 // the smallest distance the sensor will accept when setting the baseline

#define RESETAMOUNT 1  //
#define BUFFER 5 //How far away from baseline data does a result have to be to not snap to the baseline



//flow varaibles
#define TIMEUNTILSTART 75; //how many milliseconds to wait before starting to meassure for the baseline


 
//sensor and pin stuff
#define TRIGGER_PIN0  2
#define ECHO_PIN0     5

#define TRIGGER_PIN1  6
#define ECHO_PIN1     7

#define TRIGGER_PIN2  8
#define ECHO_PIN2     9

#define LEDPIN  3

#define SOAP1   11
#define SOAP2   12
#define SOAP3   13

#define TIME_BETWEEN_SOAP 10


//number of pulses to take the average of
int numberOfCycles = 7; 
int currentCycle = 0;
int baselineCycles = 10;


//declare sensors
NewPing sonar[SONAR_NUM] = {
NewPing(TRIGGER_PIN0, ECHO_PIN0, MAX_DISTANCE),
NewPing(TRIGGER_PIN1, ECHO_PIN1, MAX_DISTANCE),
NewPing(TRIGGER_PIN2, ECHO_PIN2, MAX_DISTANCE)
};

unsigned long pingTimer[SONAR_NUM]; // When each pings.
int cm[SONAR_NUM]; // Store ping distances.
int baseline[SONAR_NUM]; // Store baseLine distances
int baselineSamples[SONAR_NUM]; // how many non erronus samples on each baseline
bool baselineSet[SONAR_NUM];
uint8_t currentSensor = 0; // Which sensor is active.

//file stuff
#ifdef SSDSHEILD
//create a file 
File myFile;
#endif

unsigned int peopleCounter[SONAR_NUM] = {0,0,0}; //counts nr hold people entering
unsigned int baselineCounter[SONAR_NUM] = {0,0,0};
unsigned long timeIn[SONAR_NUM];


void setup() {
  Serial.begin(115200);
  //delay(5000); //delay for 5 secs before starting to set the baseline
  #ifdef SSDSHIELD
  //Initialize SD card and Check if the SD card and libary failed
  Serial.print("Initializing SD card...");

  pinMode(SOAP1, INPUT_PULLUP);
  pinMode(SOAP2, INPUT_PULLUP);
  pinMode(SOAP3, INPUT_PULLUP);
  
  pinMode(3, OUTPUT);
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
  
  pingTimer[0] = millis() + TIMEUNTILSTART; // First ping start in ms.
  for (uint8_t i = 1; i < SONAR_NUM; i++){
  pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  baselineSet[SONAR_NUM] = false; //set all baselines as unset
  baselineSamples[SONAR_NUM] = 0;
  }  
}

int times1 = 0;
int times2 = 0;
int times3 = 0;
 
void loop() {
  for (uint8_t i = 0; i < SONAR_NUM; i++) {
    if (millis() >= pingTimer[i]) {
      pingTimer[i] += PING_INTERVAL * SONAR_NUM;
      if (i == 0 && currentSensor == SONAR_NUM - 1)
        oneSensorCycle(); // Do something with results.
      sonar[currentSensor].timer_stop();
      currentSensor = i;
      sonar[currentSensor].ping_timer(echoCheck);
    }
  }
  // The rest of your code would go here.
  soapPrint(digitalRead(SOAP1), "SOAP1", int times1);
  soapPrint(digitalRead(SOAP2), "SOAP2", int times2);
  soapPrint(digitalRead(SOAP3), "SOAP3", int times3);
}

void soapPrint(int digiRead, String soapTitle, int times)
{
  long curTime = millis()/1000;
  
  if(soapUsed(digiRead))
  {
    times = times++;
    
    #ifdef SSDSHIELD 
      myFile = SD.open(soapTitle + ".txt", FILE_WRITE);
      myFile.println(String(times) + " "+ String(curTime));
      myFile.close();
    #endif
  }
}


int lastSoapSensorInput = 0;
long lastTimeSoapInput = 0;
//Returns true if this is the first time the soap has been used in the last 10 sec
bool soapUsed(int sensorIn)
{
  long curTime = millis()/1000;

  //Serial.print(String(lastSoapSensorInput) + " , ");
  //Serial.print(String(sensorIn != lastSoapSensorInput) + " , ");
  //Serial.print(String(sensorIn == 1) + " , ");
  //Serial.println(curTime - lastTimeSoapInput > 10);
  
  if(sensorIn != lastSoapSensorInput && sensorIn == 1 && curTime - lastTimeSoapInput > TIME_BETWEEN_SOAP)
  {
    lastSoapSensorInput = sensorIn;
    lastTimeSoapInput = curTime;
  
    return true;
  }
  else
  {
    lastSoapSensorInput = sensorIn;
    
    return false;
  }   
}
 
void echoCheck() { // If ping echo, set distance to array.
  if (sonar[currentSensor].check_timer()){
    int dist = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM; // read the distance;
    Serial.print(String(dist) + " " + String(dist - baseline[currentSensor]));
    if(baselineSet[currentSensor]){
      if(dist == 0){dist = baseline[currentSensor];}
      //{baseline[currentSensor] = 0.97* baseline[currentSensor] + 0.03 * dist;}
    }else{
      if(dist < MINDISTANCE){ // 0 If we get zero that means that there are some issue also we dont wanna set base lines on thing that are too close to the sensor
        // if the result is erronus we are gonna need a new sample
        dist = 0;
      }else{
        baselineSamples[currentSensor]++; //if the sample is in the wanted range that is gud, count it when setting the baseline
      }
    }
    cm[currentSensor] += dist;
  }
}
 
void oneSensorCycle() { // Do something with the results.
  currentCycle++;
  for(uint8_t i = 0; i < SONAR_NUM; i++){
      //cycle through the sensor and deal with results
      //if the base line has not been set, and there are enough samples to check it check if there's enough samples to set it
      if(!baselineSet[i] && baselineSamples[i] >= baselineCycles){
        baselineSet[i] = true;
        baseline[i] = cm[i]/baselineSamples[i];
        Serial.println("SETTING SENSOR " + String(i) + " BASELINE AT:" + String(baseline[i]));
        Serial.print("cm ");
        Serial.print(i);
        cm[i] = 0;
      }else if(!baselineSet[i]){
        Serial.print("      ");
      }else if(currentCycle >= numberOfCycles){ //otherwise a baseline must have been set and we can calculate the values read on the sensor
        Serial.print(i);
        Serial.print("=" + String(cm[i]) +  " ");
        cm[i] = (cm[i]/currentCycle) - baseline[i];
        if(cm[i] > BUFFER){
          cm[i] -= BUFFER;
        }else if(cm[i] < -BUFFER){
         cm[i] += BUFFER;
        }else{
          cm[i] = 0;
        }
        if(cm[i] == 0){
          baselineCounter[i]++;
          if(baselineCounter[i] == RESETAMOUNT){
            digitalWrite(LEDPIN, LOW);
            long timeNow = millis()/1000;
            #ifdef SSDSHIELD 
            myFile = SD.open("sensor" + String(i)+".txt", FILE_WRITE);
            myFile.println(String(peopleCounter[i]) + " "+ String(timeIn[i]) + " " + String(timeNow));
            myFile.close();
            #endif
          }
        }else{
          if(baselineCounter[i] >= RESETAMOUNT){
            //somebody have entered the room
            peopleCounter[i]++;
            digitalWrite(LEDPIN, HIGH);
            timeIn[i] = millis()/1000;
          }
          //there must be somebody in the room
          baselineCounter[i] = 0;
        }
        
        
        Serial.print(cm[i]);
        Serial.print("cm ");
        cm[i] = 0;
      }
  }
  Serial.println();
  if(currentCycle >= numberOfCycles){
      currentCycle = 0;
  }
}  
