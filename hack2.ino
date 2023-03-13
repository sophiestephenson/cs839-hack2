#define trigPin 13 // define TrigPin
#define echoPin 14 // define EchoPin.
#define buzzerPin 12 // define BuzzerPin.
#define recvPin 15 // define ReceiverPin for IR remote.
#define SDA 32
#define SCL 33
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
const int period = 100; // each window is 1/10 of a second
const unsigned long remoteTrigger = 0xFF30CF; // the 1 button

// STATE
const float DANGER_DIST = 25.0;
bool inDangerZone = false;

// DATA COLLECTION
long timer = 0;
float sumDistInPeriod = 0;
int numDistReadings = 0;
float sumAccInPeriod = 0;
int numAccReadings = 0;
bool remoteTriggeredInPeriod = false;

// IR STUFF
IRrecv irrecv(recvPin); // Create a class object used to receive class
decode_results results; // Create a decoding results class object

// ACC STUFF
MPU6050 mpu6050(Wire);//Attach the IIC
int16_t ax,ay,az;//define acceleration values of 3 axes
  
// SONAR STUFF
float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340; // define sound speed=340m/s


void setup() {
  Serial.begin(9600); // Open serial monitor at 9600 baud to see ping results.
  pinMode(trigPin,OUTPUT);// set trigPin to output mode
  pinMode(buzzerPin,OUTPUT); // Set Buzzer pin to output mode
  pinMode(echoPin,INPUT); // set echoPin to input mode

  irrecv.enableIRIn(); // Start the IR receiver

  Wire.begin(SDA, SCL); //attach the IIC pin
  mpu6050.begin(); //initialize the MPU6050
  mpu6050.calcGyroOffsets(true); //get the offsets value

  if (doIFTTT) {
    setUpWifi();
  }
}

void loop() {
  // Data capture
  float catDist = getSonar();
  sumDistInPeriod += catDist;
  numDistReadings++;

  float catAcc = getAcc();
  sumAccInPeriod += catAcc;
  numAccReadings++;

  if (remoteTriggered()) {
    remoteTriggeredInPeriod = true;
  }

  // Send data every period
  if (millis() - timer > period) {

    // Calculate average over the window & send data
    float avgDist = getAvg(sumDistInPeriod, numDistReadings);
    float avgAcc = getAvg(sumAccInPeriod, numAccReadings);
    sendData(avgDist, avgAcc, remoteTriggeredInPeriod);
    
    // Reset
    float sumDistInPeriod = 0;
    float numDistReadings = 0;
    float sumAccInPeriod = 0;
    float numAccReadings = 0;
    bool remoteTriggeredInPeriod = false;

    timer = millis()
  }

  // Buzzer logic
  if (catDist != 0) { // Avoid 0 issue
    
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
  
  // delay(100); // Wait 100ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
}

void sendData(float avgDist, float avgAcc, bool classification) {
  Serial.print(millis());
  Serial.print(",");
  Serial.print(avgDist);
  Serial.print(",");
  Serial.print(avgAcc);
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

bool remoteTriggered() {
  unsigned long val = 0;

  if (irrecv.decode(&results)) { // Waiting for decoding
    val = results.value;
    irrecv.resume(); // Receive the next value
  }
  
  return (results.value == remoteTrigger);
}

float getAcc() {
  mpu6050.update();
  ax=mpu6050.getRawAccX();//gain the values of X axis acceleration raw data
  ay=mpu6050.getRawAccY();//gain the values of Y axis acceleration raw data
  az=mpu6050.getRawAccZ();//gain the values of Z axis acceleration raw data
  return (ax * ay * az) / 3;
}
}
