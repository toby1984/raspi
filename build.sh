#!/bin/bash
cd /home/tgierke/tmp/raspi
mkdir bin
rm bin/* -rf
( cd bin && cmake .. && make )
