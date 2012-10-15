#!/usr/bin/env python
import gui
import wx
from matplotlib.figure import Figure
import random
import threading
import time


class MainWindow(object):
  
  def __init__(self, data):
    self.window = gui.Window(None, "PSAS MCC")
    self.data = data

    self.init_UI()
    self.update_view(None)
    
    self.window.add_widgets()

  def init_UI(self):
  
    # Device
    self.time_box       = self.window.add_box("Mission Time")
    self.time_status    = self.window.add_textinfo(self.time_box, 0, "T")

    # Network
    #--------------------------------------------------------------------------
    self.network_box    = self.window.add_box("Network")
    self.packet_count   = self.window.add_textinfo(self.network_box, 0, "Packets recieved:")
    self.packet_drop    = self.window.add_textinfo(self.network_box, 1, "Packets dropped: ")
    
    # Graph
    #---------------------------------------------------------------------------
    self.fig         = Figure((8,2), dpi=72)
    self.packet_plot = self.fig.add_subplot(111)
    self.packet_plot_data = self.packet_plot.plot([0,1,2,3,4],[4,3,5,5,3])[0]

    self.graph = self.window.add_graph(self.network_box, 2, self.fig)

    # IMU
    #---------------------------------------------------------------------------
    self.imu_box        = self.window.add_box("IMU")
    self.accel_show     = self.window.add_textinfo(self.imu_box, 0, "Accelerometer: ")

    # Timer
    self.window.add_timer(self.update_view, 100)
    
  def update_view(self, event):
    #wx.MutexGuiEnter()
    #print self.data.d
    self.time_status.SetLabel("+1:20")
    self.packet_count.SetLabel("%d" % self.data.d)
    self.packet_drop.SetLabel("%d   (1.3%%)" % int(random.uniform(100,500)))
    
    self.accel_show.SetLabel("[x, y, z]")
    
    self.packet_plot_data.set_ydata([4,5,random.uniform(0,10),3,2])
    self.graph.draw()
    #wx.MutexGuiLeave()

class datamodel():
  def __init__(self):
    self.d = 0

  def fake_api(self):
    for i in range(10):
      #print i
      self.d  = i
      time.sleep(0.1)

app = gui.Application()
data = datamodel()
view = MainWindow(data)
app.main = view

api_thread = threading.Thread(target=data.fake_api)
api_thread.daemon = True
api_thread.start()


app.run()
