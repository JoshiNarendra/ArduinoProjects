#!/usr/bin/python
import serial;
					
dataLog = open("rotaryEncoderLog.txt", "a+");
ser = serial.Serial('/dev/ttyACM0',9600);

while True:
	currentInput = ser.readline();
	print currentInput;
	dataLog.write(currentInput);

#what is life if full of care
#no time to stand and stare

#testing123
#lets say this code is a heap of unintelligible and non-functional bits of text





#!/usr/bin/python
import serial;
import time;

dataLog = open("delayedRewardLog.txt", "a+");
ser = serial.Serial('COM6',9600,);

currentInput = 1
print currentInput

print 'I want to write something to serial port'

while True:
	signal = input('Enter 1 to start recording: ')
	ser.write(str(signal))
	print 'Recording data now.'
	time.sleep(1)

	
	while True:
		currentInput = ser.readline()
		if (currentInput == 'x'):
			break
		print currentInput
		dataLog.write(currentInput)	
