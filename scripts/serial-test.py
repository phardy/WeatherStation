#!/usr/bin/python
#
# Let's retrieve some weather data!
import time
import serial

print("Opening serial port")
conn = serial.Serial('/dev/weatherstation', 115200)

print("sleeping")
time.sleep(1)
print("writing to port")
conn.write('get;')
print("reading from port")
blah = conn.readline()

print blah
