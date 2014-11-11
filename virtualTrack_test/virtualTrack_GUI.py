import Tkinter
import serial
import time
import datetime
import os, errno
ser = serial.Serial('/dev/ttyS4',115200)

#create a new window
window = Tkinter.Tk()
#set the window title
window.title("Rotary Disc Experiment")


mouse_label = Tkinter.Label(window, text="Mouse").grid(row=0) #create a label widget
mouse = Tkinter.Entry(window) #create a text entry widget
mouse.insert(0,"x") #insert default value for the mouse entry

reward_size_label = Tkinter.Label(window, text="Reward size(ms):").grid(row=1)
reward_size = Tkinter.Entry(window)
reward_size.insert(0,"60")

licks_per_reward_label = Tkinter.Label(window, text="Licks per reward:").grid(row=2)
licks_per_reward = Tkinter.Entry(window)
licks_per_reward.insert(0,"3")

initial_drop_label = Tkinter.Label(window, text="Initial drop size(ms):").grid(row=3)
initial_drop = Tkinter.Entry(window)
initial_drop.insert(0,"60")

drop_pattern_label = Tkinter.Label(window, text="Laps with initial drop:").grid(row=4)
drop_pattern = Tkinter.Entry(window, bg = 'light green')
drop_pattern.insert(0,"50")

recording_duration_label = Tkinter.Label(window, text="Recording duration(s):").grid(row=5)
recording_duration = Tkinter.Entry(window)
recording_duration.insert(0,"300")

reward_window_label = Tkinter.Label(window, text="Reward window(s):").grid(row=6)
reward_window = Tkinter.Entry(window)
reward_window.insert(0,"2")

max_lap_count_label = Tkinter.Label(window, text="Max number of laps:").grid(row=7)
max_lap_count = Tkinter.Entry(window, bg = 'light green')
max_lap_count.insert(0,"20")

track_label = Tkinter.Label(window, text="Length of virtual track(mm):").grid(row=8)
track = Tkinter.Entry(window, bg = 'light blue')
track.insert(0,"4500")

#arrange the entry fields in a grid layout
mouse.grid(row=0, column=1)
reward_size.grid(row=1, column=1)
licks_per_reward.grid(row=2, column=1)
initial_drop.grid(row=3, column=1)
drop_pattern.grid(row=4, column=1)
recording_duration.grid(row=5, column=1)
reward_window.grid(row=6, column=1)
max_lap_count.grid(row=7, column=1)
track.grid(row=8, column=1)


##########################################################################

def arduinoParameters():
	parameters = [mouse.get(), 					#0
				  reward_size.get(), 			#1
				  licks_per_reward.get(), 		#2
				  initial_drop.get(), 			#3
				  drop_pattern.get(),			#4
				  recording_duration.get(),		#5
				  reward_window.get(), 			#6
				  max_lap_count.get(),			#7
				  track.get()]					#8
	return parameters


def num(s):
    try:
        return int(s)
    except ValueError:
        return -1
	return
	
def timeStamp(fname, fmt='{fname}_%Y-%m-%d-%Hh-%Mm-%Ss'):
    return datetime.datetime.now().strftime(fmt).format(fname=fname)

def dir_timeStamp(fmt='%Y_%m_%d'):
    return datetime.datetime.now().strftime(fmt)
	
def arduinoCommunication(parameters):
	#this tells the arduino to start sending data and also provides parameter values

	duelist = arduinoParameters()
	arduino_parameter_update = '1,'+ duelist[1]+','+ duelist[2]+','+ duelist[3]+','+ duelist[4]+','+ duelist[5]+','+ duelist[6]+','+ duelist[7]+','+ duelist[8]
	ser.write(arduino_parameter_update)

def main():
	
	fyle = arduinoParameters()
	filename = timeStamp(fyle[0])
	filename = filename + ".csv"
	print 'Name of this file is: ' + filename
	
	#create a .csv file inside a folder for each mouse to save incoming data from the arduino
	dir_path = './Recordings/' + fyle[0] + '/' + fyle[0] + '_' + dir_timeStamp()
	filename = dir_path + '/' + filename
	if dir_path:
		if not os.path.isdir(dir_path):
			os.makedirs(dir_path)
		dataLog = open(filename, "a+")
	
	#record the detailed conditions of each trial
	trial_details = ('Mouse_name,'+ fyle[0] +
					',Reward_size,'+ fyle[1] + 
					',Licks_per_reward,'+ fyle[2]+
					',Initial_drop,'+ fyle[3]+
					',Laps_with_definite_drop,'+ fyle[4]+
					',Recording_duration,'+ fyle[5]+
					',Reward_window,'+ fyle[6]+
					',Track_length,'+ fyle[7]+
					'\n')	
	dataLog.write(trial_details)
	
	#this header assigns the labels for each column of the saved data
	header = 'Time,Valve,Lick,LickCount,Reward,RewardCount,Distance,TotalDistance,RewardWindow,InitialDrop,InitialDropCount,ImagingTrigger,TTLpulse,TTLtotalCount,LickRate,LapCount\n'
	print header.rstrip('\n')
	dataLog.write(header)
	
	#this tells the arduino to start sending data and also provides parameter values
	arduino_parameters = '1,'+ fyle[1]+','+ fyle[2]+','+ fyle[3]+','+ fyle[4]+','+ fyle[5]+','+ fyle[6]+','+ fyle[7]+','+ fyle[8]
	ser.write(arduino_parameters) 
	
	#updateWindow()
	
	#now receive and save data, one line per loop
	while True:
		
		currentInput = ser.readline().rstrip('\n')		#rstrip('\n') removes blank lines
		restart_cue = num(currentInput)
		if (restart_cue == 8128):
			print 'End of trial \n'
			break
		print currentInput
		dataLog.write(currentInput)
	dataLog.close()


##########################################################################


# def updateWindow():
# 	#create a new window
# 	uWindow = Tkinter.Tk()
# 	#set the window title
# 	uWindow.title("Update")
# 		
# 	click_button = Tkinter.Button(uWindow, text="Update")
# 	click_button.grid(row=11,column=1)


##########################################################################

#create a button widget
click_button = Tkinter.Button(window, text="Start", command = main)
click_button.grid(row=10,column=1)


#click_button = Tkinter.Button(window, text="Update", command = arduinoCommunication)
#click_button.grid(row=11,column=1)


# myGUI = guiMaker()
#draw the window, and start the 'application'
window.mainloop()


