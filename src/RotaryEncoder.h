#pragma once

#include <Arduino.h>




class RotaryEncoder
{
  public:
    enum EncoderType { INDUSTRIAL, SIMPLE };

    RotaryEncoder( 
      const char *name, 
      int index,
      EncoderType encoderType,
      int shift,        // Amount to shift value for our state bits.
      float delta,      // Amount that one click sends.
      void(*notifyIncrement)(const char*,int,float)
  );

    void notify( int value );

  private:
    const char * m_name;
    int m_index;
    EncoderType m_encoderType;
    int m_shift = 0;
    float m_delta = 1.0;
    int m_state = 0;
    int m_prevBits = 0;
    void(*m_notifyIncrement)(const char*,int,float) = nullptr;
};


class RotaryEnoderGroup
{
  public:
    RotaryEnoderGroup( 
      const char* name, 
      int index, 
      int (*readPort)(), 
      void (*notifyIncrement)(const char*,int,float),
      void (*notifyKnobPress)( int, int )
      );

    void init();

    void poll();

  private:
    const char* m_name;
    int m_index;
    int (*m_readPort)();
    int m_prevState = 0;
    void (*m_notifyKnobPress)( int, int ) = nullptr;

    const int m_button1Mask = 0x80;
    const int m_button2Mask = 0x40;

    int m_buttonState = 0;
    int m_buttonStateCount = 0;

    RotaryEncoder m_normalEncoder;
    RotaryEncoder m_fineEncoder;
    RotaryEncoder m_superfineEncoder;
};

