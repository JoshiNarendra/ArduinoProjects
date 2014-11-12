#include <Wire.h>

// Global Constants, to adjust sensitivity of the lick sensor
#define TOU_THRESH	0x06
#define	REL_THRESH	0x0A

int odorValve = 24;
int waterValve =  36;
int lickSensor = 13;  // Digital pin 13 for touchSensor input
boolean touchStates[12]; //to keep track of the previous touch states

int pre_odor_duration = 5000;
int odor_duration = 5000;
int trace_duration = 5000;
int reward_duration = 5000;
int post_reward_duration = 5000;
int licks_per_reward = 3;

int current_time = 0;
int reward_start_time = 0;
int lickState = 0;
int lastLickState = 0;
int lickCount = 0;
int rewardCount = 0;
int last_rewarded_lick = 0;
int timeStamp = 0;
bool waterAvailability = false;

bool runTrial = false;
bool same_as_last_loop = false;
char secretSignal = 's';

void setup() {
  Serial.begin(9600);
  pinMode(lickSensor, INPUT);
  pinMode(odorValve, OUTPUT);
  pinMode(waterValve, OUTPUT);  

  //all valves and sensors are closed initially  
  digitalWrite(lickSensor, HIGH); //enable pullup resistor
  digitalWrite(waterValve, LOW);
  digitalWrite(odorValve, LOW); 
  Wire.begin();

  mpr121_setup();
}

//to keep track of various time points in the experiment
int total_recording_time = pre_odor_duration + odor_duration + trace_duration + reward_duration + post_reward_duration;

void loop() {
  
  if(runTrial == false && same_as_last_loop == false){
    Serial.print("Enter 1 to start recording:     ");
    same_as_last_loop = true;
  }
  
  if (Serial.available()>0){
      secretSignal = Serial.read();
      Serial.println(secretSignal);
      
      if(secretSignal == 49){  // 49 is the ASCII code for the digit 1
        runTrial = true;
        same_as_last_loop = false;
        Serial.println("Trial starting now.");
      }
      else{
        runTrial = false;
        Serial.print("Enter 1 to start recording:     ");
      }
    }
  
  int start_time = millis();
  
  while(runTrial == true){ 
    current_time = millis() - start_time;
    timeStamp = current_time/1000;
    //to open the odor valve for a given amount of time
    
    
    if(current_time == pre_odor_duration){
      digitalWrite(odorValve, HIGH);
      Serial.print(timeStamp);
      Serial.println("     Odor turned ON.");    
    }
    else if(current_time >= pre_odor_duration && current_time - pre_odor_duration < odor_duration){
      digitalWrite(odorValve, HIGH);
    }
    else if(current_time == pre_odor_duration + odor_duration){
      digitalWrite(odorValve, LOW);
      Serial.print(timeStamp);
      Serial.println("     Odor turned OFF.");
    }    
    else{
      digitalWrite(odorValve, LOW);    
    }
    
    //to enable water delivery during reward period
    reward_start_time = pre_odor_duration + odor_duration + trace_duration;
    if(current_time == reward_start_time){ 
      waterAvailability = true;
      Serial.print(timeStamp);
      Serial.println("     Water turned ON.");
    }
    else if(current_time >= reward_start_time && current_time - reward_start_time  < reward_duration){
      waterAvailability = true;
    }
    else if(current_time == reward_start_time + reward_duration){
      waterAvailability = false;
      Serial.print(timeStamp);
      Serial.println("     Water turned OFF.");      
    }
    else{
      waterAvailability = false;
    } 
  
    lickState = readTouchInputs();
    
    //this if statement is to keep track of licking
    if (lickState == HIGH && lastLickState == LOW) {
      lickCount =lickCount + 1;
      Serial.print(timeStamp);
      Serial.print("  Lick # ");
      Serial.println(lickCount);   
      
      lastLickState = lickState;
    }
    
    else {
      lastLickState = lickState;
    }
    
    //to decide whether or not to give a water reward
    if (waterAvailability == true && lickCount % licks_per_reward == 0 && lickCount != last_rewarded_lick){
      digitalWrite(waterValve, HIGH);
      delay(30);
      digitalWrite(waterValve, LOW);
      last_rewarded_lick = lickCount;
      rewardCount = rewardCount + 1;
      Serial.print(timeStamp);
      Serial.print("            REWARD # ");
      Serial.println(rewardCount);
    }
  	
    //this if statement terminates the data recording session
    if (current_time > total_recording_time){
      String goodbye = "I am done. Goodbye Arduino !"; 
      Serial.println(goodbye);
      runTrial = false; //this will stop the while loop
    }
  }
  lickCount = 0;
  rewardCount = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
/* following part of the code was modified by nj2292 from the original file named mpr121.ino
 provided by the manufacturer of the MPR121 lick sensor on their website
 March 30, 2014
 modified by: nj2292@columbia.edu


 Copyright (c) 2010 bildr community

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/

bool readTouchInputs(){
  if(!checkInterrupt()){    
    //read the touch state from the MPR121
    Wire.requestFrom(0x5A,2); 
    
    byte LSB = Wire.read();
    byte MSB = Wire.read();
    
    uint16_t touched = ((MSB << 8) | LSB); //16bits that make up the touch states
    
    for (int i=0; i < 12; i++){  // Check what electrodes were pressed
      if(touched & (1<<i)){      
        if(touchStates[i] == 0)       
          return true;
        
        touchStates[i] = 1;      
      }
        touchStates[i] = 0;
      }
    }
   return false;
}

void mpr121_setup(void){

  set_register(0x5A, 0x5D, 0x00); 
  
  // Section A - Controls filtering when data is > baseline.
  set_register(0x5A, 0x2B, 0x01);
  set_register(0x5A, 0x2C, 0x01);
  set_register(0x5A, 0x2D, 0x00);
  set_register(0x5A, 0x2E, 0x00);

  // Section B - Controls filtering when data is < baseline.
  set_register(0x5A, 0x2F, 0x01);
  set_register(0x5A, 0x30, 0x01);
  set_register(0x5A, 0x31, 0xFF);
  set_register(0x5A, 0x32, 0x02);
  
  // Section C - Sets touch and release thresholds for electrode 0
  // which detects licks on the lick port
  //the remaining electrodes (from 1 to 11) are not being used
  set_register(0x5A, 0x41, TOU_THRESH);
  set_register(0x5A, 0x42, REL_THRESH);
  
  // Section D
  // Set the Filter Configuration
  // Set ESI2
  set_register(0x5A, 0x5D, 0x04);
  
  // Section E
  // Electrode Configuration
  // Set ELE_CFG to 0x00 to return to standby mode
  set_register(0x5A, 0x5E, 0x0C);  // Enables all 12 Electrodes
  set_register(0x5A, 0x5E, 0x0C);
}

boolean checkInterrupt(void){
  return digitalRead(lickSensor);
}

void set_register(int address, unsigned char r, unsigned char v){
    Wire.beginTransmission(address);
    Wire.write(r);
    Wire.write(v);
    Wire.endTransmission();
}

//hello from my mac





//checking mac connection



