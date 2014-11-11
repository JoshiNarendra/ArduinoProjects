#include <Wire.h>
#include "parameter_definitions.h"

//save data less frequently if nothing is happening
//add more comments in general
/*
  This Arduino sketch is for reading rotary disc and lick port signals and then controlling
  water and odor delivery according to the signals.
  
  Reads rotary disc analog input on pin A10 (the 10-bit values are between 0 and 1023), 
  converts it into distance in millimeters. Outer circumference of the rotary disc is about 250 millimeters.
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
  digitalWrite(arduino_to_scope, LOW);
  digitalWrite(scope_to_arduino, LOW);  

  prepare_lickport(); 
}

bool lick = false;
bool lick_sensor = false; // lickStatus is 1 when lick occurs and 0 otherwise
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
int laps_with_initial_drop = 0;
int lick_rate = 0;
int lick_count_at_last_second = 0;
int last_open_valve = 0;

int runCode = 0;
int first_loop = 0;
int recordingDuration = 10; //(recording duration in seconds)

float sensorValue = 0.0;
float lastSensorValue = 0.0;
float change = 0.0;  //amount of change in rotary encoder value during one completion of while loop
float rotation_step = 0.0;
float distance = 0.0;
float totalDistance = 0.0;


//assuming that the virtual track is 5000 mm long
//these are the points along the track where the odors turn ON/OFF:
float track = 4500.0; //length (in mm) of the virtual track
float reward_location = 3750/track; //reward becomes available halfway into the second stretch of odor A
float begin_odor_C = 250/track;
float end_odor_C = 1000/track;
float begin_odor_A1 = 1250/track;
float end_odor_A1 = 2000/track;
float begin_odor_B = 2250/track;
float end_odor_B = 3000/track;
float begin_odor_A2 = 3250/track;
float end_odor_A2 = 4480/track; //note that the actual track length can be longer than the specified track value

//unsigned long timeStamp = 0.0; //this generates a time stamp for recording data
unsigned long start_time = 0.0;
unsigned long current_time = 0.0;
unsigned long water_valve_close_time = 0.0;
unsigned long reward_window_end = 0.0;
unsigned long time_at_last_valve_switch = 0.0;

int licks_per_reward = 3;
unsigned long drop_size = 30.0; //to determine drop size (in ms)
unsigned long initial_drop = 0.0; //to determine initial drop size (in ms)
unsigned long reward_window = 5.0; //in seconds
int max_lap_count = 20;
int portStatus = 0;

String comma = ",";
String dataLog = "BLANK";
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
    laps_with_initial_drop =    Serial.parseInt();
    recordingDuration  =        Serial.parseInt();
    reward_window      =        Serial.parseInt();
    max_lap_count      =        Serial.parseInt();
    track              =        Serial.parseInt();     
    
    recordingDuration = recordingDuration * 1000;  // s to ms
    reward_window = reward_window * 1000.0;   // s to ms
    //decay_constant = decay_constant / 10.0;
    
    start_time = millis();
    digitalWrite(arduino_to_scope, HIGH); //start imaging
    
    first_loop = 1;
    lick_count = 0;
    last_rewarded_lick = 0;
    reward = 0;
    rewardCount = 0;
    ttl_count = 0;
    last_ttl = 0;
    change = 0;
    distance = 0;
    last_rewarded_lap = -1;
    totalDistance = 0;
    lap_count = 0;
    last_rewardCount = 0;
    last_lap_total_rewards = 0;
    initial_drop_status = 0;
    drop_count = 0;
    last_drop_count = 0;
    lick_count_at_reward_location = 0;
    water_valve_close_time = 0;
    reward_window_end = 0;
    lick_rate = 0;
    lick_count_at_last_second = 0;
    last_open_valve = 0;
  }
  
  while(runCode == 1){
    current_time = millis() - start_time;
    
    //to detect licks
    lick_sensor = readTouchInputs();
    lickCounter();
    
    //read absolute position on rotary disc and also calculate the corresponding virtual position
    readPosition();
    
    //end the trial if the max number of laps have been reached
    if (lap_count >= max_lap_count){
      end_trial();
      break;
    }

    //to turn various odor valves on or off
    odorControl(distance);
    
//    if (lap_count <= max_lap_count/2 - 1){
//      odorControl(distance);
//    }
//    else{
//      digitalWrite(odor_A, LOW);
//      digitalWrite(odor_B, LOW);
//      digitalWrite(odor_C, LOW);
//      digitalWrite(mineral_oil, LOW);
//      portStatus = 0;
//    } 
      
    //control water delivery
    if(whether_in_reward_window()) {
      determineReward();
    }
      
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
     
    //read the status of the imaging trigger signal; it should be ON throughout the recording session
    imaging_trigger = digitalRead(arduino_to_scope); 
    
    //save data every 10ms (rate is higher if there is other activity)
    if (current_time % 10 <= 1){
      printer();
    }
    
    //this if statement terminates the data recording session
    //also makes sure that recording does not end during reward delivery
    //if the mouse stops moving before arriving at the rewarded region, wait an extra minute before ending the recording session
    if ((lap_count == max_lap_count)|| 
       (current_time > recordingDuration + 120000) ||
       (reward_window_end > recordingDuration) && (current_time > reward_window_end) ||
       (current_time > recordingDuration) && (distance > end_odor_A2 * track)){
         end_trial();
         break;
    }
  }
}


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////detect and keep track of licks//////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

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
  sensorValue = analogRead(A10);
  //sensorValue = (-1) * sensorValue; //to assign which direction is positive rotation
  
  //this makes sure that our distance measurement starts at 0.0cm
  if (first_loop == 1){
    lastSensorValue = sensorValue;
    first_loop = 0;
  }
  
  //this gives a measure of how much the rotary encoder moved
  //also, reduces random fluctuations due to noise in the reading
  change = sensorValue -lastSensorValue;
  
  //these 'if' statements correct for the cyclic changes in sensorValue to make them linear
  //I chose 500 as the change value because it seems to adequately account for all abrupt changes
  //where the readouts change from, lets say, 1023 to 91 (instead of stepwise change from 1023 to 0)
  if (change < -500) change = (1023-lastSensorValue)+sensorValue;
  if (change > 500) change = (sensorValue-1023)-lastSensorValue;
  
  // Convert the analog reading (which goes from 0 to 1023) to a distance in mm (0 to 650 mm):
  rotation_step = change * (650/1023.0);

  totalDistance = totalDistance + rotation_step;
  distance = distance + rotation_step;
  
  //restart the virtual track when the reward period ends after the mouse gets to the reward location
  //or when the mouse runs a full lap of the virtual track
  if((distance >= reward_location * track && whether_in_reward_window() == false) || (distance >= track)){
    distance = 0;
    lap_count = lap_count + 1; //lap_count increases only if the mouse moves in forward direction   
    last_lap_total_rewards = rewardCount - last_rewardCount;
    last_rewardCount = rewardCount;
    last_drop_count = drop_count;  //to keep track of the number of initial drops delivered up to this point
  }
  
//  //this resets every time the mouse runs a full lap of the virtual track
//  if (distance > track){
//    distance = distance - track; 
//    lap_count = lap_count + 1; //lap_count increases only if the mouse moves in forward direction
////    if (lap_count >= max_lap_count){ //stop the recording when 20 laps are completed
////      runCode = 0;
////    }
//    last_lap_total_rewards = rewardCount - last_rewardCount;
//    last_rewardCount = rewardCount;
//    last_drop_count = drop_count;  //to keep track of the number of initial drops delivered up to this point
//    printer();
//  }
  
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
  
  //reward window duration starts after the mouse gets its first reward
  if (lick_count - lick_count_at_reward_location == licks_per_reward){
   reward_window_end = current_time + reward_window;
  }  
  

  //reward available only if the mouse remains within the the rewarded region of the virtual track
  return ((current_time <= reward_window_end) && 
          (distance >= reward_location * track) && 
          (distance <= end_odor_A2 * track));
}

void determineReward(){
  //give reward only if the reward window is on AND the mouse is licking 

  //for the very first water reward in the current lap
  if(drop_count == last_drop_count && initial_drop > 0){
    if(lap_count < laps_with_initial_drop){
       initial_drop_status = 1;
       drop_count = drop_count + 1;
       giveReward(initial_drop);
       initial_drop_status = 0;
    }
    
    else if(last_lap_total_rewards < 1 && lap_count > 0){
       initial_drop_status = 1;
       drop_count = drop_count + 1;
       giveReward(initial_drop);
       initial_drop_status = 0;
     }
  }
 
   //for the remaining rewards in the current lap
  int lick_count_in_reward_region = lick_count - lick_count_at_reward_location;
  if(lick_count_in_reward_region  > 0 &&
     lick_count_in_reward_region  % licks_per_reward == 0 && 
     lick_rate >= licks_per_reward &&
     lick_count != last_rewarded_lick){
        giveReward(drop_size);
      }
}



void odorControl(float curr_distance) {
  if ((curr_distance >  0) && (curr_distance <= (begin_odor_C *track))){
    valveSwitch(0);
  }
  else if ((curr_distance > (begin_odor_C * track)) && (curr_distance <= (end_odor_C *track))){
    valveSwitch(3);
  }
  else if ((curr_distance > (end_odor_C * track)) && (curr_distance <= (begin_odor_A1 * track))){
    valveSwitch(0);
  }  
  else if ((curr_distance > (begin_odor_A1 * track)) && (curr_distance <= (end_odor_A1 * track ))){
    valveSwitch(1);
  }
  else if ((curr_distance > (end_odor_A1 * track )) && (curr_distance <= (begin_odor_B * track))){
    valveSwitch(0);
  }
  else if ((curr_distance > (begin_odor_B * track)) && (curr_distance <= (end_odor_B * track))){
    valveSwitch(2);
  }
  else if ((curr_distance > (end_odor_B * track)) && (curr_distance <= (begin_odor_A2 * track))){
    valveSwitch(0);
  }
  else if ((curr_distance > (begin_odor_A2 * track)) && (curr_distance <= (end_odor_A2 *track))){
    valveSwitch(1);
  }  
  else if ((curr_distance > (end_odor_A2 * track)) && (curr_distance <= track)){
    valveSwitch(0);
  }
  
  /////for movement in negative direction/////////
  else if ((curr_distance >  (0 - track)) && (curr_distance <= ((begin_odor_C - 1)*track))){
    valveSwitch(0);
  }
  else if ((curr_distance > ((begin_odor_C - 1)*track)) && (curr_distance <= ((end_odor_C - 1)*track))){
    valveSwitch(3);
  }
  else if ((curr_distance > ((end_odor_C - 1)*track)) && (curr_distance <= ((begin_odor_A1 - 1)*track))){
    valveSwitch(0);
  }  
  else if ((curr_distance > ((begin_odor_A1 - 1)*track)) && (curr_distance <= ((end_odor_A1 - 1)*track))){
    valveSwitch(1);
  }
  else if ((curr_distance > ((end_odor_A1 - 1)*track)) && (curr_distance <= ((begin_odor_B - 1)*track))){
    valveSwitch(0);
  }
  else if ((curr_distance > ((begin_odor_B - 1)*track)) && (curr_distance <= ((end_odor_B - 1)*track))){
    valveSwitch(2);
  }
  else if ((curr_distance > ((end_odor_B - 1)*track)) && (curr_distance <= ((begin_odor_A2 - 1)*track))){
    valveSwitch(0);
  }
  else if ((curr_distance > ((begin_odor_A2 - 1)*track)) && (curr_distance <= ((end_odor_A2 - 1)*track))){
    valveSwitch(1);
  }  
  else if ((curr_distance > ((end_odor_A2 - 1)*track)) && (curr_distance <= 0)){
    valveSwitch(0);
  }
  else{
    valveSwitch(0);
  }
}

void valveSwitch(int valve){
  if(valve != last_open_valve){  // && current_time - time_at_last_valve_switch > 100){
    last_open_valve = valve;
    valveOperator(valve);
  }
}

void valveOperator(int valve){
  time_at_last_valve_switch = current_time;
  
  if (valve == 1){
    digitalWrite(odor_A, HIGH);
    digitalWrite(odor_B, LOW);
    digitalWrite(odor_C, LOW);
    digitalWrite(mineral_oil, HIGH);
    portStatus = 1;
  }
  else if (valve == 2){
    digitalWrite(odor_A, LOW);
    digitalWrite(odor_B, HIGH);
    digitalWrite(odor_C, LOW);
    digitalWrite(mineral_oil, HIGH);
    portStatus = 2;
  }
  else if (valve == 3){
    digitalWrite(odor_A, LOW);
    digitalWrite(odor_B, LOW);
    digitalWrite(odor_C, HIGH);
    digitalWrite(mineral_oil, HIGH);
    portStatus = 3;
  }
  else{
    digitalWrite(odor_A, LOW);
    digitalWrite(odor_B, LOW);
    digitalWrite(odor_C, LOW);
    digitalWrite(mineral_oil, LOW);
    portStatus = 0;
  }    
}



void printer (){
  //timeStamp = millis() - start_time; //timeStamp in milliseconds
  //could also display these parameters: lick, reward, change, rotation_step, displacement, reward_window_status
  dataLog = current_time + comma + 
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
            lap_count;
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
  digitalWrite(arduino_to_scope, LOW); //stop imaging
  Serial.println("8128");
  runCode = 0;
}
  


