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

