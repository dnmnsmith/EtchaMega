/*
  SerialPassthrough sketch

  Some boards, like the Arduino 101, the MKR1000, Zero, or the Micro, have one
  hardware serial port attached to Digital pins 0-1, and a separate USB serial
  port attached to the IDE Serial Monitor. This means that the "serial
  passthrough" which is possible with the Arduino UNO (commonly used to interact
  with devices/shields that require configuration via serial AT commands) will
  not work by default.

  This sketch allows you to emulate the serial passthrough behaviour. Any text
  you type in the IDE Serial monitor will be written out to the serial port on
  Digital pins 0 and 1, and vice-versa.

  On the 101, MKR1000, Zero, and Micro, "Serial" refers to the USB Serial port
  attached to the Serial Monitor, and "Serial2" refers to the hardware serial
  port attached to pins 0 and 1. This sketch will emulate Serial passthrough
  using those two Serial ports on the boards mentioned above, but you can change
  these names to connect any two serial ports on a board that has multiple ports.

  created 23 May 2016
  by Erik Nyquist
*/
#include <Arduino.h>
#include <Wire.h>


#include "PositionDisplay.h"
#include "RotaryEncoder.h"
#include "Keyboard.h"
#include "I2CScan.h"

#define LOG_SERIAL_RX


struct PendingPositionInfo_t
{
  const char *name;
  float pos;
  unsigned long lastSendTime;
};

PendingPositionInfo_t pendingPos[4] = 
{ 
  { "X", 0.0, 0ul},
  { "Y", 0.0, 0ul},
  { "Z", 0.0, 0ul},
  { "A", 0.0, 0ul} 
};


static const unsigned long POS_UPDATE_TIME = 50ul;

bool IsPosPending(int index)
{
  return ((pendingPos[index].pos != 0.0) && (millis() - pendingPos[index].lastSendTime) > POS_UPDATE_TIME);
}

void sendPendingPos( int index )
{
  PendingPositionInfo_t &pos = pendingPos[ index ];

  String s = String();
  s.reserve(32);

  s += "<";
  s += pos.name;
  s += ":";
  s += String(pos.pos);
  s += ">";

  //Serial.println(s);
  Serial2.println(s);

  pos.pos = 0.0;
  pos.lastSendTime = millis();
}


void PollPendingPositions()
{
  for (int i = 0; i < 4; i++)
  {
    if (IsPosPending(i))
      sendPendingPos(i);
  }
}

void notifyIncrement(const char*name, int index, float delta)
{
  pendingPos[index].pos += delta;

  if (IsPosPending(index))
  {
    sendPendingPos(index);
  }
} 

void keypressNotify( int key )
{
  String s = String();
  s.reserve(32);
  s += "<K:";
  s += String( key, 16 );
  s += ">";

  Serial.println(s);
  Serial2.println(s);
}

void notifyKnobPress( int idx, int mask )
{
  Serial.print("notifyKnobPress : ");
  Serial.print(idx);
  Serial.print(" mask : 0x");
  Serial.println(mask);

  int keycode = 0x0F | mask | (idx << 4);

  keypressNotify( keycode );
}

int readXEncodePort() { return PINL; }
int readYEncodePort() { return PINC; }
int readZEncodePort() { return PINA; }

int readAEncodePort() 
{ 
  int pink = PINK;
  int mask = 0b11000011;

  int pinkNew = (pink & mask)
    | ((pink & 0b00000100) << 1)
    | ((pink & 0b00001000) >> 1)
    | ((pink & 0b00010000) << 1)
    | ((pink & 0b00100000) >> 1);
  return pinkNew; 
}


RotaryEnoderGroup xRotoryEncoder( "X", 0, readXEncodePort, notifyIncrement, notifyKnobPress );
RotaryEnoderGroup yRotoryEncoder( "Y", 1, readYEncodePort, notifyIncrement, notifyKnobPress );
RotaryEnoderGroup zRotoryEncoder( "Z", 2, readZEncodePort, notifyIncrement, notifyKnobPress );
RotaryEnoderGroup aRotoryEncoder( "A", 3, readAEncodePort, notifyIncrement, notifyKnobPress );

int apins[] = {A8,A9,A10,A11,A12,A13,A14,A15};
const int apinCount = sizeof(apins)/sizeof(apins[0]);

Keyboard keyboard( keypressNotify );

#define BUFFER_SIZE 256

void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);

  DDRL = 0;

  Wire.begin();

  for (int p = 0; p < apinCount; p++)
  {
         pinMode(apins[p], INPUT_PULLUP);
  }

  xRotoryEncoder.init();
  yRotoryEncoder.init();
  zRotoryEncoder.init();
  aRotoryEncoder.init();

  initPositionDisplay();

  keyboard.init();

  Serial.println("Hello");
  Serial2.println("<MegaHello>");

  i2cScan( "I2C", &Wire );
}

uint8_t cmdBuffer[BUFFER_SIZE];
int pos = 0;

void loop() {

  xRotoryEncoder.poll();
  yRotoryEncoder.poll();
  zRotoryEncoder.poll();
  aRotoryEncoder.poll();

  keyboard.poll();

  PollPendingPositions();

  if (Serial2.available()) {      // If anything comes in Serial (USB),
    int ch = Serial2.read();
    //Serial.println(ch);

    if (ch > 0)
    {
      cmdBuffer[pos++] = (uint8_t)ch;
  
      if ( (ch == 0) || (pos == (BUFFER_SIZE - 1)))
      {
        // Flush buffer
        pos = 0;
        Serial.println("Flushed buffer");
      }
      else if (ch == '\n')
      { 
        cmdBuffer[ pos ] = 0;

#ifdef LOG_SERIAL_RX    
        Serial.print("Received : ");
        Serial.print( (const char*)cmdBuffer );
#endif
        processBuffer( cmdBuffer );
        pos = 0;
      }      
    }
  }
}

