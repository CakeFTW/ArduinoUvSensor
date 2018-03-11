#include <NewPing.h> // for sensors
#include <SPI.h> //communicate with the shield
#include <SD.h> // SD read and write functions
//sensor variables
#define SONAR_NUM     3 // Number or sensors.
#define MAX_DISTANCE 400 // Max distance in cm.
#define PING_INTERVAL 33 // Milliseconds between pings.

// baseline variables, (keep these high because the sensor is utter shite)
#define BUFFER 10 //How far away from baseline data does a result have to be to not snap to the baseline
#define DYNAMICBASELINE false // wheather or not to adjust baseline on the fly(not implemented yet).
#define MINDISTANCE 30

//flow varaibles
#define TIMEUNTILSTART 75; //how many milliseconds to wait before starting to meassure for the baseline


 
//sensor and pin stuff
#define TRIGGER_PIN0  2
#define ECHO_PIN0     5

#define TRIGGER_PIN1  6
#define ECHO_PIN1     7

#define TRIGGER_PIN2  8
#define ECHO_PIN2     9


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
//create a file 
File myFile;



void setup() {
  Serial.begin(115200);
  //delay(5000); //delay for 5 secs before starting to set the baseline
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
  pingTimer[0] = millis() + TIMEUNTILSTART; // First ping start in ms.
  for (uint8_t i = 1; i < SONAR_NUM; i++){
  pingTimer[i] = pingTimer[i - 1] + PING_INTERVAL;
  baselineSet[SONAR_NUM] = false; //set all baselines as unset
  baselineSamples[SONAR_NUM] = 0;
  }  
}
 
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
}
 
void echoCheck() { // If ping echo, set distance to array.
  if (sonar[currentSensor].check_timer()){
    int dist = sonar[currentSensor].ping_result / US_ROUNDTRIP_CM; // read the distance;
    
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
