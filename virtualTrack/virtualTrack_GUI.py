import Tkinter
import serial
import time
import datetime
import os, errno
ser = serial.Serial(5,115200) #ArduinoMega is on COM6 port

#If above doesn't work, try these:
#ser = serial.Serial('/dev/ttyS1',115200) #this one works with Arduino Due
#ser = serial.Serial('/dev/ttyACM1',115200)
#ser = serial.Serial('/dev/ttyUSB1',115200)
#ser = serial.Serial('com1',115200)
#ser = serial.Serial('COM1',115200)

#create a new window
window = Tkinter.Tk()
#set the window title
window.title("Mouse Virtual Reality Experiment")


mouse_label = Tkinter.Label(window, text="Mouse ID:").grid(row=0) #create a label widget
mouse = Tkinter.Entry(window) #create a text entry widget
mouse.insert(0,"x") #insert default value for the mouse entry

reward_size_label = Tkinter.Label(window, text="Reward size(ms):").grid(row=1)
reward_size = Tkinter.Entry(window)
reward_size.insert(0,"30")

licks_per_reward_label = Tkinter.Label(window, text="Licks per reward:").grid(row=2)
licks_per_reward = Tkinter.Entry(window)
licks_per_reward.insert(0,"2")

reward_window_label = Tkinter.Label(window, text="Reward window(s):").grid(row=3)
reward_window = Tkinter.Entry(window, bg = 'light green')
reward_window.insert(0,"2")

track_label = Tkinter.Label(window, text="Length of virtual track(mm):").grid(row=4)
track = Tkinter.Entry(window, bg = 'light blue')
track.insert(0,"4500")

blank_label = Tkinter.Label(window).grid(row=5)

envA_duration_label = Tkinter.Label(window, text="Environment A duration(s):").grid(row=6)
envA_duration = Tkinter.Entry(window, fg = 'blue')
envA_duration.insert(0,"300")

envA_initial_drop_label = Tkinter.Label(window, text="Initial drop size(ms):").grid(row=7)
envA_initial_drop = Tkinter.Entry(window)
envA_initial_drop.insert(0,"30")

envA_max_lap_count_label = Tkinter.Label(window, text="Max number of laps:").grid(row=8)
envA_max_lap_count = Tkinter.Entry(window)
envA_max_lap_count.insert(0,"100")

envA_odor1_label = Tkinter.Label(window, text="envA_odor1:").grid(row=9)
envA_odor1 = Tkinter.Entry(window, fg = 'red')
envA_odor1.insert(0,"3")

envA_odor2_label = Tkinter.Label(window, text="envA_odor2:").grid(row=10)
envA_odor2 = Tkinter.Entry(window, fg = 'red')
envA_odor2.insert(0,"1")

envA_odor3_label = Tkinter.Label(window, text="envA_odor3:").grid(row=11)
envA_odor3 = Tkinter.Entry(window, fg = 'red')
envA_odor3.insert(0,"2")

envA_odor4_label = Tkinter.Label(window, text="envA_odor4:").grid(row=12)
envA_odor4 = Tkinter.Entry(window, fg = 'red')
envA_odor4.insert(0,"1")

blank_label = Tkinter.Label(window).grid(row=13)

envB_duration_label = Tkinter.Label(window, text="Environment B duration(s):").grid(row=14)
envB_duration = Tkinter.Entry(window, fg = 'blue')
envB_duration.insert(0,"0")

envB_initial_drop_label = Tkinter.Label(window, text="Initial drop size(ms):").grid(row=15)
envB_initial_drop = Tkinter.Entry(window)
envB_initial_drop.insert(0,"0")

envB_max_lap_count_label = Tkinter.Label(window, text="Max number of laps:").grid(row=16)
envB_max_lap_count = Tkinter.Entry(window)
envB_max_lap_count.insert(0,"0")

envB_odor1_label = Tkinter.Label(window, text="envB_odor1:").grid(row=17)
envB_odor1 = Tkinter.Entry(window, fg = 'red')
envB_odor1.insert(0,"3")

envB_odor2_label = Tkinter.Label(window, text="envB_odor2:").grid(row=18)
envB_odor2 = Tkinter.Entry(window, fg = 'red')
envB_odor2.insert(0,"1")

envB_odor3_label = Tkinter.Label(window, text="envB_odor3:").grid(row=19)
envB_odor3 = Tkinter.Entry(window, fg = 'red')
envB_odor3.insert(0,"2")

envB_odor4_label = Tkinter.Label(window, text="envB_odor4:").grid(row=20)
envB_odor4 = Tkinter.Entry(window, fg = 'red')
envB_odor4.insert(0,"1")

blank_label = Tkinter.Label(window).grid(row=21)

#arrange the entry fields in a grid layout
mouse.grid(row=0, column=1)
reward_size.grid(row=1, column=1)
licks_per_reward.grid(row=2, column=1)
reward_window.grid(row=3, column=1)
track.grid(row=4, column=1)

envA_duration.grid(row=6, column=1)
envA_initial_drop.grid(row=7, column=1)
envA_max_lap_count.grid(row=8, column=1)
envA_odor1.grid(row=9, column=1)
envA_odor2.grid(row=10, column=1)
envA_odor3.grid(row=11, column=1)
envA_odor4.grid(row=12, column=1)

envB_duration.grid(row=14, column=1)
envB_initial_drop.grid(row=15, column=1)
envB_max_lap_count.grid(row=16, column=1)
envB_odor1.grid(row=17, column=1)
envB_odor2.grid(row=18, column=1)
envB_odor3.grid(row=19, column=1)
envB_odor4.grid(row=20, column=1)

##########################################################################
def arduinoParameters():
	parameters = [mouse.get(), 					
				  reward_size.get(),
				  licks_per_reward.get(),
				  reward_window.get(),
				  track.get(),
				  
				  envA_duration.get(),
				  envA_initial_drop.get(),
				  envA_max_lap_count.get(),
				  envA_odor1.get(),
				  envA_odor2.get(),
				  envA_odor3.get(),
				  envA_odor4.get(),
				  
				  envB_duration.get(),
				  envB_initial_drop.get(),
				  envB_max_lap_count.get(),
				  envB_odor1.get(),
				  envB_odor2.get(),
				  envB_odor3.get(),
				  envB_odor4.get()]
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

def main():
	
	fyle = arduinoParameters()
	filename = timeStamp(fyle[0])
	filename = filename + ".csv"
	print 'Name of this file is: ' + filename
	
	#create a .csv file inside a folder for each mouse to save incoming data from the arduino
	dir_path = 'C:/Users/axel/Desktop/Recordings/' + fyle[0] + '/' + fyle[0] + '_' + dir_timeStamp()
	filename = dir_path + '/' + filename
	if dir_path:
		if not os.path.isdir(dir_path):
			os.makedirs(dir_path)
		dataLog = open(filename, "a+")
	
	#record the detailed conditions of each trial
	trial_details = ('Mouse_name,'+ fyle[0] +
					',Reward_size,'+ fyle[1] + 
					',Licks_per_reward,'+ fyle[2]+
					',Reward_window,'+ fyle[3]+
					',Track_length,'+ fyle[4]+
					',EnvA_duration,'+ fyle[5]+
					',envA_initial_drop,'+ fyle[6]+
					',envA_max_number_of_laps,'+ fyle[7]+
					',envA_odor1,'+ fyle[8]+
					',envA_odor2,'+ fyle[9]+
					',envA_odor3,'+ fyle[10]+
					',envA_odor4,'+ fyle[11]+
					',EnvB_duration,'+ fyle[12]+
					',envB_initial_drop,'+ fyle[13]+
					',envB_max_number_of_laps,'+ fyle[14]+
					',envB_odor1,'+ fyle[15]+
					',envB_odor2,'+ fyle[16]+
					',envB_odor3,'+ fyle[17]+
					',envB_odor4,'+ fyle[18]+
					'\n')	
	dataLog.write(trial_details)
	
	#this header assigns the labels for each column of the saved data
	header = 'Time,Valve,Lick,LickCount,Reward,RewardCount,Distance,TotalDistance,RewardWindow,InitialDrop,InitialDropCount,ImagingTrigger,TTLpulse,TTLtotalCount,LickRate,LapCount,Environment\n'
	print header.rstrip('\n')
	dataLog.write(header)
	
	#this tells the arduino to start sending data and also provides parameter values
	arduino_parameters = ('1,'+ 
						 fyle[1]+','+ 
						 fyle[2]+','+ 
						 fyle[3]+','+ 
						 fyle[4]+','+ 
						 fyle[5]+','+ 
						 fyle[6]+','+ 
						 fyle[7]+','+ 
						 fyle[8]+','+ 
						 fyle[9]+','+ 
						 fyle[10]+','+ 
						 fyle[11]+','+ 
						 fyle[12]+','+ 
						 fyle[13]+','+ 
						 fyle[14]+','+ 
						 fyle[15]+','+ 
						 fyle[16]+','+ 
						 fyle[17]+','+ 
						 fyle[18])
	ser.write(arduino_parameters) 
	
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

#create a button widget
click_button = Tkinter.Button(window, text="Start", command = main)
click_button.grid(row=22,column=1)

# myGUI = guiMaker()
#draw the window, and start the 'application'
window.mainloop()