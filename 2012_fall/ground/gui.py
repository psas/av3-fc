import wx
import matplotlib
matplotlib.interactive(False)
matplotlib.use('WXAgg')
from matplotlib.backends.backend_wxagg import FigureCanvasWxAgg as FigCanvas
#from matplotlib.figure import Figure


#===============================================================================
# Window
#===============================================================================
class Window(wx.Frame):

  def __init__(self, parent, title):
    wx.Frame.__init__(self, parent, title=title
                      , size = (1024, 650)
                      , style = wx.DEFAULT_FRAME_STYLE|wx.NO_FULL_REPAINT_ON_RESIZE)

    self.container  = wx.Panel(self)
    self.control    = self.container
    self.vbox       = wx.BoxSizer(wx.VERTICAL)
    self.sections   = []


    
  def add_box(self, label):
    widget    = wx.Panel(self.container)
    wrapper   = wx.BoxSizer(wx.VERTICAL)
    
    box       = wx.StaticBox(widget, label=label)
    boxsizer  = wx.StaticBoxSizer(box, wx.VERTICAL)
    sizer     = wx.GridBagSizer(4, 4)

    boxsizer.Add(sizer, flag=wx.LEFT|wx.TOP|wx.EXPAND)
    
    sizer.AddGrowableCol(2)
    #widget.SetSizerAndFit(sizer)
    
    wrapper.Add(boxsizer,flag=wx.ALL|wx.EXPAND)
    widget.SetSizer(wrapper)
    self.sections.append(widget)
    return (widget, sizer)

  def add_textbox(self, box, row, label, units):
    labeltext = wx.StaticText(box[0], label=label, style=wx.ALIGN_CENTRE)
    info      = wx.TextCtrl(box[0])
    info.Disable()
    units     = wx.StaticText(box[0], label=units, style=wx.ALIGN_CENTRE)
    
    box[1].Add(labeltext, pos=(row, 0), flag=wx.TOP|wx.BOTTOM, border=5)
    box[1].Add(info,      pos=(row, 1))
    box[1].Add(units,     pos=(row, 2), flag=wx.TOP|wx.BOTTOM, border=5)
    
    return info

  def add_textinfo(self, box, row, label):
    labeltext = wx.StaticText(box[0], label=label, style=wx.ALIGN_CENTRE)
    info      = wx.StaticText(box[0])
    
    box[1].Add(labeltext, pos=(row, 0), flag=wx.TOP|wx.BOTTOM, border=5)
    box[1].Add(info,      pos=(row, 1), flag=wx.TOP|wx.BOTTOM, border=5)
    
    return info

  def add_graph(self, box, row, graph):
    self.canvas = FigCanvas(box[0], -1, graph)
    box[1].Add(self.canvas, pos=(row,1))
    return self.canvas

  def add_timer(self, callback, timeout):
    self.redraw_timer = wx.Timer(self)
    self.Bind(wx.EVT_TIMER, callback, self.redraw_timer)        
    self.redraw_timer.Start(timeout)

  def add_widgets(self):
    for section in self.sections:
      self.vbox.Add(section, flag=wx.ALL|wx.EXPAND, border=10)
    
    self.container.SetSizer(self.vbox)    

    self.Centre()

    #self.redraw_timer = wx.Timer(self)
    #self.Bind(wx.EVT_TIMER, self.on_redraw_timer, self.redraw_timer)        
    #self.redraw_timer.Start(100)

    self.Show(True)

#===============================================================================
# Application
#===============================================================================
class Application():

  def __init__(self):
    self.app  = wx.App()
    
  def run(self):
    self.app.SetTopWindow(self.main.window)
    self.app.MainLoop()
   
