// Oregon V2 decoder added - Dominique Pierre
// Oregon V3 decoder revisited - Dominique Pierre
// New code to decode OOK signals from weather sensors, etc.
// 2010-04-11 <jcw@equi4.com> http://opensource.org/licenses/mit-license.php
// $Id: ookDecoder.pde 5331 2010-04-17 10:45:17Z jcw $

class DecodeOOK {
protected:
    byte total_bits, bits, flip, state, pos, data[25];

    virtual char decode (word width) =0;
    
public:

    enum { UNKNOWN, T0, T1, T2, T3, OK, DONE };

    DecodeOOK () { resetDecoder(); }

    bool nextPulse (word width) {
        if (state != DONE)
        
            switch (decode(width)) {
                case -1: resetDecoder(); break;
                case 1:  done(); break;
            }
        return isDone();
    }
    
    bool isDone () const { return state == DONE; }

    const byte* getData (byte& count) const {
        count = pos;
        return data; 
    }
    
    void resetDecoder () {
        total_bits = bits = pos = flip = 0;
        state = UNKNOWN;
    }
    
    // add one bit to the packet data buffer
    
    virtual void gotBit (char value) {
        total_bits++;
        byte *ptr = data + pos;
        *ptr = (*ptr >> 1) | (value << 7);

        if (++bits >= 8) {
            bits = 0;
            if (++pos >= sizeof data) {
                resetDecoder();
                return;
            }
        }
        state = OK;
    }
    
    // store a bit using Manchester encoding
    void manchester (char value) {
        flip ^= value; // manchester code, long pulse flips the bit
        gotBit(flip);
    }
    
    // move bits to the front so that all the bits are aligned to the end
    void alignTail (byte max =0) {
        // align bits
        if (bits != 0) {
            data[pos] >>= 8 - bits;
            for (byte i = 0; i < pos; ++i)
                data[i] = (data[i] >> bits) | (data[i+1] << (8 - bits));
            bits = 0;
        }
        // optionally shift bytes down if there are too many of 'em
        if (max > 0 && pos > max) {
            byte n = pos - max;
            pos = max;
            for (byte i = 0; i < pos; ++i)
                data[i] = data[i+n];
        }
    }
    
    void reverseBits () {
        for (byte i = 0; i < pos; ++i) {
            byte b = data[i];
            for (byte j = 0; j < 8; ++j) {
                data[i] = (data[i] << 1) | (b & 1);
                b >>= 1;
            }
        }
    }
    
    void reverseNibbles () {
        for (byte i = 0; i < pos; ++i)
            data[i] = (data[i] << 4) | (data[i] >> 4);
    }
    
    void done () {
        while (bits)
            gotBit(0); // padding
        state = DONE;
    }
};

// 433 MHz decoders


class OregonDecoderV2 : public DecodeOOK {
public:
    OregonDecoderV2() {}
    
    // add one bit to the packet data buffer
    virtual void gotBit (char value) {
        if(!(total_bits & 0x01))
        {
            data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
        }
        total_bits++;
        pos = total_bits >> 4;
        if (pos >= sizeof data) {
            resetDecoder();
            return;
        }
        state = OK;
    }
    
    virtual char decode (word width) {
        if (200 <= width && width < 1200) {
            byte w = width >= 700;
            switch (state) {
                case UNKNOWN:
                    if (w != 0) {
                        // Long pulse
                        ++flip;
                    } else if (32 <= flip) {
                        // Short pulse, start bit
                        flip = 0;
                        state = T0;
                    } else {
                      // Reset decoder
                        return -1;
                    }
                    break;
                case OK:
                    if (w == 0) {
                        // Short pulse
                        state = T0;
                    } else {
                        // Long pulse
                        manchester(1);
                    }
                    break;
                case T0:
                    if (w == 0) {
                      // Second short pulse
                        manchester(0);
                    } else {
                        // Reset decoder
                        return -1;
                    }
                    break;
            }
        } else {
            return -1;
        }
        return total_bits == 160 ? 1: 0;
    }
};

class OregonDecoderV3 : public DecodeOOK {
public:
    OregonDecoderV3() {}
    
    // add one bit to the packet data buffer
    virtual void gotBit (char value) {
        data[pos] = (data[pos] >> 1) | (value ? 0x80 : 00);
        total_bits++;
        pos = total_bits >> 3;
        if (pos >= sizeof data) {
            resetDecoder();
            return;
        }
        state = OK;
    }
    
    virtual char decode (word width) {
        if (200 <= width && width < 1200) {
            byte w = width >= 700;
            switch (state) {
                case UNKNOWN:
                    if (w == 0)
                        ++flip;
                    else if (32 <= flip) {
                        flip = 1;
                        manchester(1);
                    } else
                        return -1;
                    break;
                case OK:
                    if (w == 0)
                        state = T0;
                    else
                        manchester(1);
                    break;
                case T0:
                    if (w == 0)
                        manchester(0);
                    else
                        return -1;
                    break;
            }
        } else {
            return -1;
        }
        return  total_bits == 80 ? 1: 0;
    }
};

OregonDecoderV2 orscV2;
OregonDecoderV3 orscV3;

#define PORT 2

volatile word pulse;

// Serial activity LED
const int serialLED = 7;
// Sensor activity LED
const int sensorLED = 6;

// Details for my sensor:
const int sensorChannel = 1;
const int sensorCode = 0xEC;

// Serial variables
const int serialLength = 32; // size of the serial buffer
char serialString[serialLength];
byte serialIndex = 0;

char lasttemp[6] = "0.0";
char lasthumid[3] = "0";
long lasttime = 0;

#if defined(__AVR_ATmega1280__)
void ext_int_1(void) {
#else
ISR(ANALOG_COMP_vect) {
#endif
    static word last;
    // determine the pulse length in microseconds, for either polarity
    pulse = micros() - last;
    last += pulse;
}

void parseData (const char* s, class DecodeOOK& decoder) {
    byte pos;
    const byte* data = decoder.getData(pos);

    // TODO: something sensible with the sensor code
    // if ((int)(data[2] >> 4) == sensorChannel && (int)data[3] == sensorCode) {
    if ((int)(data[2] >> 4) == sensorChannel) {
      // Get the temperature.
      char temp[6];
      char *tempptr = &temp[0];
      // 14th nibble indicates sign. non-zero for -ve
      if ((int)(data[6] & 0x0F) != 0) {
	*tempptr = '-';
	tempptr++;
      }
      sprintf(tempptr, "%02x", (int)(data[5]));
      tempptr = tempptr + 2;
      *tempptr = '.';
      tempptr++;
      sprintf(tempptr, "%x", (int)(data[4] >> 4));

      // Get the humidity.
      char humid[3];
      char *humidptr = &humid[0];
      sprintf(humidptr, "%x", (int)(data[7] & 0x0F));
      humidptr++;
      sprintf(humidptr, "%x", (int)(data[6] >> 4));
      humid[2] = '\0';

      strcpy(lasttemp, temp);
      strcpy(lasthumid, humid);
      lasttime = millis();
    }
    decoder.resetDecoder();
}

void readSerial() {
  while ((Serial.available() > 0) && (serialIndex < serialLength-1)) {
    digitalWrite(serialLED, HIGH);
    char serialByte = Serial.read();
    if (serialByte != ';') {
      serialString[serialIndex] = serialByte;
      serialIndex++;
    }
    if (serialByte == ';' or serialIndex == (serialLength-1)) {
      parseSerial();
      serialIndex = 0;
      memset(&serialString, 0, serialLength);
    }
  }
  digitalWrite(serialLED, LOW);
}

void parseSerial() {
  if (strcmp(serialString, "get") == 0) {
    long age = (millis() - lasttime) / 1000;
    // Format is "channel,temp,humidity,age;"
    Serial.print("1,");
    Serial.print(lasttemp);
    Serial.print(",");
    Serial.print(lasthumid);
    Serial.print(",");
    Serial.print(age);
    Serial.print(";\n");
  }
}

void setup () {
  Serial.begin(115200);
  pinMode(serialLED, OUTPUT);
  digitalWrite(serialLED, LOW);
  pinMode(sensorLED, OUTPUT);
  digitalWrite(serialLED, LOW);

  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

#if !defined(__AVR_ATmega1280__)
    pinMode(13 + PORT, INPUT);  // use the AIO pin
    digitalWrite(13 + PORT, 1); // enable pull-up

    // use analog comparator to switch at 1.1V bandgap transition
    ACSR = _BV(ACBG) | _BV(ACI) | _BV(ACIE);

    // set ADC mux to the proper port
    ADCSRA &= ~ bit(ADEN);
    ADCSRB |= bit(ACME);
    ADMUX = PORT - 1;
#else
   attachInterrupt(1, ext_int_1, CHANGE);

   DDRE  &= ~_BV(PE5);
   PORTE &= ~_BV(PE5);
#endif

   // Serial.println("\n[weatherstation initialised]");
}

void loop () {
  static int i = 0;
    cli();
    word p = pulse;
    
    pulse = 0;
    sei();

    //if (p != 0){ Serial.print(++i); Serial.print('\n');}
    
    if (p != 0) {
      digitalWrite(sensorLED, HIGH);
      if (orscV2.nextPulse(p)) {
	digitalWrite(sensorLED, HIGH);
	parseData("OSV2", orscV2);
	digitalWrite(sensorLED, LOW);
      }
      digitalWrite(sensorLED, LOW);
    }

    readSerial();
}
