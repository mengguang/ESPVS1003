
#ifndef __VS1003_H__
#define __VS1003_H__

#include <Arduino.h>

class VS1003
{
private:
  uint8_t cs_pin;
  uint8_t dcs_pin;
  uint8_t dreq_pin;
  uint8_t reset_pin;

protected:
  inline void await_data_request(void) const
  {
    while (!digitalRead(dreq_pin))
      delay(1);
  }

  inline void control_mode_on(void) const
  {
    digitalWrite(dcs_pin, HIGH);
    digitalWrite(cs_pin, LOW);
  }

  inline void control_mode_off(void) const
  {
    digitalWrite(cs_pin, HIGH);
  }

  inline void data_mode_on(void) const
  {
    digitalWrite(cs_pin, HIGH);
    digitalWrite(dcs_pin, LOW);
  }

  inline void data_mode_off(void) const
  {
    digitalWrite(dcs_pin, HIGH);
  }

  uint16_t read_register(uint8_t _reg) const;
  void write_register(uint8_t _reg, uint16_t _value) const;
  void sdi_send_buffer(const uint8_t *data, size_t len);
  void sdi_send_zeroes(size_t length);
  void print_byte_register(uint8_t reg) const;

public:
  /**
   * Constructor
   *
   * Only sets pin values.  Doesn't do touch the chip.  Be sure to call begin()!
   */
  VS1003(uint8_t _cs_pin, uint8_t _dcs_pin, uint8_t _dreq_pin, uint8_t _reset_pin);

  /**
   * Begin operation
   *
   * Sets pins correctly, and prepares SPI bus.
   */
  void begin(void);

  /**
   * Prepare to start playing
   *
   * Call this each time a new song starts.
   */
  void startSong(void);

  /**
   * Play a chunk of data.  Copies the data to the chip.  Blocks until complete.
   *
   * @param data Pointer to where the data lives
   * @param len How many bytes of data to play
   */
  void playChunk(const uint8_t *data, size_t len);

  /**
   * Finish playing a song.
   *
   * Call this after the last playChunk call.
   */
  void stopSong(void);

  /**
   * Print configuration details
   *
   * Dumps all registers to stdout.  Be sure to have stdout configured first
   * (see fdevopen() in avr/io.h).
   */
  void printDetails(void) const;

  /**
   * Set the player volume
   *
   * @param vol Volume level from 0-255, lower is louder.
   */
  void setVolume(uint8_t vol) const;
};

#endif // __VS1003_H__
