#!/bin/sh
#

# Start at midnight last night
starttime=`date -d 00:00:00 +%s`
sunrise=`head -1 /home/peter/weather/suntimes.txt`
sunset=`tail -1 /home/peter/weather/suntimes.txt`
sunrises=`date -d "$sunrise" +%s`
sunsets=`date -d "$sunset" +%s`
# rrdtool demands we escape the :
sunriset=`echo $sunrise | sed 's/:/\\\:/'`
sunsett=`echo $sunset | sed 's/:/\\\:/'`
curtime=`date | sed 's/:/\\\:/g'`

rrdtool graph ~/home.hardy.dropbear.id.au/htdocs/junk/weather-tmp.png \
	--start $starttime --end start+1d \
	--width 800 --height 500 --full-size-mode \
	--slope-mode \
	--lower-limit 0 \
	--y-grid 1:5 \
	--vertical-label "Temperature (°C)"  \
	--right-axis -4:0 \
	--right-axis-label "Humidity (%)" \
	--font DEFAULT:8:Arial --font TITLE:13 \
	DEF:todayhumid=/home/peter/weather/weather.rrd:humidity:AVERAGE \
	DEF:yesterdayhumid=/home/peter/weather/weather.rrd:humidity:AVERAGE:start=$starttime-1d:end=start+1d \
	SHIFT:yesterdayhumid:86400 \
	CDEF:todayhumidhalf=todayhumid,-0.25,\* \
	CDEF:yesterdayhumidhalf=yesterdayhumid,-0.25,\* \
	DEF:todaytemp=/home/peter/weather/weather.rrd:temperature:AVERAGE \
	DEF:yesterdaytemp=/home/peter/weather/weather.rrd:temperature:AVERAGE:start=$starttime-1d:end=start+1d \
	SHIFT:yesterdaytemp:86400 \
	VDEF:curhumid=todayhumid,LAST \
	VDEF:maxhumid=todayhumid,MAXIMUM \
	VDEF:minhumid=todayhumid,MINIMUM \
	VDEF:yesterdaymaxhumid=yesterdayhumid,MAXIMUM \
	VDEF:yesterdayminhumid=yesterdayhumid,MINIMUM \
	VDEF:curtemp=todaytemp,LAST \
	VDEF:maxtemp=todaytemp,MAXIMUM \
	VDEF:mintemp=todaytemp,MINIMUM \
	VDEF:yesterdaymaxtemp=yesterdaytemp,MAXIMUM \
	VDEF:yesterdaymintemp=yesterdaytemp,MINIMUM \
	AREA:todaytemp#FF3200:Temperature \
	CDEF:tempshade05=todaytemp,0.05,\* \
	AREA:tempshade05#FFFA00 \
	AREA:tempshade05#FFF000::STACK \
	AREA:tempshade05#FFE600::STACK \
	AREA:tempshade05#FFDC00::STACK \
	AREA:tempshade05#FFD200::STACK \
	AREA:tempshade05#FFC800::STACK \
	AREA:tempshade05#FFBE00::STACK \
	AREA:tempshade05#FFB400::STACK \
	AREA:tempshade05#FFA000::STACK \
	AREA:tempshade05#FF9600::STACK \
	AREA:tempshade05#FF8C00::STACK \
	AREA:tempshade05#FF8200::STACK \
	AREA:tempshade05#FF7800::STACK \
	AREA:tempshade05#FF6E00::STACK \
	AREA:tempshade05#FF6400::STACK \
	AREA:tempshade05#FF5A00::STACK \
	AREA:tempshade05#FF5000::STACK \
	AREA:tempshade05#FF4600::STACK \
	AREA:tempshade05#FF3C00::STACK \
	GPRINT:curtemp:"%.1lf°C" \
	COMMENT:"\t" LINE2:yesterdaytemp#A41111:"Temperature yesterday":dashes=6,4 \
	COMMENT:"\t" \
	AREA:todayhumidhalf#4169E1:Humidity \
	CDEF:humidshade05=todayhumidhalf,0.05,\* \
	AREA:humidshade05#EBEFFC \
	AREA:humidshade05#D8E0F9::STACK \
	AREA:humidshade05#C6D2F7::STACK \
	AREA:humidshade05#B6C5F4::STACK \
	AREA:humidshade05#A7B9F2::STACK \
	AREA:humidshade05#99AFF0::STACK \
	AREA:humidshade05#8CA4EE::STACK \
	AREA:humidshade05#809BEC::STACK \
	AREA:humidshade05#7693EA::STACK \
	AREA:humidshade05#6D8BE8::STACK \
	AREA:humidshade05#6485E7::STACK \
	AREA:humidshade05#5D7FE6::STACK \
	AREA:humidshade05#567AE5::STACK \
	AREA:humidshade05#5075E4::STACK \
	AREA:humidshade05#4C71E3::STACK \
	AREA:humidshade05#486EE2::STACK \
	AREA:humidshade05#456CE2::STACK \
	AREA:humidshade05#436AE1::STACK \
	AREA:humidshade05#4169E1::STACK \
	GPRINT:curhumid:"%.0lf%%" \
	COMMENT:"\t" LINE2:yesterdayhumidhalf#3D3FB0:"Humidity yesterday":dashes=6,4 \
	COMMENT:"\n" \
	GPRINT:maxtemp:"Maximum\: %.1lf°C" \
	COMMENT:"\t\t" GPRINT:yesterdaymaxtemp:"Maximum\: %.1lf°C" \
	COMMENT:"\t\t" GPRINT:maxhumid:"Maximum\: %.0lf%%" \
	COMMENT:"\t\t" GPRINT:yesterdaymaxhumid:"Maximum\: %.0lf%%" \
	COMMENT:"\n" \
	GPRINT:mintemp:"Minimum\: %.1lf°C" \
	COMMENT:"\t\t" GPRINT:yesterdaymintemp:"Minimum\: %.1lf°C" \
	COMMENT:"\t\t" GPRINT:minhumid:"Minimum\: %.0lf%%" \
	COMMENT:"\t\t" GPRINT:yesterdayminhumid:"Minimum\: %.0lf%%" \
	COMMENT:"\n" \
	COMMENT:"\t" \
	VRULE:$sunrises#5BD9F4:"Sunrise\: $sunriset" \
	VRULE:$sunsets#000000:"Sunset\: $sunsett" \
	COMMENT:"\t\t\t\t\t" \
	COMMENT:"Last updated\: $curtime" \
	COMMENT:"\n"
