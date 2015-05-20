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

//these variables are for specifying the points along the track_length where various odors turn ON/OFF:
float track_odor_region = 400;

float track_length = 4000.0; //length (in mm) of the virtual track_length
float envA_track_length = 2000.0;
float envB_track_length = 0.0;
float envC_track_length = 0.0;
float envD_track_length = 0.0;

int first_odor = 1;
int second_odor = 2;
int third_odor = 1;

float first_odor_location = 500;
float second_odor_location = 1000;
float third_odor_location = 1500;

long first_odor_duration = 500; 
long second_odor_duration = 500; 
long third_odor_duration = 500;

long first_odor_start_time = 0; 
long second_odor_start_time = 0; 
long third_odor_start_time = 0;

int two_env_random_sequence[] = {2,1,2,1,1,2,1,2,2,1,2,1,2,1,2,2,1,2,1,1,2,2,1,1,2,1,1,2,2,1,1,2,1,2,2,1,2,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,2,1,2,2,1,2,1,1,2,1,2,1,2,2,1,2,1,1,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,2,2,1,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,2,2,1,2,1,1,2,2,1,1,2,1,1,2,2,1,1,2,1,2,2,1,2,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,2,1,2,2,1,2,1,1,2,1,2,1,2,2,1,2,1,1,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,2,2,1,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,2,2,1,2,1,1,2,2,1,1,2,1,1,2,2,1,1,2,1,2,2,1,2,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,2,1,2,2,1,2,1,1,2,1,2,1,2,2,1,2,1,1,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,2,2,1,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,2,2,1,2,1,1,2,2,1,1,2,1,1,2,2,1,1,2,1,2,2,1,2,2,1,1,2,1,2,1,2,1,2,1,2,1,1,2,2,1,2,2,1,2,1,1,2,1,2,1,2,2,1,2,1,1,1,2,2,1,1,2,2,1,2,1,2,1,1,2,2,1,2,2,1,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1,2,1,2,1,1,2,1,2,2,1};
//int three_env_random_sequence[] = {1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3,1,2,3};
int three_env_random_sequence[] = {2,1,3,3,1,2,3,2,1,2,3,1,1,3,2,2,3,1,1,3,2,3,1,2,2,1,3,2,3,1,2,1,3,2,1,3,2,3,1,1,3,2,3,1,2,3,2,1,2,3,1,3,1,2,1,3,2,1,3,2,3,1,2,3,2,1,3,1,2,3,1,2,1,2,3,2,1,3,1,2,3,3,2,1,2,1,3,2,1,3,3,2,1,3,1,2,3,1,2,2,3,1,2,1,3,2,1,3,3,2,1,2,1,3,3,1,2,2,1,3,2,1,3,3,1,2,3,2,1,2,3,1,1,3,2,2,3,1,1,3,2,3,1,2,2,1,3,2,3,1,2,1,3,2,1,3,2,3,1,1,3,2,3,1,2,3,2,1,2,3,1,3,1,2,1,3,2,1,3,2,3,1,2,3,2,1,3,1,2,3,1,2,1,2,3,2,1,3,1,2,3,3,2,1,2,1,3,2,1,3,3,2,1,3,1,2,3,1,2,2,3,1,2,1,3,2,1,3,3,2,1,2,1,3,3,1,2,2,1,3,2,1,3,3,1,2,3,2,1,2,3,1,1,3,2,2,3,1,1,3,2,3,1,2,2,1,3,2,3,1,2,1,3,2,1,3,2,3,1,1,3,2,3,1,2,3,2,1,2,3,1,3,1,2,1,3,2,1,3,2,3,1,2,3,2,1,3,1,2,3,1,2,1,2,3,2,1,3,1,2,3,3,2,1,2,1,3,2,1,3,3,2,1,3,1,2,3,1,2,2,3,1,2,1,3,2,1,3,3,2,1,2,1,3,3,1,2,2,1,3,2,1,3,3,1,2,3,2,1,2,3,1,1,3,2,2,3,1,1,3,2,3,1,2,2,1,3,2,3,1,2,1,3,2,1,3,2,3,1,1,3,2,3,1,2,3,2,1,2,3,1,3,1,2,1,3,2,1,3,2,3,1,2,3,2,1,3,1,2,3,1,2,1,2,3,2,1,3,1,2,3,3,2,1,2,1,3,2,1,3,3,2,1,3,1,2,3,1,2,2,3,1,2,1,3,2,1,3,3,2,1,2,1,3,3,1,2,2,1,3};
//int four_env_random_sequence[]  = {1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4,1,2,3,4};
int four_env_random_sequence[]  = {1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4,3,4,1,2,3,4,1,2,1,2,3,4,3,4,1,2,1,2,3,4};


int envA_odor1 = 1;
int envA_odor2 = 2;
int envA_odor3 = 1;

int envB_odor1 = 3;
int envB_odor2 = 2;
int envB_odor3 = 3;

int envC_odor1 = 1;
int envC_odor2 = 0;
int envC_odor3 = 0;

int envD_odor1 = 3;
int envD_odor2 = 0;
int envD_odor3 = 0;


float envA_odor1_location = 500;
float envA_odor2_location = 1000;
float envA_odor3_location = 1500;

float envB_odor1_location = 0;
float envB_odor2_location = 0;
float envB_odor3_location = 0;

float envC_odor1_location = 0;
float envC_odor2_location = 0;
float envC_odor3_location = 0;

float envD_odor1_location = 0;
float envD_odor2_location = 0;
float envD_odor3_location = 0;

long envA_odor1_duration = 500;      
long envA_odor2_duration = 500; 
long envA_odor3_duration = 500;

long envB_odor1_duration = 0;      
long envB_odor2_duration = 0; 
long envB_odor3_duration = 0;

long envC_odor1_duration = 0;      
long envC_odor2_duration = 0; 
long envC_odor3_duration = 0;

long envD_odor1_duration = 0;      
long envD_odor2_duration = 0; 
long envD_odor3_duration = 0;

//various time variables
unsigned long start_time = 0.0;
unsigned long current_time = 0.0;
unsigned long water_valve_close_time = 0.0;
unsigned long reward_window_end = 0.0;
unsigned long time_at_last_lap_completion = 0.0;

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
int envA_lap_count = 0;
int envA_max_lap_count = 10;
int envB_lap_count = 0;
int envB_max_lap_count = 10;
int envC_lap_count = 0;
int envC_max_lap_count = 10;
int envD_lap_count = 0;
int envD_max_lap_count = 10;
int random_sequence = 0;
int last_env_switch = 0;

// the loop routine runs over and over again forever:
void loop() {
  if (Serial.available()>0){
    char cue_to_start_trial = Serial.read();
    
    if(cue_to_start_trial == 49){  // 49 is the ASCII code for the digit 1
      //read the parameter values sent from the python GUI
      drop_size          =        Serial.parseInt();  
      licks_per_reward   =        Serial.parseInt();             
      reward_window      =        Serial.parseInt();      
      recordingDuration  =        Serial.parseInt();
      random_sequence    =        Serial.parseInt();
      
      envA_initial_drop  =        Serial.parseInt();
      envA_max_lap_count =        Serial.parseInt();
      envA_track_length  =        Serial.parseInt();
      envA_odor1         =        Serial.parseInt();
      envA_odor2         =        Serial.parseInt();
      envA_odor3         =        Serial.parseInt();      
      envA_odor1_location=        Serial.parseInt();
      envA_odor2_location=        Serial.parseInt();
      envA_odor3_location=        Serial.parseInt();
      envA_odor1_duration=        Serial.parseInt();      
      envA_odor2_duration=        Serial.parseInt(); 
      envA_odor3_duration=        Serial.parseInt(); 
      
      envB_initial_drop  =        Serial.parseInt();
      envB_max_lap_count =        Serial.parseInt();
      envB_track_length  =        Serial.parseInt();
      envB_odor1         =        Serial.parseInt();
      envB_odor2         =        Serial.parseInt();
      envB_odor3         =        Serial.parseInt();      
      envB_odor1_location=        Serial.parseInt();
      envB_odor2_location=        Serial.parseInt();
      envB_odor3_location=        Serial.parseInt();
      envB_odor1_duration=        Serial.parseInt();      
      envB_odor2_duration=        Serial.parseInt(); 
      envB_odor3_duration=        Serial.parseInt(); 

      envC_initial_drop  =        Serial.parseInt();
      envC_max_lap_count =        Serial.parseInt();
      envC_track_length  =        Serial.parseInt();
      envC_odor1         =        Serial.parseInt();
      envC_odor2         =        Serial.parseInt();
      envC_odor3         =        Serial.parseInt();      
      envC_odor1_location=        Serial.parseInt();
      envC_odor2_location=        Serial.parseInt();
      envC_odor3_location=        Serial.parseInt();
      envC_odor1_duration=        Serial.parseInt();      
      envC_odor2_duration=        Serial.parseInt(); 
      envC_odor3_duration=        Serial.parseInt(); 
      
      envD_initial_drop  =        Serial.parseInt();
      envD_max_lap_count =        Serial.parseInt();
      envD_track_length  =        Serial.parseInt();
      envD_odor1         =        Serial.parseInt();
      envD_odor2         =        Serial.parseInt();
      envD_odor3         =        Serial.parseInt();      
      envD_odor1_location=        Serial.parseInt();
      envD_odor2_location=        Serial.parseInt();
      envD_odor3_location=        Serial.parseInt();
      envD_odor1_duration=        Serial.parseInt();      
      envD_odor2_duration=        Serial.parseInt(); 
      envD_odor3_duration=        Serial.parseInt(); 

   
      recordingDuration =  recordingDuration * 1000;;
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
      time_at_last_lap_completion = 0;
      lick_rate = 0;
      lick_count_at_last_second = 0;
      last_open_valve = 0;
      laps_in_envA = 0;
      environment = 0;
      location_at_last_valve_switch = 0;
      last_env_switch = -1;
      envA_lap_count = 0;
      envB_lap_count = 0;
      envC_lap_count = 0;
      envD_lap_count = 0;
      first_odor_start_time = 0;
      second_odor_start_time = 0;
      third_odor_start_time = 0;
      
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
        
        if(lap_count > last_env_switch && random_sequence == 1){
          sequentialEnvironmentControl();
          last_env_switch = lap_count;
        }
        else if(lap_count > last_env_switch && random_sequence == 2){
          randomizedEnvironmentControl(two_env_random_sequence[lap_count]);
          last_env_switch = lap_count;
        }
        else if(lap_count > last_env_switch && random_sequence == 3){
          randomizedEnvironmentControl(three_env_random_sequence[lap_count]);
          last_env_switch = lap_count;
        }
        else if(lap_count > last_env_switch && random_sequence == 4){
          randomizedEnvironmentControl(four_env_random_sequence[lap_count]);
          last_env_switch = lap_count;
        }
        else if (random_sequence == 0){
          sequentialEnvironmentControl();
        }   
        
        lickCounter(); //keep track_length of licks        
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
/////////////////detect and keep track_length of licks//////////////////////////////////////////////
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
  
  //this function checks to see if the distance needs to be reset (because it's become longer than track_length length)
  reset_distance();

  //update the last sensor value once the cycle is complete
  lastSensorValue = sensorValue; 
}


/////////////////////water delivery mechanism/////////////////////////////
//////reward is delivered if the mouse gets to a particular location//////
//////and it also has to lick at a given rate to get water drops//////////
bool whether_in_reward_window(){
  //in the current form, only positive movement leads to a reward
  //rewarded region of the virtual track_length starts at the end of the track_length
  if ((distance >= track_length) && last_rewarded_lap != lap_count){
   reward_window_end = current_time + reward_window;
   lick_count_at_reward_location = lick_count; 
   last_rewarded_lick = -1;
   last_rewarded_lap = lap_count;
  }
  
  //reward window duration resets after the mouse gets its first reward
  if (lick_count - lick_count_at_reward_location == licks_per_reward){
   reward_window_end = current_time + reward_window;
  }  
  
  //reward available only if the mouse remains within the the rewarded region of the virtual track_length
  return ((current_time < reward_window_end) && (distance >= track_length)); 
}

void determineReward(){
  //for the very first water reward in the current lap
  if(drop_count == last_drop_count && initial_drop > 0){
     initial_drop_status = 1;
     drop_count = drop_count + 1;
     giveReward(initial_drop);
     initial_drop_status = 0;
  }
 
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

  if(first_odor_location > 0 && first_odor_start_time <= time_at_last_lap_completion){
    if(location > first_odor_location){
      first_odor_start_time = current_time;
    }
  }
  if(second_odor_location > 0 && second_odor_start_time <= time_at_last_lap_completion){
    if(location > second_odor_location){
      second_odor_start_time = current_time;
    }
  }
  if(third_odor_location > 0 && third_odor_start_time <= time_at_last_lap_completion){
    if(location > third_odor_location){
      third_odor_start_time = current_time;
    }
  }
  
  //if ((location > first_odor_location) && (location < first_odor_location + track_odor_region) && (current_time < first_odor_start_time + first_odor_duration)){
  if ((location > first_odor_location) && (current_time < first_odor_start_time + first_odor_duration)){
    valveSwitch(first_odor, location);
  }  
  else if ((location > second_odor_location) && (current_time < second_odor_start_time + second_odor_duration)){
    valveSwitch(second_odor, location);
  }
  else if ((location > third_odor_location) && (current_time < third_odor_start_time + third_odor_duration)){
    valveSwitch(third_odor, location);
  }
  else{    //close all odor valves if motion is in negative direction or if somehow distance is > track_length
    valveSwitch(0, location);
  }
}

void valveSwitch(int valve, int current_location){
  // noise in the distance measurement can cause rapid switching of valves
  // to prevent that, we will wait for 1mm change in distance from last valve switch
  if(valve != last_open_valve && abs(current_location - location_at_last_valve_switch) > 1){
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

void reset_distance(){
  //restart the virtual track_length when the reward period ends after the mouse gets to the reward location
  //or when the mouse runs a full lap of the virtual track_length
  if((distance >= track_length && whether_in_reward_window() == false) || (distance > 1.25 * track_length)){
    distance = 0;
    lap_count = lap_count + 1; //lap_count increases only if the mouse moves in forward direction
    time_at_last_lap_completion = current_time;
    last_lap_total_rewards = rewardCount - last_rewardCount;
    last_rewardCount = rewardCount;
    last_drop_count = drop_count;  //to keep track_length of the number of initial drops delivered up to this point
    //Serial.println("distance reset");
  }
  
  if (distance < 0){  //if the mouse is moving in backward direction, reset distance to zero
    distance = 0;
  }    
}

void sequentialEnvironmentControl(){
  //to determine which environment should be ON  
  if(lap_count < envA_max_lap_count){
    environment = 1;
    initial_drop = envA_initial_drop;
    track_length = envA_track_length;

    first_odor   = envA_odor1;
    second_odor  = envA_odor2;
    third_odor   = envA_odor3;

    first_odor_location = envA_odor1_location;
    second_odor_location = envA_odor2_location;
    third_odor_location = envA_odor3_location;

    first_odor_duration = envA_odor1_duration; 
    second_odor_duration = envA_odor2_duration; 
    third_odor_duration = envA_odor3_duration; 
  }
  else if (lap_count >= envA_max_lap_count && lap_count < envA_max_lap_count + envB_max_lap_count){
    environment = 2;
    initial_drop = envB_initial_drop;
    track_length = envB_track_length;

    first_odor   = envB_odor1;
    second_odor  = envB_odor2;
    third_odor   = envB_odor3;

    first_odor_location = envB_odor1_location;
    second_odor_location = envB_odor2_location;
    third_odor_location = envB_odor3_location;

    first_odor_duration = envB_odor1_duration; 
    second_odor_duration = envB_odor2_duration; 
    third_odor_duration = envB_odor3_duration; 
  }
  else if (lap_count >= envA_max_lap_count + envB_max_lap_count && lap_count < envA_max_lap_count + envB_max_lap_count + envC_max_lap_count){
    environment = 3;
    initial_drop = envC_initial_drop;
    track_length = envC_track_length;

    first_odor   = envC_odor1;
    second_odor  = envC_odor2;
    third_odor   = envC_odor3;

    first_odor_location = envC_odor1_location;
    second_odor_location = envC_odor2_location;
    third_odor_location = envC_odor3_location;

    first_odor_duration = envC_odor1_duration; 
    second_odor_duration = envC_odor2_duration; 
    third_odor_duration = envC_odor3_duration; 
  }
  else if (lap_count >= envA_max_lap_count + envB_max_lap_count + envC_max_lap_count){
    environment = 4;
    initial_drop = envD_initial_drop;
    track_length = envD_track_length;

    first_odor   = envD_odor1;
    second_odor  = envD_odor2;
    third_odor   = envD_odor3;

    first_odor_location = envD_odor1_location;
    second_odor_location = envD_odor2_location;
    third_odor_location = envD_odor3_location;

    first_odor_duration = envD_odor1_duration; 
    second_odor_duration = envD_odor2_duration; 
    third_odor_duration = envD_odor3_duration; 
  }
  else{
    Serial.println(lap_count);
    Serial.println("Reached maximum number of laps.");
  }
}

void randomizedEnvironmentControl(int next_env){
  //to determine which environment should be ON  
  if(next_env == 1 && envA_lap_count < envA_max_lap_count){
    environment = 1;
    initial_drop = envA_initial_drop;
    track_length = envA_track_length;
    envA_lap_count = envA_lap_count + 1;

    first_odor   = envA_odor1;
    second_odor  = envA_odor2;
    third_odor   = envA_odor3;

    first_odor_location = envA_odor1_location;
    second_odor_location = envA_odor2_location;
    third_odor_location = envA_odor3_location;

    first_odor_duration = envA_odor1_duration; 
    second_odor_duration = envA_odor2_duration; 
    third_odor_duration = envA_odor3_duration; 
  }
  else if(next_env == 2 && envB_lap_count < envB_max_lap_count){
    environment = 2;
    initial_drop = envB_initial_drop;
    track_length = envB_track_length;
    envB_lap_count = envB_lap_count + 1;

    first_odor   = envB_odor1;
    second_odor  = envB_odor2;
    third_odor   = envB_odor3;

    first_odor_location = envB_odor1_location;
    second_odor_location = envB_odor2_location;
    third_odor_location = envB_odor3_location;

    first_odor_duration = envB_odor1_duration; 
    second_odor_duration = envB_odor2_duration; 
    third_odor_duration = envB_odor3_duration; 
  }
  else if(next_env == 3 && envC_lap_count < envC_max_lap_count){
    environment = 3;
    initial_drop = envC_initial_drop;
    track_length = envC_track_length;
    envC_lap_count = envC_lap_count + 1;

    first_odor   = envC_odor1;
    second_odor  = envC_odor2;
    third_odor   = envC_odor3;

    first_odor_location = envC_odor1_location;
    second_odor_location = envC_odor2_location;
    third_odor_location = envC_odor3_location;

    first_odor_duration = envC_odor1_duration; 
    second_odor_duration = envC_odor2_duration; 
    third_odor_duration = envC_odor3_duration; 
  }
  else if(next_env == 4 && envD_lap_count < envD_max_lap_count){
    environment = 4;
    initial_drop = envD_initial_drop;
    track_length = envD_track_length;    
    envD_lap_count = envD_lap_count + 1;

    first_odor   = envD_odor1;
    second_odor  = envD_odor2;
    third_odor   = envD_odor3;

    first_odor_location = envD_odor1_location;
    second_odor_location = envD_odor2_location;
    third_odor_location = envD_odor3_location;

    first_odor_duration = envD_odor1_duration; 
    second_odor_duration = envD_odor2_duration; 
    third_odor_duration = envD_odor3_duration; 
  }
  else{
    Serial.println(next_env);
    Serial.println("Could not randomize");
    sequentialEnvironmentControl();  
  }
}

void printer (){
  //could also display these parameters: lick, reward, change, rotation_step, displacement, reward_window_status
  String comma = ",";
  
  String dataLog= ttl_count + comma + 
                  current_time + comma + 
                  portStatus + comma + 
                  lick_count + comma + 
                  rewardCount + comma + 
                  drop_count + comma + 
                  whether_in_reward_window() + comma + 
                  int(distance) + comma + 
                  int(totalDistance) + comma + 
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
