#include <Arduino.h>
#include <Wire.h>
#include "parameter_definitions.h"

/*
  This code reads an analog input on pin A0 (the 10-bit values are between 0 and 1023), 
  converts it into distance in centimeters and prints the result to the serial monitor.
  
  Attach the green pin of the rotary encoder to pin A0, the black one to the ground and 
  the red one to +5V.
  
  outer circumference of the rotary disc is about 400 millimeters
  the virtual circular track is 4 meters (4000 mm) in circumference
 */
 
int water_valve = 36; //water valve

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 115200 bits per second (bauds)
  // a large baud value allows us to sample and store data at high frequency
  Serial.begin(115200);
  
  //we define that the led pins and Hbridge pins will give output signal
  pinMode(water_valve, OUTPUT);      
  
  //the lickSensor will provide input to the arduino
  //lickSensor pin (pin 13) is assigned in the parameter_definitions.h file
  pinMode(lickSensor, INPUT);
  
  //all valves are turned off initially
  digitalWrite(water_valve, LOW);
  
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

int runCode = 0;
int first_loop = 0;

int sensorValue = 0;
int lastSensorValue = 0;
float change = 0;  //amount of change in rotary encoder value during one completion of while loop
float rotation_step = 0;
float distance = 0;
float totalDistance = 0;
int reward = 0;
int rewardCount = 0;
int run_lick = 0;
int lick_time = 0;
int track_length = 100; //(in mm), minimum distance to be covered to get a reward
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
//bool reward_window_status = false;

String comma = ",";
char classified_code = 'c';

// the loop routine runs over and over again forever:
void loop() {
  
  //If using the Arduino Serial Moniotor, press 1 and then Enter to start trial
  if (Serial.available()>0){
    classified_code = Serial.read();
    
    if(classified_code == 49){  // 49 is the ASCII code for the digit 1
      runCode = 1;
    }

    drop_size          =        Serial.parseInt();  
    licks_per_reward   =        Serial.parseInt();    
    initial_drop       =        Serial.parseInt(); 
    recordingDuration  =        Serial.parseInt();
    reward_window      =        Serial.parseInt();
    track_length       =        Serial.parseInt();    
  
    
    recordingDuration = recordingDuration * 1000;  // s to ms
    reward_window = reward_window * 1000;   // s to ms
     
    start_time = millis();
    first_loop = 1;
    lick_count = 0;
    last_rewarded_lick = 0;
    reward = 0;
    rewardCount = 0;
    ttl_count = 0;
    last_ttl = 0;
    change = 0;
    distance = 0;
    lap_count = 0;
    last_rewarded_lap = 0;
    totalDistance = 0;
    current_time = 0;
    reward_window_end = 0;
    water_valve_close_time = 0;
    lick_rate = 0;
    lick_count_at_last_second = 0;
 
  }
  
  while(runCode == 1){
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
    
    //save data at least every 10 ms when nothing else is happening.
    if (current_time % 10 == 0) printer();    
    
    //this if statement terminates the data recording session when the last lap completes
    //there is a time extension ( of 200000ms) available for the mouse to complete its current lap
    if ((current_time > (recordingDuration + 200000)) || 
       ((reward_window_end > recordingDuration) && (current_time > reward_window_end))){
      digitalWrite(water_valve, LOW);
      Serial.println("8128");
      runCode = 0; //this will stop the while loop
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


void readPosition(){

  // read the position input from rotary encoder on analog pin A0:
  sensorValue = analogRead(A10);
  //sensorValue = (-1) * sensorValue; //to assign which direction is positive rotation
  
  //this makes sure that our distance measurement starts at 0mm
  if (first_loop == 1){
    lastSensorValue = sensorValue;
    first_loop = 0;
  }
  
  //this gives a measure of how much the rotary encoder moved
  change = sensorValue - lastSensorValue;
  
  //these 'if' statements correct for the cyclic changes in sensorValue to make them linear
  //I chose 500 as the change value because it seems to adequately account for all abrupt changes
  //where the readouts change from, lets say, 1023 to 91 (instead of stepwise change from 1023 to 0)
  if (change < -500) change = (1023-lastSensorValue) + sensorValue;
  if (change > 500) change = (sensorValue-1023) - lastSensorValue;
  
  //the rotary disc circumference is 650 mm (65 cm)
  // Convert the analog reading (which goes from 0 to 1023) to a distance in mm (0 to 650 mm):
  rotation_step = change * (650/1023.0);
  //if (abs(change) < 5) change = 0;
  
  //distance = distance + rotation_step;
  totalDistance = totalDistance + rotation_step;
  distance = distance + rotation_step;
 
  //this resets every time the mouse runs a full lap of the virtual track
  if (distance > track_length){
    distance = distance - track_length; 
    lap_count = lap_count + 1; //lap_count increases only if the mouse moves in forward direction
    printer();
  }
  
  if (distance*(-1) > track_length){  //if the mouse is moving in backward direction
    distance = distance + track_length;
  }

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
  String dataLog = current_time + comma + 
                   0 + comma + 
                   lick + comma + 
                   lick_count + comma + 
                   reward + comma + 
                   rewardCount + comma + 
                   int(distance) + comma + 
                   int(totalDistance) + comma + 
                   (current_time < reward_window_end) + comma + 
                   0 + comma + 
                   0 + comma + 
                   0 + comma + 
                   0 + comma + 
                   0 + comma + 
                   lick_rate + comma + 
                   lap_count;
  Serial.println(dataLog);
}
  


