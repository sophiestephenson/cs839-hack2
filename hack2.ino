#define trigPin 13        // define TrigPin
#define echoPin 14        // define EchoPin.
#define buzzerPin 12      // define BuzzerPin.
#define MAX_DISTANCE 200  // Maximum sensor distance is rated at 400-500cm.
#define recvPin 15        // define ReceiverPin for IR remote.

#include <WiFi.h>
#include <HTTPClient.h>
#include "arduinosecrets.h"
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>

// CONFIG
const float DANGER_DIST = 25.0;
const bool doIFTTT = true;

// STATE
bool danger = false;
bool email = true;
bool asleep = false;
const float WARNING_PERIOD = 10 * 1000;

// SONAR STUFF
float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340;  // define sound speed=340m/s

// IR STUFF
IRrecv irrecv(recvPin);  // Create a class object used to receive class
decode_results results;  // Create a decoding results class object

const unsigned long buzzerOffButton = 0xFF30CF;  // the 1 button
const unsigned long sleepButton = 0xFF18E7;      // the 1 button
const unsigned long awayButton = 0xFF7A85;       // the 1 button
const unsigned long warningButton = 0xFF10EF;    // the 1 button

// WEB STUFF
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASS;
const int HTTP_PORT = 80;
const String HTTP_METHOD = "GET";       // or "POST"
const char HOST_NAME[] = "google.com";  // hostname of web server:
const String PATH_NAME = "";

const String IFTTT_URL = "http://maker.ifttt.com/trigger/sensor_status/with/key/bc3z7e6-EAtEmsTx3FrdgJ";

void setup() {
  Serial.begin(9600);          // Open serial monitor at 9600 baud to see ping results.
  pinMode(trigPin, OUTPUT);    // set trigPin to output mode
  pinMode(buzzerPin, OUTPUT);  // Set Buzzer pin to output mode
  pinMode(echoPin, INPUT);     // set echoPin to input mode

  irrecv.enableIRIn(); // Start the IR receiver

  if (doIFTTT) {
    // Set up WiFi
    WiFi.begin(ssid, password);

    Serial.print("Connecting to WiFi...");
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.print(".");
    }

    Serial.println();
    Serial.println("Connected to the WiFi network");
  }
}

void loop() {
  float catDist = getSonar();
  printDist(catDist);

  unsigned long buttonPressed = getButtonPressed();
  switch (buttonPressed) {
    case buzzerOffButton:
      Serial.println("-----TURNING BUZZER OFF");
      stopBuzzer();
      // Don't turn danger off, because that will make it trigger the buzzer immediately
      break;

    case sleepButton:
      asleep = !asleep;
      email = !asleep; // email should be the opposite of asleep - if you're asleep, turn on emails
      Serial.print("-----SLEEP = ");
      Serial.println(asleep);
      break;

    case awayButton:
      email = !email;
      Serial.print("-----EMAIL = ");
      Serial.println(email);
      break;

    case warningButton:
      stopBuzzer();
      danger = false;
      Serial.println("-----PAUSING BUZZER FOR 10 SECONDS");
      delay(WARNING_PERIOD);
      break;

    default:
      //just do normal stuff

      // Avoid 0 issue - 0 means it's past the max distance
      if (catDist == 0) {  
        catDist = MAX_DISTANCE;
      }

      // turn the buzzer on
      if (catDist <= DANGER_DIST && !danger) {
        if (!asleep) {
          danger = true;
          startBuzzer();
        }
        if (email && doIFTTT) {
          emailOwner();
        }
      }

      // turn the buzzer off
      else if (catDist > DANGER_DIST && danger) {
        danger = false;
        stopBuzzer();
      }
  }
  delay(100);  // Wait 100ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
}


void startBuzzer() {
  int x = 0;
  float sinVal = sin(x * (PI / 180));
  int toneVal = 2000 + sinVal * 500;
  ledcAttachPin(buzzerPin, 0);
  tone(buzzerPin, toneVal);
  Serial.println("BUZZER ON");
}

void setBuzzerTone(float catDist) {
  long x = map(catDist, 0, 25, 360, 0);
  float sinVal = sin(x * (PI / 180));
  int toneVal = 2000 + sinVal * 500;
  tone(buzzerPin, toneVal);
  Serial.print(toneVal);
}

void stopBuzzer() {
  ledcDetachPin(buzzerPin);
  Serial.println("BUZZER OFF");
}

void emailOwner() {
  if ((WiFi.status() == WL_CONNECTED)) {
    //HTTPClient http;
    //http.begin(IFTTT_URL);
    //http.GET();
    //http.end();
  }
  Serial.println("EMAIL SENT");
}

void printDist(float distance) {
  Serial.print("Distance: ");
  Serial.print(distance);  // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.println("cm");
}

float getSonar() {
  unsigned long pingTime;
  float distance;
  digitalWrite(trigPin, HIGH);  // make trigPin output high level lasting for 10μs to triger HC_SR04,
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  pingTime = pulseIn(echoPin, HIGH, timeOut);              // Wait HC-SR04 returning to the high level and measure out this waitting time
  distance = (float)pingTime * soundVelocity / 2 / 10000;  // calculate the distance according to the time
  return distance;                                         // return the distance value
}

unsigned long getButtonPressed() {
  unsigned long val = 0;

  if (irrecv.decode(&results)) {  // Waiting for decoding
    val = results.value;
    irrecv.resume();  // Receive the next value
  }

  return val;
}
