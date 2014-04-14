#include <Wire.h>
#include "parameter_definitions.h"

int current_time = 0;
int timeStamp = 0;
int reward_start_time = 0;
int lickCount = 0;
int rewardCount = 0;
int last_rewarded_lick = 0;

bool lickState = false;
bool lastLickState = false;
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
  
  prepare_lickport();
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
      delay(reward_size);
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

