#include <NewPing.h>
 //sensor 0
#define TRIGGER_PIN0  1
#define ECHO_PIN0     3

#define TRIGGER_PIN1  2
#define ECHO_PIN1     5

#define TRIGGER_PIN2  7
#define ECHO_PIN2     6

NewPing sonar[3] = {
NewPing(TRIGGER_PIN0, ECHO_PIN0),
NewPing(TRIGGER_PIN1, ECHO_PIN1),
NewPing(TRIGGER_PIN2, ECHO_PIN2)
};
 
void setup() {
  Serial.begin(115200);
}
 
void loop() {
  //ping each sensor
  for(int i = 2; i < 3; i++){
    delay(500);
    Serial.print("Ping" + String(i) + ": ");
    Serial.print(sonar[i].ping_cm());
    Serial.println("cm");
  }
}
