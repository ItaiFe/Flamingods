#include <Arduino.h>
#include "FS.h"
#include "SD_MMC.h"
#include "SD.h"
#include "SPI.h"
#include <Audio.h>              // ESP32 Audio library
#include <AudioGeneratorWAV.h>
#include <AudioFileSourceSD.h>
#include <AudioFileSourceFS.h>
#include <AudioOutputI2S.h>
#include <Wire.h>
#include <driver/i2s.h>
#include "ES8388.h"

#define TOUCH_PIN 14            // GPIO 14 for touch detection
int sensorValue;
// #define SD_CS 5                 // SD card CS pin (for SPI fallback)
// #define I2S_LRCK  25  // example
// #define I2S_BCLK  26
// #define I2S_DOUT  22
// #define I2S_DIN   35  // optional

// ES8388 audioCodec;
// SPIClass spi(VSPI);             // VSPI bus for SPI SD card

// // Audio objects
// AudioGeneratorWAV *wav = nullptr;
// AudioFileSourceFS *file = nullptr;
// AudioOutputI2S *out =  new AudioOutputI2S();

// bool sdMMCmode = true;          // Track which interface is used

void setup() {
//   pinMode(TOUCH_PIN, INPUT_PULLUP);
   Serial.begin(115200);
//   delay(1000);
//   Serial.println("Attempting to initialize SD card...");

//   if (!SD_MMC.begin("/sdcard", true)) { // 1-bit mode
//     Serial.println("SD_MMC failed, trying SPI mode...");
//     spi.begin(18, 19, 23, SD_CS);
//     if (!SD.begin(SD_CS, spi)) {
//       Serial.println("Both SD_MMC and SPI failed. Check connections:");
//       Serial.println("- Ensure SD card is inserted");
//       Serial.println("- Format SD card as FAT32");
//       return;
//     } else {
//       Serial.println("SD card initialized in SPI mode");
//       sdMMCmode = false;
//     }
//   } else {
//     Serial.println("SD card initialized in SD_MMC mode");
//   }

//   Serial.print("Active mode: ");
//   Serial.println(sdMMCmode ? "SD_MMC" : "SPI SD");

//   // Print card info
//   uint8_t cardType;
//   uint64_t cardSize;
//   if (sdMMCmode) {
//     cardType = SD_MMC.cardType();
//     cardSize = SD_MMC.cardSize() / (1024 * 1024);
//   } else {
//     cardType = SD.cardType();
//     cardSize = SD.cardSize() / (1024 * 1024);
//   }

//   if (cardType == CARD_NONE) {
//     Serial.println("No SD card attached");
//     return;
//   }

//   Serial.print("SD Card Type: ");
//   switch (cardType) {
//     case CARD_MMC:  Serial.println("MMC"); break;
//     case CARD_SD:   Serial.println("SDSC"); break;
//     case CARD_SDHC: Serial.println("SDHC"); break;
//     default:        Serial.println("UNKNOWN"); break;
//   }
//   Serial.printf("SD Card Size: %lluMB\n", cardSize);

//   // List root files
//   Serial.println("Listing files in root directory:");
//   File root = sdMMCmode ? SD_MMC.open("/") : SD.open("/");
//   if (root) {
//     File innerfile = root.openNextFile();
//     while (innerfile) {
//       Serial.print("  FILE: "); Serial.print(innerfile.path());
//       Serial.print("  SIZE: "); Serial.println(innerfile.size());
//       uint8_t buf[44];
//       innerfile.read(buf, sizeof(buf));
//       Serial.print("Header bytes: ");
//       for (int i=0; i<sizeof(buf); i++) Serial.printf("%02X ", buf[i]);
//       Serial.println();
//       innerfile.close();
//       innerfile = root.openNextFile();
//     }
//     root.close();
//   } else {
//     Serial.println("Failed to open root directory");
//   }

//   // Play WAV file if not already playing
//   if (!wav || !wav->isRunning()) {
//     // Initialize audio output
//     Wire.begin();
//     if (!audioCodec.begin(Wire, ES8388_ADDR)){
//       Serial.println("ES8388 initialization failed!");
//     }
//     else{
//       Serial.println("ES8388 initialized successfully!");
      
//       // Set volume (0–100)
//       audioCodec.setOutputVolume(80);

//       // Unmute output
//       audioCodec.mute(false);

//       // Enable output
//       audioCodec.outputEnable(true);

//       // // ---- I2S peripheral setup ----
//       // const i2s_config_t i2s_config = {
//       //   .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
//       //   .sample_rate = 44100,
//       //   .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
//       //   .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
//       //   .communication_format = I2S_COMM_FORMAT_I2S_MSB,
//       //   .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
//       //   .dma_buf_count = 8,
//       //   .dma_buf_len = 64,
//       //   .use_apll = false,
//       //   .tx_desc_auto_clear = true,
//       //   .fixed_mclk = 0
//       // };

//       // const i2s_pin_config_t pin_config = {
//       //     .bck_io_num = I2S_BCLK,
//       //     .ws_io_num = I2S_LRCK,
//       //     .data_out_num = I2S_DOUT,
//       //     .data_in_num = I2S_PIN_NO_CHANGE
//       // };

//       // i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
//       // i2s_set_pin(I2S_NUM_0, &pin_config);
//       // i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);

//       // Serial.println("I2S configured — starting tone");
//     }

//     // out->SetPinout(I2S_BCLK, I2S_LRCK, I2S_DOUT, 0); // last param = MCLK pin (0 or your pin)
//     // out->SetGain(0.8);  // optional volume adjustment
//     // out->SetChannels(2);
//     // out->SetBitsPerSample(16);
//     // out->SetRate(44100);
//     // out->begin();
  
//     if (sdMMCmode) {
//       file = new AudioFileSourceFS(SD_MMC, "/1.wav");
//     } else {
//       file = new AudioFileSourceFS(SD, "/1.wav");
//     }

//     // Open WAV file
//     if(file){
//       Serial.println("File opened successfully!");
//     }else{
//       Serial.println("File NOT found!");
//     }
//   }

//   // Open WAV file from SD card
//   wav = new AudioGeneratorWAV();

//   // if (!wav->begin(file, out)) {
//   //   Serial.println("Failed to start WAV playback!");
//   // } else {
//   //   Serial.println("Playing /1.wav...");
//   // }
//   loop();
}

void loop() {
  // delay(1000);
  // printf("loop");

  // static int phase = 0;
  // const int freq = 1000;
  // const int sampleRate = 44100;
  // int16_t sample[2];
  // size_t bytes_written;

  // for (int i = 0; i < 441; i++) {
  //   float value = sinf(2 * M_PI * freq * phase / (float)sampleRate);
  //   int16_t s = (int16_t)(value * 32767);
  //   sample[0] = s;
  //   sample[1] = s;
  //   i2s_write(I2S_NUM_0, sample, sizeof(sample), &bytes_written, portMAX_DELAY);
  //   phase++;
  // }


  // if (wav->isRunning()) {
  //   wav->loop();
  //   delay(1);
  // } else {
  //   Serial.println("Playback finished.");
  //   wav->stop();
  //   while (1);
  // }

  // Read touch sensor
  sensorValue = digitalRead(TOUCH_PIN); // 1 - Touch, 0 - No Touch
  Serial.print(sensorValue);

  if (sensorValue == 0) {
    Serial.println(" *** NO TOUCH DETECTED! ***");
  }
  else {
    Serial.println(" *** TOUCH DETECTED! ***");
  }

  delay(250); // Prevent multiple triggers, Debounce & loop delay
}