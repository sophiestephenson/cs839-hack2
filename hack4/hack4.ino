#include "AudioTools.h"

uint16_t sample_rate=44100;
uint8_t channels = 2;                                      // The stream will have 2 channels 
AnalogAudioStream in; 
CsvStream<int16_t> out(Serial); 
ConverterAutoCenter<int16_t> center(2); // set avg to 0

// Arduino Setup
void setup(void) {  
  // Open Serial 
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Info);

  // Define CSV Output
  auto config = out.defaultConfig();
  config.sample_rate = sample_rate; 
  config.channels = channels;
  config.bits_per_sample = sizeof(int16_t)*8; 
  out.begin(config);

  // Setup sine wave
  auto cfgRx = in.defaultConfig(RX_MODE);
  cfgRx.sample_rate = sample_rate;
  cfgRx.channels = channels;
  in.begin(cfgRx); // frequency of note B4
}

StreamCopy copier(out, in);                             
void loop() {
  copier.copy(center);
}