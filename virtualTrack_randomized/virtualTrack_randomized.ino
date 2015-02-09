#include <Arduino.h>
#include <Wire.h>
#include "parameter_definitions.h"

/*
  This Arduino sketch is for reading rotary encoder analog input on pin A10 and lick port signals 
  and then controlling water and odor delivery according to the signals.
 */

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
  
  //we define that these pins will be for output signals
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

  prepare_lickport(); //prepares the lick_port for reading licks
}


//try to use as few global variables as possible
bool lick = false;
bool last_lick = false;
int lick_count = 0;
int last_rewarded_lick = 0;
int reward = 0;
int rewardCount = 0;
int scope_ttl_pulse = 0;
int last_ttl = 0;
int ttl_count = 0;
int imaging_trigger = 0;
int lap_count = 0;
int last_rewarded_lap = -1;
int last_rewardCount = 0;
int last_lap_total_rewards = 0;
int initial_drop_status = 0;
int drop_count = 0;
int last_drop_count = 0;
int lick_count_at_reward_location = 0;
int lick_rate = 0;
int lick_count_at_last_second = 0;
int last_open_valve = 0;
int laps_in_envA = 0;
int environment = 0;
int location_at_last_valve_switch = 0;

//variables for keeping track of distance recorded by rotary encoder
float lastSensorValue = 0.0;
float distance = 0.0;
float last_distance = 0.0;
float totalDistance = 0.0;

//these are the points along the track where various odors turn ON/OFF:
float track = 4000.0; //length (in mm) of the virtual track
float reward_location = 4000/track;
float begin_first_odor = 1000/track;
float end_first_odor = 1500/track;
float begin_second_odor = 2000/track;
float end_second_odor = 2500/track;
float begin_third_odor = 3000/track;
float end_third_odor = 3500/track;

int first_odor = 1;
int second_odor = 2;
int third_odor = 1;

int envA_odor1 = 1;
int envA_odor2 = 2;
int envA_odor3 = 1;

int envB_odor1 = 3;
int envB_odor2 = 2;
int envB_odor3 = 3;


int envC_odor1 = 0;
int envC_odor2 = 0;
int envC_odor3 = 0;

int envD_odor1 = 0;
int envD_odor2 = 0;
int envD_odor3 = 0;

//various time variables
unsigned long start_time = 0.0;
unsigned long current_time = 0.0;
unsigned long water_valve_close_time = 0.0;
unsigned long reward_window_end = 0.0;
unsigned long lap_completion_time = 0.0;

int licks_per_reward = 3;
unsigned long drop_size = 30.0; //to determine drop size (in ms)
long initial_drop = 0.0; //to determine initial drop size (in ms)
long envA_initial_drop = 0.0;
long envB_initial_drop = 0.0;
long envC_initial_drop = 0.0;
long envD_initial_drop = 0.0;

unsigned long reward_window = 5.0; //in seconds
unsigned long recordingDuration = 50.0; //(recording duration in seconds)
unsigned long durationInEnvA = 25.0;
unsigned long durationInEnvB = 25.0;
unsigned long durationInEnvC = 25.0;
unsigned long durationInEnvD = 25.0;

int portStatus = 0;
int max_lap_count = 20;
int envA_max_lap_count = 10;
int envB_max_lap_count = 10;
int envC_max_lap_count = 10;
int envD_max_lap_count = 10;

// the loop routine runs over and over again forever:
void loop() {
  if (Serial.available()>0){
    char cue_to_start_trial = Serial.read();
    
    if(cue_to_start_trial == 49){  // 49 is the ASCII code for the digit 1
      //read the parameter values sent from the python GUI
      drop_size          =        Serial.parseInt();  
      licks_per_reward   =        Serial.parseInt();             
      reward_window      =        Serial.parseInt();      
      track              =        Serial.parseInt();  
      
      durationInEnvA     =        Serial.parseInt();
      envA_initial_drop  =        Serial.parseInt();
      envA_max_lap_count =        Serial.parseInt();
      envA_odor1         =        Serial.parseInt();
      envA_odor2         =        Serial.parseInt();
      envA_odor3         =        Serial.parseInt();
      
      durationInEnvB     =        Serial.parseInt();
      envB_initial_drop  =        Serial.parseInt();
      envB_max_lap_count =        Serial.parseInt();
      envB_odor1         =        Serial.parseInt();
      envB_odor2         =        Serial.parseInt();
      envB_odor3         =        Serial.parseInt();

      durationInEnvC     =        Serial.parseInt();
      envC_initial_drop  =        Serial.parseInt();
      envC_max_lap_count =        Serial.parseInt();
      envC_odor1         =        Serial.parseInt();
      envC_odor2         =        Serial.parseInt();
      envC_odor3         =        Serial.parseInt();    
  
      durationInEnvD     =        Serial.parseInt();
      envD_initial_drop  =        Serial.parseInt();
      envD_max_lap_count =        Serial.parseInt();
      envD_odor1         =        Serial.parseInt();
      envD_odor2         =        Serial.parseInt();
      envD_odor3         =        Serial.parseInt();


      durationInEnvA = durationInEnvA * 1000;  // s to ms
      durationInEnvB = durationInEnvA + durationInEnvB * 1000;  // s to ms
      durationInEnvC = durationInEnvB + durationInEnvC * 1000;  // s to ms
      durationInEnvD = durationInEnvC + durationInEnvD * 1000;  // s to ms
      
      recordingDuration = durationInEnvF;
      reward_window = reward_window * 1000.0;   // s to ms
      max_lap_count = envA_max_lap_count + envB_max_lap_count + envC_max_lap_count + envD_max_lap_count;
      
      //initialize global variable values for the current trial
      lick_count = 0;
      last_rewarded_lick = 0;
      reward = 0;
      rewardCount = 0;
      ttl_count = 0;
      last_ttl = 0;
      distance = 0;
      last_distance = 0;
      last_rewarded_lap = -1;
      totalDistance = 0;
      lap_count = 0;
      last_rewardCount = 0;
      last_lap_total_rewards = 0;
      initial_drop = 0;
      initial_drop_status = 0;
      drop_count = 0;
      last_drop_count = 0;
      lick_count_at_reward_location = 0;
      water_valve_close_time = 0;
      reward_window_end = 0;
      lap_completion_time = 0;
      lick_rate = 0;
      lick_count_at_last_second = 0;
      last_open_valve = 0;
      laps_in_envA = 0;
      environment = 0;
      location_at_last_valve_switch = 0;
  
      //now all ready to start the trial
      start_time = millis();
      lastSensorValue = analogRead(A10); //to make sure that distance measurement starts from 0 mm
      digitalWrite(arduino_to_scope, LOW); //start imaging
      
      while(true){
        current_time = millis() - start_time; //get the current time in this trial
        readPosition(); //read absolute position on rotary encoder and also calculate the corresponding virtual position

        //this if statement terminates the data recording session
        //when the max number of laps have been reached or when recording duration ends
        //it also makes sure that recording does not end during reward delivery
        //if the mouse stops moving before arriving at the rewarded region, wait an extra 120s before ending the recording session
        if ((lap_count >= max_lap_count)||
           (current_time > (recordingDuration + 120000)) ||
           ((reward_window_end > recordingDuration) && (whether_in_reward_window() == false))){
             end_trial();
             break;
        }

        //if the water valve is in opened state, check if it's time to close the valve
        if(current_time >= water_valve_close_time){
          digitalWrite(water_valve, LOW);
        }
        
        //to determine which environment should be ON  
        if(lap_completion_time < durationInEnvA && lap_count < envA_max_lap_count){
          first_odor   = envA_odor1;
          second_odor  = envA_odor2;
          third_odor   = envA_odor3;
          environment = 1;
          initial_drop = envA_initial_drop;
        }
        else if ((lap_completion_time > durationInEnvA || lap_count >= envA_max_lap_count) && (environment == 1)){
          first_odor   = envB_odor1;
          second_odor  = envB_odor2;
          third_odor   = envB_odor3;
          environment = 2;
          initial_drop = envB_initial_drop;
          envB_max_lap_count = lap_count + envB_max_lap_count;
        }
        else if ((lap_completion_time > durationInEnvB || lap_count >= envB_max_lap_count) && (environment == 2)){
          first_odor   = envC_odor1;
          second_odor  = envC_odor2;
          third_odor   = envC_odor3;
          environment = 3;
          initial_drop = envC_initial_drop;
          envC_max_lap_count = lap_count + envC_max_lap_count;
        }
        else if ((lap_completion_time > durationInEnvC || lap_count >= envC_max_lap_count) && (environment == 3)){
          first_odor   = envD_odor1;
          second_odor  = envD_odor2;
          third_odor   = envD_odor3;
          environment = 4;
          initial_drop = envD_initial_drop;
          envD_max_lap_count = lap_count + envD_max_lap_count;
        }
        
        lickCounter(); //keep track of licks        
        odorControl();

        //control water delivery
        if(whether_in_reward_window()) {
          determineReward();
        }
          
        //calculate current lick rate every second ( = number of licks in the last second)
        if(current_time % 1000 == 0){
          lick_rate = lick_count - lick_count_at_last_second;
          lick_count_at_last_second = lick_count;
        }
        
        //read the ttl pulse coming in from the microscope
        readTTL();
         
        //read the status of the imaging trigger signal; it should be ON throughout the recording session
        imaging_trigger = digitalRead(arduino_to_scope); 
        
        //save data every 100ms (rate is higher if there is other activity)
        if (current_time % 100 == 0 || abs(distance - last_distance) > 2){
          printer();
          last_distance = distance;
        }
      }
    }
  }
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////detect and keep track of licks//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

void lickCounter(){
  bool lick_sensor = readTouchInputs(); //to detect licks
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


void giveReward(unsigned long reward_size){
  reward = 1;
  rewardCount = rewardCount + 1;
  last_rewarded_lick = lick_count;
  digitalWrite(water_valve, HIGH);
  water_valve_close_time = current_time + reward_size;
  printer();
  reward = 0;  
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
  // read the input on analog pin A0:
  float sensorValue = analogRead(A10);
  //sensorValue = (-1) * sensorValue; //to assign which direction is positive rotation
  
  //this gives a measure of how much the rotary encoder moved
  //also, reduces random fluctuations due to noise in the reading
  float change = sensorValue - lastSensorValue;
  
  //these 'if' statements correct for the cyclic changes in sensorValue to make them linear
  //I chose 500 as the change value because it seems to adequately account for all abrupt changes
  //where the readouts change from, lets say, 1023 to 91 (instead of stepwise change from 1023 to 0)
  if (change < -500) change = (1023-lastSensorValue)+sensorValue;
  if (change > 500) change = (sensorValue-1023)-lastSensorValue;
  
  // Convert the analog reading (which goes from 0 to 1023) to a distance in mm (0 to 650 mm):
  float rotation_step = change * (650/1023.0);

  totalDistance = totalDistance + rotation_step / 1000.0;
  distance = distance + rotation_step;
  
  //restart the virtual track when the reward period ends after the mouse gets to the reward location
  //or when the mouse runs a full lap of the virtual track
  if((distance >= reward_location * track && whether_in_reward_window() == false) || (distance >= track)){
    distance = 0;
    lap_count = lap_count + 1; //lap_count increases only if the mouse moves in forward direction
    lap_completion_time = current_time;
    last_lap_total_rewards = rewardCount - last_rewardCount;
    last_rewardCount = rewardCount;
    last_drop_count = drop_count;  //to keep track of the number of initial drops delivered up to this point
  } 
  
  if (distance*(-1) > track){  //if the mouse is moving in backward direction
    distance = distance + track;
  }    

  //update the last sensor value once the cycle is complete
  lastSensorValue = sensorValue; 
}


/////////////////////water delivery mechanism/////////////////////////////
//////reward is delivered if the mouse gets to a particular location//////
//////and it also has to lick at a given rate to get water drops//////////
bool whether_in_reward_window(){
  //in the current form, only positive movement leads to a reward
  //rewarded region of the virtual track starts at (reward_location * track)
  if (distance > reward_location * track && last_rewarded_lap != lap_count){
   reward_window_end = current_time + reward_window;
   lick_count_at_reward_location = lick_count; 
   last_rewarded_lick = -1;
   last_rewarded_lap = lap_count;
  }
  
  //reward window duration resets after the mouse gets its first reward
  if (lick_count - lick_count_at_reward_location == licks_per_reward){
   reward_window_end = current_time + reward_window;
  }  
  
  //reward available only if the mouse remains within the the rewarded region of the virtual track
  return ((current_time <= reward_window_end) && 
          (distance >= reward_location * track) && 
          (distance <= end_fourth_odor * track));
}

void determineReward(){
  //for the very first water reward in the current lap
  if(drop_count == last_drop_count && initial_drop > 0){
     initial_drop_status = 1;
     drop_count = drop_count + 1;
     giveReward(initial_drop);
     initial_drop_status = 0;
  }
  
  //if the mouse didn't get any rewards in the last lap, give an initial drop in this lap
//  if(drop_count == last_drop_count && initial_drop == 0 && last_lap_total_rewards < 1 && lap_count > 0){
//     initial_drop_status = 1;
//     drop_count = drop_count + 1;
//     giveReward(drop_size);
//     initial_drop_status = 0;
//  }
 
  //give reward only if the reward window is on AND the mouse is licking at the specified lick_rate
  int lick_count_in_reward_region = lick_count - lick_count_at_reward_location;
  if(lick_count_in_reward_region  > 0 &&
     lick_count_in_reward_region  % licks_per_reward == 0 && 
     lick_rate >= licks_per_reward &&
     lick_count != last_rewarded_lick){
        giveReward(drop_size);
      }
}


void odorControl() {
  
  int location = int(distance);
  
  if ((location > (begin_first_odor * track)) && (location <= (end_first_odor *track))){
    valveSwitch(first_odor, location);
  }  
  else if ((location > (begin_second_odor * track)) && (location <= (end_second_odor * track ))){
    valveSwitch(second_odor, location);
  }
  else if ((location > (begin_third_odor * track)) && (location <= (end_third_odor * track))){
    valveSwitch(third_odor, location);
  }
  else{    //close all odor valves if motion is in negative direction or if somehow distance is > track
    valveSwitch(0, location);
  }
}

void valveSwitch(int valve, int current_location){
  // noise in the distance measurement can cause rapid switching of valves
  // to prevent that, we will wait for 10mm change in distance from last valve switch
  if(valve != last_open_valve && abs(current_location - location_at_last_valve_switch) > 10){
    last_open_valve = valve;
    location_at_last_valve_switch = current_location;
    valveOperator(valve);
    printer ();
  }
}

void valveOperator(int valve){
  //first, close (LOW==OFF) all odor valves
  digitalWrite(odor_A, LOW);
  digitalWrite(odor_B, LOW);
  digitalWrite(odor_C, LOW);
  digitalWrite(odor_D, LOW);
  digitalWrite(odor_E, LOW);
  digitalWrite(odor_F, LOW);
  //mineral oil is ON (LOW==ON) when all odors are OFF
  digitalWrite(mineral_oil, LOW);
  
  if (valve == 1){
    digitalWrite(odor_A, HIGH);      //turn on odor A
    digitalWrite(mineral_oil, HIGH); // turn off mineral oil
    portStatus = 1;
  }
  else if (valve == 2){
    digitalWrite(odor_B, HIGH);
    digitalWrite(mineral_oil, HIGH);
    portStatus = 2;
  }
  else if (valve == 3){
    digitalWrite(odor_C, HIGH);
    digitalWrite(mineral_oil, HIGH);
    portStatus = 3;
  }
  else if (valve == 4){
    digitalWrite(odor_D, HIGH);
    digitalWrite(mineral_oil, HIGH);
    portStatus = 4;
  }
  else if (valve == 5){
    digitalWrite(odor_E, HIGH);
    digitalWrite(mineral_oil, HIGH);
    portStatus = 5;
  }
  else if (valve == 6){
    digitalWrite(odor_F, HIGH);
    digitalWrite(mineral_oil, HIGH);
    portStatus = 6;
  }  
  else{
    digitalWrite(mineral_oil, LOW);
    portStatus = 0;
  }    
}


void printer (){
  //could also display these parameters: lick, reward, change, rotation_step, displacement, reward_window_status
  String comma = ",";
  
  String dataLog= current_time + comma + 
                  portStatus + comma + 
                  lick + comma + 
                  lick_count + comma + 
                  reward + comma + 
                  rewardCount + comma + 
                  int(distance) + comma + 
                  int(totalDistance) + comma + 
                  whether_in_reward_window() + comma + 
                  initial_drop_status + comma + 
                  drop_count + comma + 
                  imaging_trigger + comma + 
                  scope_ttl_pulse + comma + 
                  ttl_count + comma + 
                  lick_rate + comma +
                  lap_count + comma +
                  environment;
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
  portStatus = 0;
  
  //stop imaging
  digitalWrite(arduino_to_scope, HIGH);
  
  //this while loop is to make sure that we don't miss the ttl pulse for the last frame
  while(millis() < current_time + 1000){
    readTTL();
    delay(1);
  }
  
  //this print statement tells the python script to stop listening to Arduino
  Serial.println("8128");
}
