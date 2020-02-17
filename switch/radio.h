
#ifndef _RADIO_H__
#define _RADIO_H__

#include <Arduino.h>

const int short_pulse = 316;
const int long_pulse = 818;
const short attempts = 4;
const unsigned long address = 0x12340; //this could be any 20bit value (not all tested)

const byte radiocommands[] = {
    0b00000100, // all on   (0x4)
    0b00001000, // all off  (0x8)
    0b00001111, // 1 on     (0xF)
    0b00001110, // 1 off    (0xE)
    0b00001101, // 2 on     (0xD)
    0b00001100, // 2 off    (0xC)
    0b00001011, // 3 on     (0xB)
    0b00001010, // 3 off    (0xA)
    0b00000111, // 4 on     (0x7)
    0b00000110  // 4 off    (0x6)
};

// n = switch, mode = bool. n=0 means all. (n%=5 by default)
#define RADIOSWITCH(n, mode) \
  (radiocommands[(n % 5) * 2 + (short)!((bool)mode)])

byte rev(byte in)
{ //rev bit oriner in byte
  return ((in & 0x80) >> 7) |
         ((in & 0x40) >> 5) |
         ((in & 0x20) >> 3) |
         ((in & 0x10) >> 1) |
         ((in & 0x08) << 1) |
         ((in & 0x04) << 3) |
         ((in & 0x02) << 5) |
         ((in & 0x01) << 7);
}

void _rf(short output_pin, bool high)
{
  digitalWrite(output_pin, 1);
  delayMicroseconds(high ? long_pulse : short_pulse);
  digitalWrite(output_pin, 0);
  delayMicroseconds(high ? short_pulse : long_pulse);
}
void sendPacket(short pin, unsigned long data)
{
  unsigned long idx;
  for (idx = 0x80000000UL; idx > 0; idx >>= 1)
  {
    _rf(pin, idx & data); //write out each bit of data.
  }
  _rf(pin, 0); // 3 final low bits
  _rf(pin, 0);
  _rf(pin, 0);
  delay(10);
}
byte calculateCRC(unsigned long d)
{
  byte a, b, c;
  a = rev(d >> 16);
  b = rev(d >> 8);
  c = rev(d & 0xFF);
  return rev(a + b + c);
}
unsigned long makePacket(unsigned long address, byte command)
{
  //packet structure is, ( with 4bit nibble):
  // aaaaacxx; = 20bit address, 4bit command, 8bit crc;
  // making for a total of 32bits (unsigned long)
  unsigned long ret;
  ret = ((address & 0xFFFFF) << 4) | (command & 0xF);
  byte cx = calculateCRC(ret);

  return ((ret << 8) | cx);
}

void sendRadioPacket(short pin, byte cmd)
{
  unsigned long pkt = makePacket(address, cmd);
  for (int i = 0; i < attempts; i++)
  {
    sendPacket(pin, pkt);
  }
  delay(2000);
}

void radioSwitch(short pin, short id, bool state){
  sendRadioPacket(pin, RADIOSWITCH(id,state));
}
#endif