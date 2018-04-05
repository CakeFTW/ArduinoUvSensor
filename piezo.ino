const int led = 13;
const int threshold = 20;

//http://www.robotoid.com/appnotes/sensors-piezo-disc-touch-bar.html

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int disc = analogRead(A0);
  Serial.println(disc);  
  if(disc>threshold)
    flashLed();
  delay(10);
}

void flashLed() {
  digitalWrite(led, HIGH);
  delay(50);
  digitalWrite(led, LOW);
}
