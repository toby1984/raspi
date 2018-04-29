#!/bin/bash
rm bin/* -rf
( cd bin && cmake .. && make )
