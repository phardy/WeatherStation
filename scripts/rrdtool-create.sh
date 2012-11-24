#!/bin/sh

# Capture data every two minutes, and store real data for a week.
# Store 10-minute samples for one month.
# Store 30-minute samples for one year.
# Store daily min and max for five years.
rrdtool create weather.rrd --start 1353242370 --step 120 \
	DS:temperature:GAUGE:240:-10:50 \
	DS:humidity:GAUGE:240:0:100 \
	RRA:AVERAGE:0.5:1:5040 \
	RRA:AVERAGE:0.5:5:4464 \
	RRA:AVERAGE:0.5:15:17520 \
	RRA:MIN:0.2:720:1810 \
	RRA:MAX:0.2:720:1810
