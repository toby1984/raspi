#!/bin/bash
cd /home/tgierke/tmp/raspi
rm bin/* -rf
( cd bin && cmake .. && make )
