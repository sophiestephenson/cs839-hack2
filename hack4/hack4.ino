#include <WiFiClientSecure.h>
#include <WiFi.h>

#include "Arduino.h"
#include "AudioTools.h"
#include "SPIFFS.h"
#include "arduinosecrets.h"

I2SStream in;  // Access I2S as a stream
File audioFile;  //empty audio file object

const char* ssid = WIFI_SSID;   //  your network SSID (name)
const char* pass = WIFI_PASS;  // your network password

const char* filename = "/audiofile.wav";

void setupWifi() {
  Serial.print("Connecting to Wifi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  } 
  Serial.println();
  Serial.println("Connected to Wifi");
}

void sendfile() {
  Serial.println("Attempting to send file...");

  if ((WiFi.status() == WL_CONNECTED)) {    
    audioFile = SPIFFS.open(filename);

    WiFiClientSecure *m_wifi_client = new WiFiClientSecure();
    m_wifi_client->connect("api.wit.ai", 443);
    m_wifi_client->println("POST /speech?v=20200927 HTTP/1.1");
    m_wifi_client->println("host: api.wit.ai");
    m_wifi_client->println("authorization: Bearer 6QVLPGAHBAV2AL55FZ4PDIXI2BYDLADR");
    m_wifi_client->println("content-type: audio/wav;");
    m_wifi_client->print("content-length: ");
    m_wifi_client->println(audioFile.size());
    m_wifi_client->println();
    m_wifi_client->write(audioFile);
    m_wifi_client->println();

    audioFile.close();

    Serial.println("File sent.");
  }
}

// Arduino Setup
void setup() {
  Serial.begin(115200);
  setupWifi();

  SPIFFS.begin();

  Serial.println("starting I2S...");
  auto config = in.defaultConfig(RX_MODE);
  config.i2s_format = I2S_STD_FORMAT;  // if quality is bad change to I2S_LSB_FORMAT #23
  config.sample_rate = 44100;
  config.channels = 1;
  config.bits_per_sample = 32;
  in.begin(config);
  Serial.println("I2S started");

  Serial.println("starting out stream...");
  EncodedAudioStream out(&audioFile, new WAVEncoder());  // Access audioFile as a stream
  auto cfg_out = out.defaultConfig();
  cfg_out.bits_per_sample = 32;
  cfg_out.channels = 1;
  cfg_out.sample_rate = 44100;
  out.begin(cfg_out);

  StreamCopy copier(out, in);  

  SPIFFS.remove(filename);
  audioFile = SPIFFS.open(filename, FILE_WRITE);  

  Serial.println("RECORDING");
  long starttime = millis();
  while (starttime + 2000 > millis())  // loop end after 2 sec
  {
    copier.copy();  // Copy sound from I2S to the audio file
  }
  audioFile.close();  // Closes the file after 5 seconds
  Serial.println("Stopped recording");

  // get file size
  audioFile = SPIFFS.open(filename, FILE_READ);
  size_t fileSize1 = audioFile.size();
  Serial.println("FILE SZE:" + String(fileSize1));
  audioFile.close();
  sendfile();
}

void loop() {
  Serial.println("Got to loop");
  delay(2000);
}