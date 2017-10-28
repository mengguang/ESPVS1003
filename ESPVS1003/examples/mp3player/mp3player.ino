#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ESPVS1003.h>
#include <SPI.h>
#include <FS.h>
#include <SPIFFS.h>
/*
 * XDCS - 0
 * DREQ - 25
 * XRES - 26
 * XCS  - 27
 * SCLK - 14
 * MOSI - 13
 * MISO - 12
 */

#define BUFFER_SIZE 128

VS1003 player(27, 0, 25, 26); // cs_pin, dcs_pin, dreq_pin, reset_pin

void setup()
{
    SPI.begin(14, 12, 13); //SCK, MISO, MOSI
    Serial.begin(115200);
    player.begin();
    player.setVolume(0x10);
    SPIFFS.begin();
}

void loop()
{
    File fp = SPIFFS.open("/qrsy-small.mp3");
    if(!fp) {
        Serial.println("Open file error.");
        delay(100);
        return;
    }
    Serial.println("start to play.");
    player.startSong();
    uint8_t buffer[BUFFER_SIZE];
    while(fp.position() < fp.size()) {
        uint8_t nread = fp.readBytes((char *)buffer,BUFFER_SIZE);
        player.playChunk(buffer, nread);
    }
    player.stopSong();
    fp.close();
    Serial.println("Stop play.");
    delay(1000);
}
