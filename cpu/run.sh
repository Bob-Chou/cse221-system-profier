#!/bin/bash

make clean && make && schedtool -a 0 -e nice -n-20 ./cpu
