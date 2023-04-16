#include <WiFiClientSecure.h>
#include <esp_crt_bundle.h>
#include <ssl_client.h>

#include <SPI.h>
#include "Arduino.h"
#include "AudioTools.h"
#include "SPIFFS.h"

#define PIN_AUDIO_KIT_SD_CARD_CS 5
#define PIN_AUDIO_KIT_SD_CARD_MISO 19
#define PIN_AUDIO_KIT_SD_CARD_MOSI 23
#define PIN_AUDIO_KIT_SD_CARD_CLK 18
const uint16_t sample_rate = 44100;

bool recording = false;
unsigned long runtime;

I2SStream in;  // Access I2S as a stream
//AnalogAudioStream in;
File audioFile;  //empty audio file object

char ssid[] = "TP-Link_506D";   //  your network SSID (name)
char pass[] = "56709179";  // your network password
int status = WL_IDLE_STATUS;
// Initialize the client library
WiFiClientSecure client;

/*
void setupWifi() {
  Serial.println("Attempting to connect to WPA network...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  status = WiFi.begin(ssid, pass);
  if (status != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  } else {
    Serial.println("Connected to wifi");
  }
}

void sendfile() {
  const String url = "http://maker.ifttt.com/trigger/sensor_status/with/key/bc3z7e6-EAtEmsTx3FrdgJ";

  if ((WiFi.status() == WL_CONNECTED)) {
    client.setInsecure();
    if (!client.connect(url)) {
      Serial.println("Connection failed");
    } else {
      client.println("POST " + url + " HTTP/1.0");
      client.println(F("User-Agent: ESP"));
      client.println(F("Content-Type: application/x-www-form-urlencoded;"));
      File file = SPIFFS.open("/AudioFile01.wav");
      client.print("Content-Length: ");
      client.println(file.size());
      client.write(file);
    }
  }
}
*/

// Arduino Setup
void setup(void) {
  Serial.begin(115200);
  //setupWifi();

  AudioLogger::instance().begin(Serial, AudioLogger::Debug);  // AudioLogger set to Debug mode

  Serial.println("SERIAL STARTED");
  SPI.begin(PIN_AUDIO_KIT_SD_CARD_CLK, PIN_AUDIO_KIT_SD_CARD_MISO, PIN_AUDIO_KIT_SD_CARD_MOSI, PIN_AUDIO_KIT_SD_CARD_CS);

  if (!SPIFFS.begin(PIN_AUDIO_KIT_SD_CARD_CS)) {

    Serial.println("SPIFFS FAILED");
  }

  // configure I2S stream
  Serial.println("starting I2S...");

  EncodedAudioStream out(&audioFile, new WAVEncoder());  // Access audioFile as a stream
  StreamCopy copier(out, in);                            // copies sound from i2s to audio file stream

  // RX automatically uses port 0 with pin GPIO34
  // auto cfgRx = in.defaultConfig(RX_MODE);
  // cfgRx.sample_rate = sample_rate;
  // in.begin(cfgRx);

  Serial.println("starting I2S...");
  auto config = in.defaultConfig(RX_MODE);
  config.i2s_format = I2S_STD_FORMAT;  // if quality is bad change to I2S_LSB_FORMAT #23
  config.sample_rate = 44100;
  config.channels = 1;
  config.bits_per_sample = 32;
  in.begin(config);
  Serial.println("I2S started");

  // Configure out stream
  auto cfg_out = out.defaultConfig();
  cfg_out.bits_per_sample = 32;
  cfg_out.channels = 1;
  cfg_out.sample_rate = 44100;
  out.begin(cfg_out);
  //copier.setCheckAvailableForWrite(false);

  SPIFFS.remove("/AudioFile01.wav");
  Serial.println("FILE open");
  File file = SPIFFS.open("/AudioFile01.wav", FILE_READ);
  size_t fileSize = file.size();
  Serial.println("FILE SZE:" + String(fileSize));
  file.close();
  audioFile = SPIFFS.open("/AudioFile01.wav", FILE_WRITE);  // Open a new file on the SD card for writing to
  copier.begin(out, in);                                    // Configure copier input and output
  Serial.println("RECORDING");
  while (runtime < 2000)  // loop end after 30 sec
  {
    runtime = millis();
    copier.copy();  // Copy sound from I2S to the audio file
  }
  audioFile.close();  // Closes the file after 5 seconds
  Serial.println("FILE CLOSED");
  audioFile = SPIFFS.open("/AudioFile01.wav", FILE_READ);
  size_t fileSize1 = audioFile.size();
  Serial.println("FILE SZE:" + String(fileSize1));
  audioFile.close();
  Serial.println("FILE END");
  delay(5000);

  //sendfile();
}
void loop() {
}