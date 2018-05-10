#!/usr/bin/env python2

import uilib
import time

def callback(buttonId):
  print "Called "+str(buttonId)

uilib.init()

uilib.add_button("test",50,50,150,20,callback);

time.sleep(10);

uilib.close()
