<<<<<<< HEAD
#include "parameter_definitions.h"

int current_time = 0;
int time_stamp = 0;
int lick_count = 0;
int reward_count = 0;
int last_rewarded_lick = 0;

bool lick_sensor = false;
bool lick = false;
bool last_lick = false;
bool reward = false;
bool odor_window = false;
bool reward_window = false;

bool run_trial = false;
char classified_code = 's';

String data_point = "d";
String comma = ",";
=======
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
>>>>>>> 53d64ab0d3f65aa755339d041e24e52eb058a0f6

void setup() {
  Serial.begin(9600);
  pinMode(lickSensor, INPUT);
  pinMode(odorValve, OUTPUT);
  pinMode(waterValve, OUTPUT);  

  //all valves and sensors are closed initially  
  digitalWrite(lickSensor, HIGH); //enable pullup resistor
  digitalWrite(waterValve, LOW);
  digitalWrite(odorValve, LOW); 
  
<<<<<<< HEAD
=======
  prepare_lickport();
>>>>>>> 53d64ab0d3f65aa755339d041e24e52eb058a0f6
}

//to keep track of various time points in the experiment
int total_recording_time = pre_odor_duration + odor_duration + trace_duration + reward_duration + post_reward_duration;
<<<<<<< HEAD
int reward_start_time = pre_odor_duration + odor_duration + trace_duration;

//this void loop keeps running for multiple trials
void loop() {
  //If using the Arduino Serial Moniotor, press 1 and then Enter to start trial
  if (Serial.available()>0){
      classified_code = Serial.read();
      
      if(classified_code == 49){  // 49 is the ASCII code for the digit 1
        run_trial = true;
        //Serial.println("Trial srarting now.");
=======

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
>>>>>>> 53d64ab0d3f65aa755339d041e24e52eb058a0f6
      }
    }
  
  int start_time = millis();
  
<<<<<<< HEAD
  //this while loop repeats until the end of one trial
  while(run_trial == true){ 
    current_time = millis() - start_time;
    time_stamp = current_time/1000;
    
    //to open the odor valve for a given amount of time
    odorControl(current_time);
    
    //to enable water delivery during reward period
    reward_window = rewardWindowControl(current_time);

    //detect whether or not a lick has occured
    lick_sensor = digitalRead(lickSensor);
    
    //to keep track of the total number of licks
    lickCounter();
    
    //to decide whether or not to give a water reward
    deliverReward();
    
    //just a time stamp per second
//    if(current_time % 1000 == 0)
//      printer();
      
    //this if statement terminates the data recording session
    if (current_time > total_recording_time){
      Serial.println("8128");  //this is meant as a signal for python to end this recording and start a new one
      run_trial = false; //this will stop the while loop
    }
  }
  lick_count = 0;
  reward_count = 0;
}



bool odorControl(int current_time){
  
  if(current_time == pre_odor_duration){
    digitalWrite(odorValve, HIGH);
    odor_window = true;
    printer();     
  }
  
  else if(current_time >= pre_odor_duration && current_time - pre_odor_duration < odor_duration){
    digitalWrite(odorValve, HIGH);
    odor_window = true;
  }
  
  else if(current_time == pre_odor_duration + odor_duration){
    digitalWrite(odorValve, LOW);
    odor_window = false;
    printer(); 
  }
  
  else{
    digitalWrite(odorValve, LOW); 
    odor_window = false;   
  }
}


bool rewardWindowControl(int current_time){
  
  if(current_time == reward_start_time){ 
    reward_window = true;
    printer(); 
  }
  
  else if(current_time >= reward_start_time && current_time - reward_start_time  < reward_duration){
    reward_window = true;
  }
  
  else if(current_time == reward_start_time + reward_duration){
    reward_window = false;
    printer();      
  }
  
  else{
    reward_window = false;
  }
  
  return reward_window;
} 

void lickCounter(){
  
  if (lick_sensor == HIGH && last_lick == LOW) {
	lick = true;
    lick_count = lick_count + 1;
    printer();         
    last_lick = HIGH;
  }
  
  else {
	lick = false;
    last_lick = lick_sensor;
  }
}


void deliverReward(void){
  
  if (reward_window == true && lick_count % licks_per_reward == 0 && lick_count > last_rewarded_lick){
    digitalWrite(waterValve, HIGH);
    delay(reward_size);
    digitalWrite(waterValve, LOW);   
    last_rewarded_lick = lick_count;
    reward_count = reward_count + 1;
    
    reward = true;
    printer();
  }
  else{
    reward = false;
  }
}


void printer(){
  data_point = current_time + comma + lick + comma + reward + comma + odor_window + comma + reward_window + comma + lick_count + comma + reward_count;
  Serial.println(data_point);
=======
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
>>>>>>> 53d64ab0d3f65aa755339d041e24e52eb058a0f6
}
