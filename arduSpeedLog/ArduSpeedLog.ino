/*
 * Copyright (C) 2020 Humberto Martínez Barberá
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Paddlewheel speed log for the Arduino UNO
//

#include <LiquidCrystal_PCF8574.h>
#include <Wire.h>

/*-------------------------------------------
    Connection pins
 --------------------------------------------*/

#define PIN_AX A0       // Hall effect sensor analog intput
#define PIN_LED 13      // LED debugging output

/*-------------------------------------------
   LCD
 --------------------------------------------*/

LiquidCrystal_PCF8574 lcd(0x27); // set the LCD address to 0x27 for a 16 chars and 2 line display

/*-------------------------------------------
    Timers
 --------------------------------------------*/
#define TIMER_SER 250

uint32_t timerSER;

/*-------------------------------------------
    Variables
 --------------------------------------------*/
bool has_lcd = false;
bool state = false;
int count;

void setup () 
{            
  // Set IO pin configuration    
  pinMode (PIN_LED, OUTPUT); 
  pinMode (PIN_AX, INPUT);

  // Open serial port
  Serial.begin (115200);

  // Initialise LCD and check for erros
  Wire.begin();
  Wire.beginTransmission (0x27);
  int error = Wire.endTransmission ();
  if (error == 0)
  {
    Serial.println ("LCD initialised");
    has_lcd = true;
    
    lcd.setBacklight (255);
    lcd.home ();
    lcd.setCursor (0, 0);
    lcd.print("Ardu Speed Log");
    lcd.setCursor (0, 1);
    lcd.print ("Count: ");
  }
  else
  {
    Serial.print ("LCD error=");
    Serial.println (error);
    
    has_lcd = false;
  }

  // Initialise counters
  timerSER = millis (); 
  count = 0;
}

void loop () 
{
  int       hall;
  bool      level;
  uint32_t  curtime, dt_time;

  // read current timestamp
  curtime = millis();

  // read hall effect sensor
  hall = analogRead (PIN_AX);
  if (hall > 512)
  {
    level = true;
    digitalWrite (PIN_LED, HIGH);
  }
  else
  {
    level = false;
    digitalWrite (PIN_LED, LOW);
  }

  // check if there is a state change
  if (level != state)
  {
    count ++;
    state = level;
  }
  
  dt_time = curtime - timerSER;
  if (dt_time > TIMER_SER) 
  {
    Serial.println (count);

    // display counter on LCD
    if (has_lcd)
    {
      lcd.setCursor (7, 1);
      lcd.print (count);
      lcd.print ("   ");   
    }
    
    count = 0;
    timerSER = curtime;
  }
}
