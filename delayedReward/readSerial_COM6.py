#!/usr/bin/python
import serial
import time
import datetime
ser = serial.Serial('/dev/ttyACM0',9600)

mouse_name = raw_input('Enter mouse name: ')

def num(s):
    try:
        return int(s)
    except ValueError:
        return -1
	return
	
def timeStamped(fname, fmt='{fname}_%Y-%m-%d-%Hh-%Mm-%Ss'):
    return datetime.datetime.now().strftime(fmt).format(fname=fname)

def display_and_saveData():
	print 'Recording data now.'
	time.sleep(1)
	
	while True:
		currentInput = ser.readline().rstrip('\n')		#rstrip('\n') removes blank lines
		restart_cue = num(currentInput)
		if (restart_cue == 8128):
			print 'End of trial'
			break
		print currentInput
		dataLog.write(currentInput)
	return


while True:
	signal = raw_input('Enter 1 to start recording or 0 to quit: ')
	signal = num(signal)
	if(signal == 0):
		break
		
	if(signal == 1):
		trial_number = raw_input('Trial number: ')
		filename = mouse_name + "_trial" + trial_number
		filename = timeStamped(filename)
		filename = filename + ".txt"
		
		print 'Name of this file is: ' + filename
		dataLog = open(filename, "a+")
		header = 'Time,Licks,Reward,Odor,RewardWindow,LickCount,RewardCount\n'
		dataLog.write(header)
		
		ser.write(str(signal))
		display_and_saveData()
		dataLog.close()
	else:
		print 'Invalid input. Try again.'
	time.sleep(1)
