#define trigPin 13 // define TrigPin
#define echoPin 14 // define EchoPin.
#define buzzerPin 12 // define BuzzerPin.
#define recvPin 15 // define ReceiverPin for IR remote.
#define MAX_DISTANCE 200 // Maximum sensor distance is rated at 400-500cm.

#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

// CONFIG
const bool doIFTTT = false; // notify the owner?
const int period = 200; // each window is 1/5 of a second
const int ultrasonicMin = 100; // 100 ms min between ultrasonic readings
const unsigned long remoteTrigger = 0xFF30CF; // the 1 button

// STATE
const float DANGER_DIST = 25.0;
bool inDangerZone = false;

// DATA COLLECTION
long timer = 0;
long ultrasonicTimer = 0;
float sumDistInPeriod = 0;
int numDistReadings = 0;
bool stoppedInPeriod = false;
bool startedInPeriod = false;

// IR STUFF
IRrecv irrecv(recvPin); // Create a class object used to receive class
decode_results results; // Create a decoding results class object
  
// SONAR STUFF
float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340; // define sound speed=340m/s


void setup() {
  Serial.begin(9600); // Open serial monitor at 9600 baud to see ping results.
  pinMode(trigPin,OUTPUT);// set trigPin to output mode
  pinMode(buzzerPin,OUTPUT); // Set Buzzer pin to output mode
  pinMode(echoPin,INPUT); // set echoPin to input mode

  irrecv.enableIRIn(); // Start the IR receiver

  if (doIFTTT) {
    setUpWifi();
  }
}

void loop() {
  
  // avoid too quick ultrasonic measurements
  if (millis() - ultrasonicTimer > ultrasonicMin) {
    // Data capture
    float catDist = getSonar();
    sumDistInPeriod += catDist;
    numDistReadings++;

    handleBuzzer(catDist);

    ultrasonicTimer = millis();
  }

  String remote = remoteTriggered();
  if (remote == "start") {
    startedInPeriod = true;
  }
  else if (remote == "stop") {
    stoppedInPeriod = true;
  }

  // Send data every period
  if (millis() - timer > period) {

    // Calculate average over the window & send data
    float avgDist = getAvg(sumDistInPeriod, numDistReadings);  
    sendData(avgDist, 1, startedInPeriod, stoppedInPeriod);
    
    // Reset
    sumDistInPeriod = 0;
    numDistReadings = 0;
    startedInPeriod = false;
    stoppedInPeriod = false;

    timer = millis();
  }
}

bool isInDangerZone() {
  // read prediction from the python
  if(Serial.available() > 0) {
    int prediction = Serial.parseInt();
    Serial.print("Prediction:");
    Serial.println(prediction);
    return (prediction == 1);
  } 

  return false;
}

void handleBuzzer(float catDist) {
    // Buzzer logic
    if (catDist != 0) { // Avoid 0 issue

      inDangerZone = isInDangerZone();
      
      if (catDist <= DANGER_DIST && !inDangerZone) {
        inDangerZone = true;
        startBuzzer();
        if (doIFTTT) {
          notifyOwner();
        }
      }
      
      else if (catDist > DANGER_DIST && inDangerZone) {
        inDangerZone = false;
        stopBuzzer();
      }
    } 
}

void sendData(float avgDist, float avgAcc, bool startedInPeriod, bool stoppedInPeriod) {

  String classification = "-";
  if (startedInPeriod) {
    classification = "start";
  }
  else if (stoppedInPeriod) {
    classification = "stop";
  }

  Serial.print(millis());
  Serial.print(",");
  Serial.print(avgDist);
  Serial.print(",");
  Serial.println(classification);
}

void notifyOwner() {
  const String IFTTT_URL = "http://maker.ifttt.com/trigger/sensor_status/with/key/bc3z7e6-EAtEmsTx3FrdgJ";
  
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(IFTTT_URL);
    http.GET();
    http.end();   
  }
}

void setUpWifi() {
  const char* ssid = "Galaxy";
  const char* password = "";

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
}

float getAvg(float sumReadings, int numReadings) {
  if (numReadings > 0) {
    return sumReadings / numReadings;
  }
  return 0;
}

// ------ SENSOR HANDLING --------

void startBuzzer() {
  int x = 0;
  float sinVal = sin(x * (PI / 180));
  int toneVal = 2000 + sinVal * 500;
  ledcAttachPin(buzzerPin, 0);
  tone(buzzerPin, toneVal);
}

void stopBuzzer() {
  ledcDetachPin(buzzerPin);
}

float getSonar() {
  unsigned long pingTime;
  float distance;
  digitalWrite(trigPin, HIGH); // make trigPin output high level lasting for 10μs to triger HC_SR04,
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  pingTime = pulseIn(echoPin, HIGH, timeOut); // Wait HC-SR04 returning to the high level and measure out this waitting time
  distance = (float)pingTime * soundVelocity / 2 / 10000; // calculate the distance according to the time
  return distance; // return the distance value
}

String remoteTriggered() {
  unsigned long val = 0;

  if (irrecv.decode(&results)) { // Waiting for decoding
    val = results.value;
    irrecv.resume(); // Receive the next value
  }

  if (val == 0xFF02FD)
    return "start";

  if (val == 0xFF9867)
    return "stop";

  return "";
}

