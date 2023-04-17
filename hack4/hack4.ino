#include <WiFiClientSecure.h>
#include <WiFi.h>

#include "Arduino.h"
#include "AudioTools.h"
#include "SPIFFS.h"
#include "arduinosecrets.h"
#include "WAVFileWriter.h"

I2SStream in;  // Access I2S as a stream

const char* ssid = WIFI_SSID;   //  your network SSID (name)
const char* pass = WIFI_PASS;  // your network password

// Arduino Setup
void setup() {
  Serial.begin(115200);
  
  // Set up wifi
  Serial.print("Connecting to Wifi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  } 
  Serial.println();
  Serial.println("Connected to Wifi");
  
  SPIFFS.begin();

  //Serial.println("starting out stream...");
  //WAVEncoder *wavEncoder = new WAVEncoder();
  //EncodedAudioStream out(&audioFile, wavEncoder);  // Access audioFile as a stream
  //StreamCopy copier(out, in); 

  Serial.println("starting I2S...");
  auto config = in.defaultConfig(RX_MODE);
  config.i2s_format = I2S_STD_FORMAT;  // if quality is bad change to I2S_LSB_FORMAT #23
  config.sample_rate = 16000;
  config.channels = 2;
  config.bits_per_sample = 16;
  in.begin(config);
  Serial.println("I2S started");

  SPIFFS.remove("/audiofile.wav");

  File audioFile = SPIFFS.open("/audiofile.wav", FILE_WRITE); 
  EncodedAudioStream out(&audioFile, new WAVEncoder());
  auto cfg_out = out.defaultConfig();
  cfg_out.bits_per_sample = 32;
  cfg_out.channels = 1;
  cfg_out.sample_rate = 44100;
  out.begin(cfg_out);

  StreamCopy copier(out, in);  

  Serial.println("RECORDING");
  long starttime = millis();
  while (starttime + 2000 > millis())  // loop end after 2 sec
  {
    copier.copy();  // Copy sound from I2S to the audio file
  }
  copier.end();
  in.end();
  out.end();
  Serial.println("Stopped recording");

  // Send the file
  Serial.println("Attempting to send file...");

  if ((WiFi.status() == WL_CONNECTED)) {    
    //audioFile = SPIFFS.open("/audiofile.wav", FILE_READ);

    WiFiClientSecure m_wifi_client; // = new WiFiClientSecure();
    m_wifi_client.connect("api.wit.ai", 443);
    m_wifi_client.println("POST /speech?v=20200927 HTTP/1.1");
    m_wifi_client.println("host: api.wit.ai");
    m_wifi_client.println("authorization: Bearer 6QVLPGAHBAV2AL55FZ4PDIXI2BYDLADR");
    m_wifi_client.println("content-type: audio/wav;");
    m_wifi_client.print("content-length: ");
    m_wifi_client.println(audioFile.size());
    m_wifi_client.println();
    m_wifi_client.write(audioFile);
    m_wifi_client.println();
    m_wifi_client.stop();
    //audioFile.close();
  }
  audioFile.close();
}

void loop() {
  Serial.println("Got to loop");
  delay(2000);
}