#!/bin/bash
rm -rf build 2>&1 >/dev/null
python setup.py build
