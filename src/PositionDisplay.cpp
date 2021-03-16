#include <Arduino.h>

#include <SPI.h>

#include <HCMAX7219.h>

#include "PositionDisplay.h"

#define NUM_AXES 4
#define X_7SEG_LOAD 40
#define Y_7SEG_LOAD 41
#define Z_7SEG_LOAD 39
#define A_7SEG_LOAD 38

HCMAX7219 HCMAX7219_X(X_7SEG_LOAD);
HCMAX7219 HCMAX7219_Y(Y_7SEG_LOAD);
HCMAX7219 HCMAX7219_Z(Z_7SEG_LOAD);
HCMAX7219 HCMAX7219_A(A_7SEG_LOAD);

HCMAX7219* segs[] = { &HCMAX7219_X, &HCMAX7219_Y, &HCMAX7219_Z, &HCMAX7219_A };


int countChar( const char *s, char c)
{
  int count = 0;
  for (const char *p = s; *p; p++)
  {
    if (*p == c)
      count++;
  }
  return count;
}

void initPositionDisplay()
{
  SPI.begin();

  for(int i = 0; i < NUM_AXES; i++)
  {
    segs[i]->Intensity(1,0);
    segs[i]->Clear();
    segs[i]->print7Seg("HELLO",8);
    segs[i]->Refresh();
  }

}


void displayAxis( uint8_t axis, const char * s)
{
  HCMAX7219* pDisplay = nullptr;
  if (axis == 'X')
  {
    pDisplay = &HCMAX7219_X;
  }
  else if (axis == 'Y')
  {
    pDisplay = &HCMAX7219_Y;
  }
  else if (axis == 'Z')
  {
    pDisplay = &HCMAX7219_Z;
  }
  else if (axis == 'A')
  {
    pDisplay = &HCMAX7219_A;
  }

  int offset = strlen( s ) - 1;
  pDisplay->Clear();
  pDisplay->print7Seg( s,offset);
  pDisplay->Refresh();  
}


char *trim( char *s )
{
    int start = 0;
    int len = strlen(s);

    while (len > 0 && isSpace(s[len-1]))
    {
      len--;
      s[len]=0;
    }
    while (len > 0 && isSpace( s[start] ))
    {
      start++;
      len--;
    }

    return &s[ start ];
}


void processBuffer(uint8_t* u )
{
    char *s = trim( reinterpret_cast<char *>(u) );
    int len = strlen(s);

    //Serial.println("processTrimmedBuffer");
    if ( (countChar(s,'<') !=1 ) || (countChar(s,'>') !=1 ) || (countChar(s,':') !=1 ))
    {
          Serial.println("Rejected - bad delim count");
          return;
    }

    // Bracket delimited.
    if (s[0] == '<' && s[len-1] == '>')
    {
      if (s[1] == 'W' && s[2] == 'P' && s[3] && 'O' && s[4] == 'S' && s[5] == ':')
      {
          char *xVal = (char *)&s[6];

          char *yVal = strchr(xVal,',');
          if (yVal == nullptr)
          {
            Serial.println("Y delim not found.");
            return;
          }
          *yVal++='\0';
          char *zVal = strchr(yVal,',');
          if (zVal == nullptr)
          {
            Serial.println("Z delim not found.");
            return;
          }
          *zVal++='\0';
          char *end = strchr(zVal,'>');
          if (end == nullptr)
          {
            Serial.println("> end delim not found.");
            return;
          }
          *end='\0';

          displayAxis( 'X', xVal );
          displayAxis( 'Y', yVal );
          displayAxis( 'Z', zVal );
      } 
      else if (s[1]=='A' && s[2]==':')
      {
          char *aVal = (char *)&s[3];
          char *end = strchr(aVal,'>');
          if (end == nullptr)
          {
            Serial.println("> end delim not found.");
            return;
          }
          *end='\0';
          displayAxis( 'A', aVal );
      }
      else
      {
        Serial.println("Rejected ");
      }

    }
    else
    {
      Serial.println("Rejected ");
    }
    
}
