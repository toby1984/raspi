#!/usr/bin/env python2

import uilib
import time

def buttonClicked(buttonId):
  print "########### inside python "+str(buttonId)

uilib.init()

# buttonId = uilib.add_image_button("/home/tobi/qtcreator_workspace/raspi/test.png",50,50,150,20,buttonClicked)
buttonId = uilib.add_button("Hallo Billa!",50,50,150,20,buttonClicked)
print "Added button "+str(buttonId)

time.sleep(60)

# test = 5.0
# for x in range(0,10000000):
#  test = test*2

uilib.close()
