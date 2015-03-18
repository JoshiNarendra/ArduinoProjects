#include <Arduino.h>
#include <Wire.h>
#include "parameter_definitions.h"

int water_valve = 36; //water valve
int odor_A = 22; //odor valve A
int odor_B = 24; //odor valve B
int odor_C = 26; //odor valve C
int odor_D = 28;
int odor_E = 30;
int odor_F = 32;
int mineral_oil = 34; //mineral oil is turned ON when all the odors are OFF
int arduino_to_scope = 40;
int scope_to_arduino = 44;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 115200 bits per second (bauds)
  // a large baud value allows us to sample and store data at high frequency
  Serial.begin(115200);
  
  //we define that the led pins and Hbridge pins will give output signal
  pinMode(water_valve, OUTPUT);      
  pinMode(odor_A, OUTPUT);
  pinMode(odor_B, OUTPUT); 
  pinMode(odor_C, OUTPUT); 
  pinMode(odor_D, OUTPUT);
  pinMode(odor_E, OUTPUT); 
  pinMode(odor_F, OUTPUT);  
  pinMode(mineral_oil, OUTPUT);
  
  pinMode(arduino_to_scope, OUTPUT);
  pinMode(scope_to_arduino, INPUT); 
  
  //the lickSensor will provide input to the arduino
  //lickSensor pin (pin 13) is assigned in the parameter_definitions.h file
  pinMode(lickSensor, INPUT);
  
  //all valves are turned off initially
  digitalWrite(water_valve, LOW);
  digitalWrite(odor_A, LOW);
  digitalWrite(odor_B, LOW);
  digitalWrite(odor_C, LOW);
  digitalWrite(odor_D, LOW);
  digitalWrite(odor_E, LOW);
  digitalWrite(odor_F, LOW);
  digitalWrite(mineral_oil, LOW);
  digitalWrite(arduino_to_scope, HIGH);
  digitalWrite(scope_to_arduino, LOW); 
  
  prepare_lickport();
  
}

bool lick = false;
bool lick_sensor = false; // lickStatus is 1 when lick occurs and 0 otherwise
bool last_lick = false;
int lick_count = 0;
int last_rewarded_lick = 0;
int scope_ttl_pulse = 0;
int last_ttl = 0;
int ttl_count = 0;
int imaging_trigger = 0;

float lastSensorValue = 0;
float distance = 0;
float last_distance = 0.0;
float totalDistance = 0;
float track_length = 100; //(in mm), minimum distance to be covered to get a reward

int reward = 0;
int rewardCount = 0;
int run_lick = 0;
int lick_time = 0;

int lap_count = 0;
int last_rewarded_lap = 0;
int lick_rate = 0;
int lick_count_at_last_second = 0;

unsigned long start_time = 0.0; //time when a recording trial starts
unsigned long current_time = 0.0; // the current time at any point during a trial
unsigned long water_valve_close_time = 0.0; //time when a water valve should close after delivering a water reward
unsigned long reward_window_end = 0.0;

int licks_per_reward = 3;
unsigned long drop_size = 30.0; //to determine drop size (in ms)
unsigned long initial_drop = 0.0; //to determine initial drop size (in ms)
unsigned long reward_window = 1.0; //in seconds
unsigned long recordingDuration = 10; //(recording duration in seconds)

// the loop routine runs over and over again forever:
void loop() {
  if (Serial.available()>0){
    char cue_to_start_trial = Serial.read();
    
    if(cue_to_start_trial == 49){  // 49 is the ASCII code for the digit 1
      //read the parameter values sent from the python GUI
      drop_size          =        Serial.parseInt();  
      licks_per_reward   =        Serial.parseInt();    
      initial_drop       =        Serial.parseInt(); 
      recordingDuration  =        Serial.parseInt();
      reward_window      =        Serial.parseInt();
      track_length       =        Serial.parseInt();    
      
      recordingDuration = recordingDuration * 1000;  // s to ms
      reward_window = reward_window * 1000;   // s to ms
       
      lick_count = 0;
      last_rewarded_lick = 0;
      reward = 0;
      rewardCount = 0;
      ttl_count = 0;
      last_ttl = 0;
      distance = 0;
      last_distance = 0;
      lap_count = 0;
      last_rewarded_lap = 0;
      totalDistance = 0;
      current_time = 0;
      reward_window_end = 0;
      water_valve_close_time = 0;
      lick_rate = 0;
      lick_count_at_last_second = 0;
      
      start_time = millis();
      lastSensorValue = analogRead(A10); //to make sure that distance measurement starts from 0 mm
      digitalWrite(arduino_to_scope, LOW); //start imaging
    
      while(true){
        current_time = millis() - start_time;
        
        //to detect licks
        lick_sensor = readTouchInputs();
        lickCounter();
        
        //read absolute position on rotary disc and also calculate the corresponding virtual position
        readPosition();
       
        //control water delivery
        determineReward();
    
        //if the water valve was opened to start a reward, close the valve at the appropriate time
        if(current_time >= water_valve_close_time){
          digitalWrite(water_valve, LOW); 
        }
    
        //calculate current lick rate every second ( = number of licks in the last second)
        if(current_time % 1000 == 0){
          lick_rate = lick_count - lick_count_at_last_second;
          lick_count_at_last_second = lick_count;
        }
    
        //read the ttl pulse coming in from the microscope
        readTTL();
        
        imaging_trigger = digitalRead(arduino_to_scope);
        
        //save data every 100ms (rate is higher if there is other activity)
        if (current_time % 100 == 0 || abs(distance - last_distance) > 10){
          printer();
          last_distance = distance;
        }  
        
        //this if statement terminates the data recording session when the last lap completes
        //there is a time extension ( of 200000ms) available for the mouse to complete its current lap
        if ((current_time > (recordingDuration + 200000)) || 
           ((reward_window_end > recordingDuration) && (current_time > reward_window_end))){
             end_trial();
             break;
        }
      }
    }
  }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////functions///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////detect and keep track of licks////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void lickCounter(){
  if (lick_sensor == HIGH && last_lick == LOW) {
    lick = true;
    lick_count = lick_count + 1;
    printer();
    lick = false;    
    last_lick = HIGH;
  }
  else {
    lick = false;
    last_lick = lick_sensor;
  }
}

void readTTL(){
  scope_ttl_pulse = digitalRead(scope_to_arduino);
  
  if(scope_ttl_pulse == 1 && last_ttl == 0){
    ttl_count = ttl_count + scope_ttl_pulse;
    scope_ttl_pulse = ttl_count; //to save the current TTL count instead of just 1 or 0
    printer();
    last_ttl = 1;
    scope_ttl_pulse = 0;
  }
  else{
    last_ttl = scope_ttl_pulse;
    scope_ttl_pulse = 0; //to make sure that a TTL pulse gets registered only once
  }
}

void readPosition(){

  // read the position input from rotary encoder on analog pin A0:
  float sensorValue = analogRead(A10);
  //sensorValue = (-1) * sensorValue; //to assign which direction is positive rotation
  
  //this gives a measure of how much the rotary encoder moved
  float change = sensorValue - lastSensorValue;
  
  //these 'if' statements correct for the cyclic changes in sensorValue to make them linear
  //I chose 500 as the change value because it seems to adequately account for all abrupt changes
  //where the readouts change from, lets say, 1023 to 91 (instead of stepwise change from 1023 to 0)
  if (change < -500) change = (1023-lastSensorValue) + sensorValue;
  if (change > 500) change = (sensorValue-1023) - lastSensorValue;
  
  //the rotary disc circumference is 650 mm (65 cm)
  // Convert the analog reading (which goes from 0 to 1023) to a distance in mm (0 to 650 mm):
  float rotation_step = change * (650/1023.0);
  //if (abs(change) < 5) change = 0;
  
  //totalDistance is in meters
  totalDistance = totalDistance + (rotation_step / 1000.0);
  distance = distance + rotation_step;
 
  //this resets every time the mouse runs a full lap of the virtual track
  if (distance > track_length){
    distance = distance - track_length; 
    lap_count = lap_count + 1; //lap_count increases only if the mouse moves in forward direction
    printer();
  }

  if (distance < 0){  //if the disstance is negative, reset distance to zero
    distance = 0;
  }  
//  if (distance*(-1) > track_length){  //if the mouse is moving in backward direction
//    distance = distance + track_length;
//  }

  //update the last sensor value once the cycle is complete
  lastSensorValue = sensorValue;
  
}

//reward is delivered only if the mouse runs a certain distance and also licks//
///////////lets say, we will deliver water at every third lick//////////////////
void determineReward(){  
  //when a mouse completes a lap in forward direction, it can get reward for certain amount of time (reward_window)
  //if it licks at a certain rate (licks_per_reward)
  if (lap_count > last_rewarded_lap){
   if(initial_drop > 0) giveReward(initial_drop); //give an initial drop only if it is non-zero in size;
   reward_window_end = current_time + reward_window;
   last_rewarded_lap = lap_count;
  } 

  //give reward only if the reward window is on AND the mouse is licking
  if(current_time < reward_window_end && 
     lick_count % licks_per_reward == 0 && 
     lick_count > last_rewarded_lick){
       giveReward(drop_size);
     }
}


////to open the water valve for a duration of time = reward_size (in miliseconds)////
void giveReward(unsigned long reward_size){
  reward = 1;
  rewardCount = rewardCount + 1;
  last_rewarded_lick = lick_count;
  digitalWrite(water_valve, HIGH);
  water_valve_close_time = current_time + reward_size;
  printer();
  reward = 0;
}


void printer (){
  //timeStamp = millis() - start_time; //timeStamp in milliseconds
  //could also display these parameters: lick, reward, change, rotation_step, displacement, reward_window_status
  String comma = ",";
  
  String dataLog = ttl_count + comma + 
                   current_time + comma + 
                   0 + comma + 
                   lick_count + comma + 
                   rewardCount + comma + 
                   lap_count + comma + //here, initial drop count is same as lap count
                   (current_time < reward_window_end) + comma + //reward window
                   int(distance) + comma + 
                   int(totalDistance) + comma + 
                   lap_count + comma + //initial drop count
                   1 ;//the environment variable
  Serial.println(dataLog);
}
  
void end_trial(){
  //turn OFF water valve and all odor valves
  digitalWrite(water_valve, LOW);
  digitalWrite(odor_A, LOW);
  digitalWrite(odor_B, LOW);
  digitalWrite(odor_C, LOW);
  digitalWrite(odor_D, LOW);
  digitalWrite(odor_E, LOW);
  digitalWrite(odor_F, LOW);  
  digitalWrite(mineral_oil, LOW);
  digitalWrite(arduino_to_scope, HIGH); //stop imaging
  
  //this while loop is to make sure that the ttl pulse for the last frame is not missed
  while(millis() < current_time + 1000){
    readTTL();
    delay(1);
  }
  
  //this print statement tells the python script to stop listening to Arduino
  Serial.println("8128");
}

