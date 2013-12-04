/**
 *   Copyright (C) 2013 Ben Martin
 *
 *   Code for doing interesting things on Arduino.
 *
 *   libferris is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   libferris is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with libferris.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   For more details see the COPYING file in the root directory of this
 *   distribution.
 *
 *   $Id: Ferris.hh,v 1.70 2011/07/31 21:30:49 ben Exp $
 *
 ********************************************************************
 *   
 * A server for the attiny84 that controls an OLED screen on the main
 * arduino's commands. The attiny sleeps when it can and after a delay
 * will turn off the OLED too.
 *
 *
 *
 * ATMEL ATTINY84 / ARDUINO
 *
 *                    +-\/-+
 *         VCC       1| XX |14 GND
 *         (D10) PB0 2|    |13 AREF(D0)
 *         (D9)  PB1 3|    |12 PA1 (D1)
 *               PB3 4|    |11 PA2 (D2)
 * PWM INT0 (D8) PB2 5|    |10 PA3 (D3)
 * PWM      (D7) PA7 6|    |9  PA4 (D4)
 * PWM      (D6) PA6 7|    |8  PA5 (D5) PWM
 *                    +----+
 */

#include "pins_arduino.h"
#include <avr/sleep.h>

#include <ByteBuffer.h>
#include <SimpleMessagePassing5.h>
#include <Adafruit_CharacterOLED.h>

#include "attiny_oled_server_spi_privdetails.h"


int chipSelect = 7; // physical pin6
SimpleMessagePassing5 smp( chipSelect );

//
// SPI uses 4,5,6,7 so we have to skip those.
//
Adafruit_CharacterOLED oled( 0, 1, 2, 3, 8, 9, 10 );



void setup()
{
    delay(2000);
    smp.init();
}


void snooze() 
{
  set_sleep_mode (SLEEP_MODE_PWR_DOWN);  
  sleep_enable();
  sleep_mode(); // <-- goes to sleep here
  sleep_cpu ();  
  sleep_disable();   
}


void loop() 
{
    oled.display();
    
    while( smp.takeMessage() )
    {
        byte c  = smp.buffer().get();
        byte sz = smp.buffer().get();
        byte m  = smp.buffer().get();
        
        switch( m )
        {
            case METH_clear:     oled.clear();           break;
            case METH_home:      oled.home();            break;
            case METH_noDisplay: oled.noDisplay();       break;
            case METH_display:   oled.display();         break;
            case METH_noBlink:   oled.noBlink();         break;
            case METH_blink:     oled.blink();           break;
            case METH_noCursor:  oled.noCursor();        break;
            case METH_cursor:    oled.cursor();          break;
            case METH_scrollDisplayLeft:  oled.scrollDisplayLeft(); break;
            case METH_scrollDisplayRight: oled.scrollDisplayRight(); break;
            case METH_leftToRight:  oled.leftToRight();  break;
            case METH_rightToLeft:  oled.rightToLeft();  break;
            case METH_autoscroll:   oled.autoscroll();   break;
            case METH_noAutoscroll: oled.noAutoscroll(); break;

            case METH_begin:
                oled.begin( smp.buffer().get(), smp.buffer().get() );
                break;

                // This needs an array, so I left it for now
//              METH_createChar,
                
            case METH_setCursor: // uint8_t, uint8_t
            { 
                byte a = smp.buffer().get();
                byte b = smp.buffer().get();
                oled.setCursor( a, b );
            }
                break;
            case METH_write: // uint8_t (RET=size_t)
                oled.write( smp.buffer().get() ); 
                break;
            case METH_command: // uint8_t
                oled.command( smp.buffer().get() ); 
                break;
        }
    }

    if( smp.timedOut() )
    {
        // been a while since we got any new messages
        // so lets turn off the screen before we go back to sleep
        oled.noDisplay();
    }
    
    if( smp.shouldSleep() )
        snooze();
}




