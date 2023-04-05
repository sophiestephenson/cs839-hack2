/*
  modified on Sep 8, 2020
  Modified by MohammedDamirchi from Arduino Examples
  Home
*/


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  int sensorValue = analogRead(25);
  // print out the value you read:
  Serial.print(0);
  Serial.print(" ");
  Serial.print(4000);
  Serial.print(" ");
  Serial.println(sensorValue);
  }