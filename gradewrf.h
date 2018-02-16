#ifndef include_gradewrf
#define include_gradewrf

#if defined(ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#else
	#define RaspberryPi
	#include <wiringPi.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <string.h>
	#include <errno.h>
	typedef unsigned char byte;
#endif

#define PULSE_IGNORE 154
#define PULSE_SYNC1 400
#define PULSE_SYNC2 800
#define PULSE_SYNC3 1200
#define PULSE_SYNC4 400
#define PULSE_SHORT 232
#define PULSE_LONG 1160
#define BYTE_SYNC 75
#define MAX_SIGNAL_DATA 128

#if defined(ESP8266) || defined(ESP32)
    #define ISR_ATTR ICACHE_RAM_ATTR
#else
    #define ISR_ATTR
#endif

class GradewRF
{
private:
  volatile static unsigned long lastTime;
  volatile static unsigned long currentTime;
  volatile static unsigned long diffTime;
  volatile static int syncState;
  volatile static int previousDiff;
  volatile static int tByte, bits, nBytes;
  volatile static int stopReceiving;
  
  volatile static byte userData[4];
  volatile static int userDataSize;
  volatile static bool userDataAvailable;
  volatile static int rfInt;
  
// Transmission
  static int signalData[MAX_SIGNAL_DATA];
  static byte lastVal;
  static unsigned long totalDuration;
  static int signalIdx;
  static int userIdx;
  static int rfPin;
  static byte txRepetitions;


public:
  static bool checkOrder(unsigned long value, unsigned long ref, unsigned long tol);
  static void addBit(char v);
  static void handleInterrupt();
  bool isDataAvailable();
  void resetData();
  unsigned long getData32();
  void setReceive(int _rfInt);
  
  void generatePulse(byte p);
  void encodeByte(byte b);
  void startSignal();
  void encodeString(char *str);
  void endSignal();
  void setTransmit(int _rfPin);
  void transmitData(unsigned long val);
};

#endif
