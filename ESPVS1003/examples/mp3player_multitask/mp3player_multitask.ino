#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ESPVS1003.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

/*
 * XDCS - 0
 * DREQ - 25
 * XRES - 26
 * XCS  - 27
 * SCLK - 14
 * MOSI - 13
 * MISO - 12
 */

VS1003 player(27, 0, 25, 26); // cs_pin, dcs_pin, dreq_pin, reset_pin

#define BUFFER_SIZE 8192
bool player_have_data = false;
uint8_t data_buffer[BUFFER_SIZE];
uint32_t data_length = 0;
uint8_t player_buffer[BUFFER_SIZE];
uint32_t player_length = 0;
File fp;
void readerLoop(void *pvParameters)
{
  while (1)
  {
    if (player_have_data == false)
    {
      if (fp.position() < fp.size())
      {
        Serial.printf("read data: %d\n", fp.position());
        data_length = fp.readBytes((char *)data_buffer, BUFFER_SIZE);
        Serial.printf("readed data: %d\n", fp.position());
        player_have_data = true;
      }
      else
      {
        fp.seek(0, SeekSet);
      }
    }
    delay(1);
  }
}

void playerLoop(void *pvParameters)
{
  while (1)
  {
    if (player_have_data)
    {
      memcpy(player_buffer, data_buffer, data_length);
      player_length = data_length;
      player_have_data = false;
      Serial.printf("play data %d\n", fp.position());
      player.playChunk(player_buffer, player_length);
    }
    delay(1);
  }
}

void setup()
{
  SPI.begin(14, 12, 13); //SCK, MISO, MOSI
  Serial.begin(115200);
  player.begin();
  player.setVolume(0x10);
  SPIFFS.begin();
  fp = SPIFFS.open("/qrsy-small.mp3");
  if (!fp)
  {
    Serial.println("Open file error.");
    delay(100);
    return;
  }
  xTaskCreatePinnedToCore(readerLoop, "reader", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreatePinnedToCore(playerLoop, "player", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}

void loop()
{
  delay(100);
  Serial.println(millis());
}
