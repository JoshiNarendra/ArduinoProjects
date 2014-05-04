import Tkinter
import threading

class experimentGUI(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
		self.start()
	def callback(self):
	   self.root.quit() 
	
	def showMeTheMoney(self):
		print self.mouse.get()
		print self.trial.get()
		print self.pre_odor.get()
		print self.odor.get()
		print self.trace.get()
		print self.reward_window.get()
		print self.post_reward.get()
		print self.reward_size.get()
		print self.licks_per_reward.get()
	def run(self):
		self.root=Tkinter.Tk()
		self.root.protocol("WM_DELETE_WINDOW", self.callback)
		
		##################Labels and their respective values##############
		mouse_label = Tkinter.Label(self.root, text="Mouse").grid(row=0) #create a label widget
		self.mouse = Tkinter.Entry(self.root) #create a text entry widget
		self.mouse.insert(0,"Black6") #insert default value for the mouse entry

		trial_label = Tkinter.Label(self.root, text="Trial").grid(row=1)
		self.trial = Tkinter.Entry(self.root)
		self.trial.insert(0,"1")

		pre_odor_label = Tkinter.Label(self.root, text="Pre-odor duration(s):").grid(row=2)
		self.pre_odor = Tkinter.Entry(self.root)
		self.pre_odor.insert(0,"30")

		odor_label = Tkinter.Label(self.root, text="Odor duration(s):").grid(row=3)
		self.odor = Tkinter.Entry(self.root, bg = 'light green')
		self.odor.insert(0,"5")

		trace_label = Tkinter.Label(self.root, text="Trace period(s):").grid(row=4)
		self.trace = Tkinter.Entry(self.root)
		self.trace.insert(0,"20")

		reward_window_label = Tkinter.Label(self.root, text="Reward window(s):").grid(row=5)
		self.reward_window = Tkinter.Entry(self.root)
		self.reward_window.insert(0,"15")

		post_reward_label = Tkinter.Label(self.root, text="Post-reward duration(s):").grid(row=6)
		self.post_reward = Tkinter.Entry(self.root)
		self.post_reward.insert(0,"0")

		reward_size_label = Tkinter.Label(self.root, text="Reward size(ms):").grid(row=7)
		self.reward_size = Tkinter.Entry(self.root)
		self.reward_size.insert(0,"30")

		licks_per_reward_label = Tkinter.Label(self.root, text="Licks per reward:").grid(row=8)
		self.licks_per_reward = Tkinter.Entry(self.root)
		self.licks_per_reward.insert(0,"5")


		#arrange the entry fields in a grid layout
		self.mouse.grid(row=0, column=1)
		self.trial.grid(row=1, column=1)
		self.pre_odor.grid(row=2, column=1)
		self.odor.grid(row=3, column=1)
		self.trace.grid(row=4, column=1)
		self.reward_window.grid(row=5, column=1)
		self.post_reward.grid(row=6, column=1)
		self.reward_size.grid(row=7, column=1)
		self.licks_per_reward.grid(row=8, column=1)
		
		#create a button widget
		click_button = Tkinter.Button(self.root, text="Start", command = self.showMeTheMoney)
		click_button.grid(row=9,column=1)
		
		
		self.root.mainloop()
app = experimentGUI()
print 'now can continue running code while mainloop runs'