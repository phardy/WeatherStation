#!/bin/sh
#
# Grab sunrise and sunset times and put them in a file.

# The w parameter in the URL specifies Sydney.
url="http://weather.yahooapis.com./forecastrss?w=12706662"
outputfile="/home/peter/weather/suntimes.txt"

times=`curl -s $url | grep yweather:astronomy`
echo $times | grep -q yweather:astronomy
if [ $? ]; then
	# sunrise
	echo $times | awk -F\" '{ print $2 }' > $outputfile
	echo $times | awk -F\" '{ print $4 }' >> $outputfile
fi

