int lickSensor = 5;
int odorValve = 2;
int waterValve =  13;

int odor_duration = 5000;
int pre_odor_duration = 30000;
int trace_duration = 20000;
int reward_duration = 20000;
int post_reward_duration = 10000;
int licks_per_reward = 5;

int current_time = 0;
int reward_start_time = 0;
int lickState = 0;
int lastLickState = 0;
int lickCount = 0;
int rewardCount = 0;
int last_rewarded_lick = 0;
int timeStamp = 0;
bool waterAvailability = false;

void setup() {
  Serial.begin(9600);
  pinMode(lickSensor, INPUT);
  pinMode(odorValve, OUTPUT);
  pinMode(waterValve, OUTPUT);  

  //all valves and sensors are closed initially  
  digitalWrite(lickSensor, LOW);
  digitalWrite(waterValve, LOW);
  digitalWrite(odorValve, LOW); 
}


//to keep track of various time points in the experiment
int start_time = millis();

void loop() {
  current_time = millis() - start_time;
  Serial.println(millis());
  Serial.println(current_time);
  
  //to open the odor valve for a given amount of time
  if(current_time >= pre_odor_duration && ((current_time - pre_odor_duration) < odor_duration)){
  digitalWrite(odorValve, HIGH);
  Serial.println("Odor valve turned ON");
  }
  else{
  digitalWrite(odorValve, LOW);  
  }
  
  //to enable water delivery during reward period
  reward_start_time = pre_odor_duration + odor_duration + trace_duration;
  Serial.println("Some fixed values:");
  Serial.println(pre_odor_duration);
  Serial.println(odor_duration);
  Serial.println(trace_duration);
  Serial.println(reward_start_time);
  if(current_time > reward_start_time && ((current_time - reward_start_time)  <= reward_duration)){
    waterAvailability = true;
    Serial.println("Water is available now....waaa");
  }
  else{
    waterAvailability = false;
    Serial.println("Water is not available....    :(");
  } 

  lickState = digitalRead(lickSensor);
  
  //this if statement is to keep track of licking
  if (lickState == HIGH && lastLickState == LOW) {
    lickCount =lickCount + 1;
    timeStamp = millis()/1000;
    Serial.print(timeStamp);
    Serial.print("  Lick # ");
    Serial.println(lickCount);   
      
    //initialize all ports to zero
    lastLickState = lickState;
    digitalWrite(lickSensor, LOW);
  }
  
  else {
    //initialize all ports to zero
    lastLickState = lickState;
    digitalWrite(lickSensor, LOW);
  }
  
  //to decide whether or not to give a water reward
  if (waterAvailability == true && ((lickCount % licks_per_reward) == 0) && lickCount != last_rewarded_lick){
    digitalWrite(waterValve, HIGH);
    delay(30);
	digitalWrite(waterValve, LOW);
	last_rewarded_lick = lickCount;
	rewardCount = rewardCount + 1;
	
	timeStamp = millis()/1000;
    Serial.print(timeStamp);
	Serial.print("            REWARD # ");
    Serial.println(rewardCount);
	}
  Serial.println("");
  Serial.println("");
}
