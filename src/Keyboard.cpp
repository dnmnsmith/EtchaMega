#include <Arduino.h>

#include "Keyboard.h"

#define DEBOUNCE_COUNT 3

int rows[] = {A0,A1,A2,A3,A4,A5,A6,A7};
const int rowCount = sizeof(rows)/sizeof(rows[0]);

int cols[] = {2,3,4,5,6,7,8,9,10,11,12,13};
const int colCount = sizeof(cols)/sizeof(cols[0]);

Keyboard::Keyboard(void(*keyPressNotify)( int keyCode ) )
    : m_keyPressNotify( keyPressNotify )
{

}

void Keyboard::init(void(*keyPressNotify)( int keyCode ) )
{
    if (keyPressNotify != nullptr)
        m_keyPressNotify = keyPressNotify;

    for (int x=0; x<rowCount; x++) {
        pinMode(rows[x], INPUT);
    }

    for (int x=0; x<colCount; x++) {
        pinMode(cols[x], INPUT_PULLUP);
    }

}


static const int MAX_KEYS = 10;

void Keyboard::poll()
{
    int keys = 0;
    int key = -1;
    int keyList[MAX_KEYS];


    // iterate the columns
    for (int colIndex=0; colIndex < colCount; colIndex++) 
    {
        // col: set to output to low
        byte curCol = cols[colIndex];
        pinMode(curCol, OUTPUT);
        digitalWrite(curCol, LOW);

        // row: interate through the rows
        for (int rowIndex=0; rowIndex < rowCount; rowIndex++) {
            byte rowCol = rows[rowIndex];
            pinMode(rowCol, INPUT_PULLUP);
            if (!digitalRead(rowCol) )
            {
                key = makeKeyCode( colIndex, rowIndex );
                if (keys < MAX_KEYS)
                    keyList[keys] = key;
                // Serial.print("Key down = ");
                // Serial.println((uint8_t)key,16);
                keys++;
            }
            pinMode(rowCol, INPUT);
        }
 
        // disable the column
        pinMode(curCol, INPUT);

    }
    // if (keys > 1)
    // {
    //     Serial.print("Keys hit : ");
    //     for (int i =0; i < min(keys,MAX_KEYS);i++)
    //     {
    //         Serial.print(keyList[i],16);
    //         Serial.print(" ");    
    //     }
    //     Serial.println();
    // }

    if (m_waitRelease)
    {
        if (key == -1)
        {
            m_keyPress = -1;
            m_count = 0;
            m_waitRelease = false;
        }        
    }
    else if (keys != 1)
    {
        m_keyPress = -1;
        m_count = 0;
    }
    else if (m_keyPress == -1)
    {
        m_keyPress = key;
        m_count = 0;
    }
    else if (key != m_keyPress)
    {
        m_keyPress = key;
        m_count = 0;
    }
    else if (m_keyPress == key)
    {
        if (m_count < DEBOUNCE_COUNT)
        {
            m_count++;
            if (m_count == DEBOUNCE_COUNT)
            {
                m_keyPressNotify( key );
                // Serial.print("Key Pressed : 0x");
                // Serial.println(m_keyPress,16 );
                m_waitRelease = true;
            }
        }
    }
    

}

int Keyboard::makeKeyCode( int column, int row)
{
     return column | (row << 4);
}
