#define trigPin 13 // define TrigPin
#define echoPin 14 // define EchoPin.
#define buzzerPin 12 // define BuzzerPin.
#define MAX_DISTANCE 200 // Maximum sensor distance is rated at 400-500cm.

#include <WiFi.h>
#include <HTTPClient.h>

// STATE
const float DANGER_DIST = 25.0;
bool inDangerZone = false;
  
// SONAR STUFF
float timeOut = MAX_DISTANCE * 60;
int soundVelocity = 340; // define sound speed=340m/s

// WEB STUFF
const char* ssid = "Galaxy";
const char* password = "";
const int    HTTP_PORT   = 80;
const String HTTP_METHOD = "GET"; // or "POST"
const char   HOST_NAME[] = "google.com"; // hostname of web server:
const String PATH_NAME   = "";

const String IFTTT_URL = "http://maker.ifttt.com/trigger/sensor_status/with/key/bc3z7e6-EAtEmsTx3FrdgJ";

void setup() {
  Serial.begin(9600); // Open serial monitor at 9600 baud to see ping results.
  pinMode(trigPin,OUTPUT);// set trigPin to output mode
  pinMode(buzzerPin,OUTPUT); // Set Buzzer pin to output mode
  pinMode(echoPin,INPUT); // set echoPin to input mode

  // Set up WiFi
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  
  Serial.println("Connected to the WiFi network");
}

void loop() {
  float catDist = getSonar();
  printDist(catDist);

  if (catDist != 0) { // Avoid 0 issue
    
    if (catDist <= DANGER_DIST && !inDangerZone) {
      inDangerZone = true;
      startBuzzer();
      textOwner();
    }
    
    else if (catDist > DANGER_DIST && inDangerZone) {
      inDangerZone = false;
      stopBuzzer();
    }

  } 
  
  delay(100); // Wait 100ms between pings (about 20 pings/sec). 29ms should be the shortest delay between pings.
}


void startBuzzer() {
  int x = 0;
  float sinVal = sin(x * (PI / 180));
  int toneVal = 2000 + sinVal * 500;
  ledcAttachPin(buzzerPin, 0);
  tone(buzzerPin, toneVal);
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
}

void textOwner() {
  if ((WiFi.status() == WL_CONNECTED)) {
    HTTPClient http;
    http.begin(IFTTT_URL);
    http.GET();
    http.end();   
  }
}

void printDist(float distance) {
  Serial.print("Distance: ");
  Serial.print(distance); // Send ping, get distance in cm and print result (0 = outside set distance range)
  Serial.println("cm");
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
