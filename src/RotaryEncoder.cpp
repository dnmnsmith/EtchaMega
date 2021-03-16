#include <Arduino.h>

#include "RotaryEncoder.h"

//#define DEBUG_ROTARY

static const int BUTTON_DEBOUNCE_COUNT = 4;

RotaryEncoder::RotaryEncoder( 
    const char *name, 
    int index,
    EncoderType encoderType,
    int shift,        // Amount to shift value for our state bits.
    float delta,      // Amount that one click sends.
    void(*notifyIncrement)(const char*,int,float)
      ) : 
        m_name(name),
        m_index(index),
        m_encoderType(encoderType),
        m_shift(shift),
        m_delta(delta),
        m_notifyIncrement(notifyIncrement)
{


}

void RotaryEncoder::notify( int value )
{
    int newBits = (value >> m_shift) & 3;

    if (newBits != m_prevBits)
    {
        m_prevBits = newBits;

        m_state  = ((m_state << 2) & 0xFC) | newBits;

#ifdef DEBUG_ROTARY
        Serial.print("New state for ");
        Serial.print(m_name);
        Serial.print("(");
        Serial.print(m_delta);
        Serial.print(") = ");
        Serial.print(m_state, BIN );
        Serial.print(" = ");
        Serial.print(m_state, HEX );
        Serial.println();
#endif

        if (m_notifyIncrement != nullptr)
        {
            if (m_encoderType == INDUSTRIAL)
            {
                switch( m_state)
                {
                case B01111000:
                    m_notifyIncrement( m_name, m_index, -1 * m_delta );
                    m_state = 0;
                    break;

                case B10110100:
                    m_notifyIncrement( m_name, m_index, m_delta );
                    m_state = 0;
                    break;
                }
            }
            else
            {
                switch( m_state)
                {
                case B01001011:
                    m_notifyIncrement( m_name, m_index, -1 * m_delta );
                    m_state = 0;
                    break;

                case B10000111:
                    m_notifyIncrement( m_name, m_index, m_delta );
                    m_state = 0;
                    break;
                }
            }
        }

    }
}



RotaryEnoderGroup::RotaryEnoderGroup( 
    const char* name, 
    int index, 
    int(*readPort)(), 
    void(*notifyIncrement)(const char*,int,float),
    void (*notifyKnobPress)( int, int )) : 
    m_name(name), 
    m_index(index),
    m_readPort(readPort),
    m_notifyKnobPress(notifyKnobPress),
    m_normalEncoder( name, index, RotaryEncoder::INDUSTRIAL, 0, 1.0, notifyIncrement ),
    m_fineEncoder( name, index, RotaryEncoder::SIMPLE, 2, 0.1, notifyIncrement ),
    m_superfineEncoder( name, index, RotaryEncoder::SIMPLE, 4, 0.01, notifyIncrement )
{

}

void RotaryEnoderGroup::init()
{
    m_prevState = m_readPort();
}

void RotaryEnoderGroup::poll()
{
    int portVal = m_readPort();

    int buttonState = portVal & (m_button1Mask | m_button2Mask);

    if (portVal != m_prevState)
    {
#ifdef DEBUG_ROTARY        
        //Serial.println(portVal,16);
#endif        
        m_prevState = portVal;

        m_normalEncoder.notify( portVal );
        m_fineEncoder.notify( portVal );
        m_superfineEncoder.notify( portVal );

        if ( (m_buttonState == 0) && (buttonState != 0))
        {
            m_buttonState = buttonState;
            m_buttonStateCount = 0;
        }
        else
        {
            m_buttonState = 0;
            m_buttonStateCount = 0;
        }        
    }
    else
    {
        if (m_buttonState == buttonState)
            m_buttonStateCount++;
        
        if (m_buttonStateCount == BUTTON_DEBOUNCE_COUNT && m_notifyKnobPress != nullptr)
            m_notifyKnobPress( m_index, m_buttonState );
    }
    
}
