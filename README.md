# ESPVS1003
VS1003 Player Chip Arduino (ESP32) Library

This project is based on https://github.com/andykarpov/VS1003 . Thanks.  
Tested on ESP32 Arduino Environment https://github.com/espressif/arduino-esp32  
There are 2 examples:  
1. mp3player: player mp3 files on spiffs. you can upload mp3 files using https://github.com/me-no-dev/arduino-esp32fs-plugin
2. mp3player_multitask: multitask version of mp3player. one thread read data from spiffs, another thread feed the data to vs1003.

The multitask code is learned from https://github.com/copercini/esp32-iot-examples .

