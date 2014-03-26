#!/usr/bin/python
import serial;

dataLog = open("rotaryEncoderLog.txt", "a+");
ser = serial.Serial('/dev/ttyACM0',9600);

while True:
	currentInput = ser.readline();
	print currentInput;
	dataLog.write(currentInput);
	
	
	//just to see if git edits can be done online. 
	//ae rajai lai bari lai
