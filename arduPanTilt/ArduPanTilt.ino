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

// Pan tilt controller for the Arduino UNO
//

#include <Servo.h>

/*-------------------------------------------
    General configuration
 --------------------------------------------*/
#define DUMP 0          // Dump control variables to serial port. Set to 1
#define DELAY 50

/*-------------------------------------------
    Connection pins
 --------------------------------------------*/
#define PIN_PAN 10     // PWM output
#define PIN_TILT 9     // PWM output

#define PIN_AX A0      // Analog intput
#define PIN_AY A1      // Analog intput
#define PIN_SW 7       // Digital intput

/*-------------------------------------------
    Mode constants
 --------------------------------------------*/
#define MODE_MANUAL 0
#define MODE_AUTO 1

/*-------------------------------------------
    Servo positions
 --------------------------------------------*/
#define MIN_PAN 40
#define MAX_PAN 140
#define INI_PAN 90
#define KP_AX 15.0
#define KP_PAN 0.5

#define MIN_TILT 50
#define MAX_TILT 160
#define INI_TILT 75
#define KP_AY 20.0
#define KP_TILT 0.5

/*-------------------------------------------
    Variables
 --------------------------------------------*/
Servo servo_tilt;
Servo servo_pan;

float centerx;
float centery;

int pos_tilt = INI_TILT;
int pos_pan = INI_PAN;

int trg_tilt = pos_tilt;
int trg_pan = pos_pan;

int mode = MODE_MANUAL;

/*-------------------------------------------
    Selectors
 --------------------------------------------*/
bool hasSERIAL = false;
 
/*-------------------------------------------
    INITIALIZATION
 --------------------------------------------*/
void setup() 
{
  // wait for hardware serial to appear
  for (int i = 0; (i < 10) && !hasSERIAL; i++)
    if (Serial)
    {
      hasSERIAL = true;
      break;
    }
    else
      delay (500);
  
  // measure center joystick values
  int sumx = 0;
  int sumy = 0;
  for (int i = 0; i < 10; i++)
  {
    sumx += analogRead (PIN_AX);
    sumy += analogRead (PIN_AY);
  }
  centerx = (float) sumx / 10.0;
  centery = (float) sumy / 10.0;
  
  // make this baud rate fast enough so we aren't waiting on it
  if (hasSERIAL)
  {
    Serial.begin(115200);
    
    if (DUMP)
      Serial.println("ADx ADy PosX PosY TrgX TrgY");
  }

  // -----------------------
  // D/PWM Initialization
  // -----------------------
  pinMode(PIN_TILT, OUTPUT);
  pinMode(PIN_PAN, OUTPUT);
  pinMode(PIN_SW, INPUT_PULLUP);
  
  // -----------------------
  // Servo Initialization
  // -----------------------
  servo_tilt.attach (PIN_TILT);
  servo_tilt.write (pos_tilt);

  servo_pan.attach (PIN_PAN);
  servo_pan.write (pos_pan);

  // -----------------------
  // ADC Initialization
  // -----------------------
  //analogReference (AR_EXTERNAL);
  //analogReadResolution (10);
}
 
/*-------------------------------------------
    CYCLE LOOP
 --------------------------------------------*/
void loop() 
{
  int     adx, ady;
  float   deltax, deltay;

  if (mode == MODE_MANUAL)
  {
    if (digitalRead (PIN_SW) == LOW)
    {
      // read joystick switch
      trg_tilt = INI_TILT;
      trg_pan = INI_PAN;
    }
    else
    {
      // read joystick analog values
      adx = analogRead (PIN_AX);
      ady = analogRead (PIN_AY);
      
      deltax = ((float) adx - centerx) * KP_AX / 512.0;
      deltay = ((float) ady - centery) * KP_AY / 512.0;
    
      trg_pan -= (int) deltax;
      trg_tilt -= (int) deltay;
    
      // limit servo position for each axis independently
      if (trg_pan < MIN_PAN)       trg_pan = MIN_PAN;
      if (trg_pan > MAX_PAN)       trg_pan = MAX_PAN;
    
      if (trg_tilt < MIN_TILT)     trg_tilt = MIN_TILT;
      if (trg_tilt > MAX_TILT)     trg_tilt = MAX_TILT;
    }

    // dump analog input for serial plotter
    if (DUMP && hasSERIAL)
    {
      Serial.print(adx-centerx);
      Serial.print(" ");
      Serial.print(ady-centery);
      Serial.print(" ");
      Serial.print(pos_pan);
      Serial.print(" ");
      Serial.print(pos_tilt);
      Serial.print(" ");
      Serial.print(trg_pan);
      Serial.print(" ");
      Serial.print(trg_tilt);
      Serial.println();
    }
  }

  // write servo positions, smoothing out response curve
  pos_pan += (trg_pan - pos_pan) * KP_PAN;
  pos_tilt += (trg_tilt - pos_tilt) * KP_TILT;
  
  servo_tilt.write (pos_tilt);
  servo_pan.write (pos_pan);
  
  delay (DELAY);
}
