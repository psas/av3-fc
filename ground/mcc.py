#!/usr/bin/env python
import gui
import psas
import wx
from matplotlib.figure import Figure
import random
import threading
import time

#==========================================================================
# Main Window Class
#==========================================================================
class MainWindow(object):
  
  def __init__(self, data):
    # Make a window
    self.window = gui.Window(None, "PSAS MCC")
    # pointer to data model that is updated in another thread
    self.model = data

    # build UI using gui framework
    self.init_UI()
    # push ui componates into window
    self.window.add_widgets()
    # run once to init
    self.update_view(None)

  def init_UI(self):
  
    # Mission box
    #--------------------------------------------------------------------------
    self.time_box       = self.window.add_box("Mission Time")
    self.time_status    = self.window.add_textinfo(self.time_box, 0, "T")

    # Network connection box
    #--------------------------------------------------------------------------
    self.network_box    = self.window.add_box("Network")
    self.packet_count   = self.window.add_textinfo(self.network_box, 0, "Packets recieved:")
    self.packet_drop    = self.window.add_textinfo(self.network_box, 1, "Packets dropped: ")
    
    # Graph
    self.fig         = Figure((8,2), dpi=72)
    self.packet_plot = self.fig.add_subplot(111)
    self.packet_plot_data = self.packet_plot.plot([0,5],[0,5])[0]
    self.graph = self.window.add_graph(self.network_box, 2, self.fig)

    # IMU box
    #---------------------------------------------------------------------------
    self.imu_box        = self.window.add_box("IMU")
    self.accel_show     = self.window.add_textinfo(self.imu_box, 0, "Accelerometer: ")

    # Timer, this updates the frame every x milliseconds
    self.window.add_timer(self.update_view, 100)
    
  def update_view(self, event):
    #wx.MutexGuiEnter()
    self.time_status.SetLabel("+1:20")
    self.packet_count.SetLabel("%d" % self.model.num_packets)
    self.packet_drop.SetLabel("%d    (%0.1f%%)" % (self.model.missed_packets, self.model.miss_percent))
    
    self.accel_show.SetLabel("[x, y, z]")
    
    self.packet_plot_data.set_xdata(range(len(self.model.packets)))
    self.packet_plot_data.set_ydata(self.model.packets)
    self.graph.draw()
    #wx.MutexGuiLeave()

#===============================================================================
# Data model
#===============================================================================
class datamodel():
  def __init__(self):
    self.num_packets            = 0
    self.missed_packets         = 0
    self.miss_percent           = 0.0
    self.packets                = []


app = gui.Application()
data = datamodel()
view = MainWindow(data)
app.main = view

fc_conn = psas.FC_Socket(data)
#api_thread = threading.Thread(target=fc_conn.fake_recieve)
api_thread = threading.Thread(target=fc_conn.listen)

api_thread.daemon = True
api_thread.start()


app.run()
