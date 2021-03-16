#pragma once

#include <Arduino.h>

class Keyboard
{

    public:
        Keyboard( void(*keyPressNotify)( int keyCode ) );
        ~Keyboard() = default;

        void init( void(*keyPressNotify)( int keyCode ) = nullptr );

        void poll();

        // From column number and row combo.
        int makeKeyCode( int column, int row);

    private:
        int m_keyPress = -1;
        int m_count = 0;
        bool m_waitRelease = false;

        void(*m_keyPressNotify)( int keyCode ) = nullptr;
};
