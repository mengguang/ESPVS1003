#include <SPI.h>
#include <ESPVS1003.h>

const uint8_t vs1003_chunk_size = 32;


#define VS_WRITE_COMMAND 0x02
#define VS_READ_COMMAND 0x03

// SCI Registers

const uint8_t SCI_MODE = 0x0;
const uint8_t SCI_STATUS = 0x1;
const uint8_t SCI_BASS = 0x2;
const uint8_t SCI_CLOCKF = 0x3;
const uint8_t SCI_DECODE_TIME = 0x4;
const uint8_t SCI_AUDATA = 0x5;
const uint8_t SCI_WRAM = 0x6;
const uint8_t SCI_WRAMADDR = 0x7;
const uint8_t SCI_HDAT0 = 0x8;
const uint8_t SCI_HDAT1 = 0x9;
const uint8_t SCI_AIADDR = 0xa;
const uint8_t SCI_VOL = 0xb;
const uint8_t SCI_AICTRL0 = 0xc;
const uint8_t SCI_AICTRL1 = 0xd;
const uint8_t SCI_AICTRL2 = 0xe;
const uint8_t SCI_AICTRL3 = 0xf;
const uint8_t SCI_num_registers = 0xf;

// SCI_MODE bits

const uint8_t SM_DIFF = 0;
const uint8_t SM_LAYER12 = 1;
const uint8_t SM_RESET = 2;
const uint8_t SM_OUTOFWAV = 3;
const uint8_t SM_EARSPEAKER_LO = 4;
const uint8_t SM_TESTS = 5;
const uint8_t SM_STREAM = 6;
const uint8_t SM_EARSPEAKER_HI = 7;
const uint8_t SM_DACT = 8;
const uint8_t SM_SDIORD = 9;
const uint8_t SM_SDISHARE = 10;
const uint8_t SM_SDINEW = 11;
const uint8_t SM_ADPCM = 12;
const uint8_t SM_ADCPM_HP = 13;
const uint8_t SM_LINE_IN = 14;

static void debug_log(char *message) {
  Serial.println(message);
}

uint16_t VS1003::read_register(uint8_t _reg) const
{
  uint16_t result;
  control_mode_on();
  delayMicroseconds(1);             // tXCSS
  SPI.transfer(VS_READ_COMMAND);    // Read operation
  SPI.transfer(_reg);               // Which register
  result = SPI.transfer(0xff) << 8; // read high byte
  result |= SPI.transfer(0xff);     // read low byte
  delayMicroseconds(1);             // tXCSH
  await_data_request();
  control_mode_off();
  return result;
}

void VS1003::write_register(uint8_t _reg, uint16_t _value) const
{
  control_mode_on();
  delayMicroseconds(1);           // tXCSS
  SPI.transfer(VS_WRITE_COMMAND); // Write operation
  SPI.transfer(_reg);             // Which register
  SPI.transfer(_value >> 8);      // Send hi byte
  SPI.transfer(_value & 0xff);    // Send lo byte
  delayMicroseconds(1);           // tXCSH
  await_data_request();
  control_mode_off();
}

void VS1003::sdi_send_buffer(const uint8_t *data, size_t len)
{
  uint16_t i = 0;
  data_mode_on();
  uint8_t c;
  while (len)
  {
    await_data_request();
    delayMicroseconds(3);

    size_t chunk_length = min(len, vs1003_chunk_size);
    len -= chunk_length;
    while (chunk_length--)
    {
      c = *(data + i);
      SPI.transfer(c);
      i++;
    }
  }
  data_mode_off();
}

void VS1003::sdi_send_zeroes(size_t len)
{
  data_mode_on();
  while (len)
  {
    await_data_request();
    size_t chunk_length = min(len, vs1003_chunk_size);
    len -= chunk_length;
    while (chunk_length--)
      SPI.transfer(0);
  }
  data_mode_off();
}

VS1003::VS1003(uint8_t _cs_pin, uint8_t _dcs_pin, uint8_t _dreq_pin, uint8_t _reset_pin)
{
  cs_pin = _cs_pin;
  dcs_pin = _dcs_pin;
  dreq_pin = _dreq_pin;
  reset_pin = _reset_pin;
}

void VS1003::begin(void)
{

  // Keep the chip in reset until we are ready
  pinMode(reset_pin, OUTPUT);
  digitalWrite(reset_pin, LOW);

  // The SCI and SDI will start deselected
  pinMode(cs_pin, OUTPUT);
  digitalWrite(cs_pin, HIGH);
  pinMode(dcs_pin, OUTPUT);
  digitalWrite(dcs_pin, HIGH);

  // DREQ is an input
  pinMode(dreq_pin, INPUT);

  delay(1);

  

  // init SPI slow mode
  SPI.setClockDivider(SPI_CLOCK_DIV64); // Slow!

  // release from reset
  digitalWrite(reset_pin, HIGH);

  // Declick: Immediately switch analog off
  write_register(SCI_VOL, 0xffff); // VOL

  /* Declick: Slow sample rate for slow analog part startup */
  write_register(SCI_AUDATA, 10);

  delay(100);
  
  /* Switch on the analog parts */
  write_register(SCI_VOL, 0xfefe); // VOL

  write_register(SCI_AUDATA, 44101); // 44.1kHz stereo

  write_register(SCI_VOL, 0x2020); // VOL

  // soft reset
  write_register(SCI_MODE, _BV(SM_SDINEW) | _BV(SM_RESET));
  delay(1);
  await_data_request();
  
  write_register(SCI_CLOCKF, 0xB800); // Experimenting with higher clock settings
  delay(1);
  await_data_request();

  // Now you can set high speed SPI clock
  SPI.setClockDivider(SPI_CLOCK_DIV4); // Fastest available
}

void VS1003::setVolume(uint8_t vol) const
{
  uint16_t value = vol;
  value <<= 8;
  value |= vol;

  write_register(SCI_VOL, value);
}

void VS1003::startSong(void)
{
  sdi_send_zeroes(10);
}

void VS1003::playChunk(const uint8_t *data, size_t len)
{
  sdi_send_buffer(data, len);
}

void VS1003::stopSong(void)
{
  sdi_send_zeroes(2048);
}
