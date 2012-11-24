#!/usr/bin/env python
#
# Poll arduino for latest weather data, and insert in to RRD.
# Depends on having an RRD precreated. Refer to scripts in
# weather package.

import datetime, time, rrdtool, serial, sys

# Serial device properties
arduinodev = "/dev/weatherstation"
arduinospeed = 115200
# Path to our RRD
rrdfile = "/home/peter/weather/weather.rrd"
# Path to our log file
logfile = "/home/peter/weather/weather.log"

# Retrieve sensor data
conn = serial.Serial(arduinodev, arduinospeed)
conn.write("get;")
datain = conn.readline()
conn.close()

# Sensor data is received in the form
# "<channel number>,<temperature>,<humidity>,<time offset>;\n"
# First we split this out to a list of values,
# then cast each to numeric values and assign to variables.
x = datain.split(";")[0].split(",")
if x[0] is "1":
    temperature = float(x[1])
    humidity = int(x[2])
    timeoffset = int(x[3])
    print("Received %s;%s;%s" % (temperature, humidity, timeoffset))
else:
    print("Unknown channel received.")
    sys.exit(1)

# Figure out the time the recording was taken.
polltime = long(time.time() - timeoffset)

# Write values to RRD
rrdtool.update(rrdfile,
               "--template", "temperature:humidity",
               "%s:%s:%s" % (polltime, temperature, humidity))
# Write values to logfile
now = datetime.datetime.now()
lfh = open(logfile, 'a')
lfh.write("%s %s:%s:%s\n" % (now.strftime("%b %d %H:%M:%S"), polltime,
                           temperature, humidity))
lfh.close()
