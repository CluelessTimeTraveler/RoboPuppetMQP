/**
 * @file Buttons.cpp
 */
#include "Buttons.h"
#include <Arduino.h>
/**
 * Private subsystem info
 */
namespace Buttons
{
  const int gripperToggle = 34;
  const int holdToggle = 35;
  bool gripperState;
  bool holdState;
}

void Buttons::init(){
  pinMode(gripperToggle, INPUT);
  pinMode(holdToggle, INPUT);
  holdState = true;
  gripperState = false;
}

void Buttons::update(){
 
 if(digitalRead(holdToggle)){
    Serial.print("hold toggle: ");
    gripperState =! gripperState;
    Serial.println(gripperState);
}

if(digitalRead(gripperToggle))
    gripperState =! gripperState;
}

bool Buttons::getHoldStatus()
{
  return holdState;
}

bool Buttons::getGripperStatus()
{
  return gripperState;
}

