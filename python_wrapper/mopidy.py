#!/usr/bin/env python2

import uilib

from mpd import MPDClient

client = None

def connect():
  client = MPDClient()               # create client object
  client.timeout = 10                # network timeout in seconds (floats allowed), default: None
  client.idletimeout = None          # timeout for fetching the result of the idle command is handled seperately, default: None
  client.connect("localhost", 6600)  # connect to localhost:6600

def disconnect():
  client.close()
  client.disconnect()
  client = None

connect()

print(client.mpd_version)          # print the MPD version
print(client.find("any", "house")) # print result of the command "find any house"

disconnect()
