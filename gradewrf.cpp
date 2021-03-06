#include "gradewrf.h"

volatile unsigned long GradewRF::lastTime;
volatile unsigned long GradewRF::currentTime;
volatile unsigned long GradewRF::diffTime;
volatile int GradewRF::syncState;
volatile int GradewRF::previousDiff;
volatile int GradewRF::tByte, GradewRF::bits, GradewRF::nBytes;
volatile int GradewRF::stopReceiving;

volatile byte GradewRF::userData[4];
volatile int GradewRF::userDataSize;
volatile bool GradewRF::userDataAvailable;
volatile int GradewRF::rfInt;
volatile int GradewRF::rxChecksum;


// transmission
int GradewRF::signalData[MAX_SIGNAL_DATA];
volatile byte GradewRF::txData[4];
byte GradewRF::lastVal;
unsigned long GradewRF::totalDuration;
int GradewRF::signalIdx;
int GradewRF::userIdx;
int GradewRF::rfPin;
byte GradewRF::txRepetitions;

bool GradewRF::checkOrder(unsigned long value, unsigned long ref, unsigned long tol)
{
  unsigned long _min=ref-tol;
  unsigned long _max=ref+tol;
  if(value>=_min && value<=_max){
    return true;
  }
  return false;
}

void GradewRF::addBit(char v)
{
  GradewRF::tByte<<=1;
  GradewRF::tByte+=v;
  GradewRF::bits++;
  if(GradewRF::bits==8){
    // Where we at ?
    if(GradewRF::nBytes==0){
      if(GradewRF::tByte==BYTE_SYNC){
        //Serial.println("Sync header OK");
      }else{
        GradewRF::stopReceiving=1;
      }
    }else if(GradewRF::nBytes==1){
      //Serial.print(tByte);
      //Serial.println(" bytes!");
      //printf("Length: %d bytes\n", tByte);
    }else if(GradewRF::nBytes==2){
      GradewRF::rxChecksum=tByte;
    }else{
      // Actual data
      GradewRF::userData[GradewRF::userDataSize++]=GradewRF::tByte;
      if(GradewRF::nBytes==6){
        GradewRF::stopReceiving=1;
      }
    }
    GradewRF::bits=0;
    GradewRF::tByte=0;
    GradewRF::nBytes++;
    // Stop receiving
    if(GradewRF::stopReceiving){
      GradewRF::syncState=0;
      GradewRF::nBytes=0;
      GradewRF::stopReceiving=0;
      if(GradewRF::verifyChecksum()){
        GradewRF::userDataAvailable=1;
		//printf("Checksum: OK\n");
      }else{
		//printf("Checksum: ERROR\n");
	}
    }
  }
}

void ISR_ATTR GradewRF::handleInterrupt()
{
  GradewRF::currentTime=micros();
  GradewRF::diffTime=GradewRF::currentTime-GradewRF::lastTime;
  if(GradewRF::diffTime<PULSE_IGNORE){
    return;
  }
  GradewRF::lastTime=GradewRF::currentTime;
  //Serial.println(diffTime);
  if(GradewRF::syncState==0){
    if(checkOrder(GradewRF::diffTime, PULSE_SYNC1, 250)){
      //Serial.println("SYNC1");
      GradewRF::syncState=1;
      return;
    }
  }

  if(GradewRF::syncState==1){
    if(checkOrder(GradewRF::diffTime, PULSE_SYNC2, 250)){
      //Serial.println("SYNC2");
      GradewRF::syncState=2;
      GradewRF::previousDiff=0;
      return;
    }else{
      GradewRF::syncState=0;
      return;
    }
  }

  if(GradewRF::syncState==2){
    if(checkOrder(GradewRF::diffTime, PULSE_SYNC3, 250)){
      //Serial.println("SYNC3");
      GradewRF::syncState=3;
      GradewRF::previousDiff=0;
      return;
    }else{
      GradewRF::syncState=0;
      return;
    }
  }

  if(GradewRF::syncState==3){
    if(checkOrder(GradewRF::diffTime, PULSE_SYNC4, 250)){
      //Serial.println("Synchronized! Starting receiving");
      GradewRF::syncState=4;
      GradewRF::previousDiff=0;
      GradewRF::tByte=0;
      GradewRF::bits=0;
      GradewRF::nBytes=0;
      GradewRF::userDataSize=0;
      GradewRF::userDataAvailable=0;
      return;
    }else{
      GradewRF::syncState=0;
      return;
    }
  }

  if(GradewRF::syncState==4){
    // From here on, we are receiving data!
    if(GradewRF::previousDiff>0){
      // We have a bit!
      if(GradewRF::previousDiff<GradewRF::diffTime){
        addBit(0);
      }else{
        addBit(1);
      }
      // Tell to wait for the next signal
      GradewRF::previousDiff=0;
    }else{
      // Let's wait for the next signal
      GradewRF::previousDiff=GradewRF::diffTime;
    }
  }
}

bool GradewRF::isDataAvailable()
{
  return this->userDataAvailable;
}

void GradewRF::resetData()
{
  this->userDataAvailable=0;
}

unsigned long GradewRF::getData32()
{
  unsigned long v=0;
  int i;
  for(i=0;i<4;i++){
    v<<=8;
    v+=this->userData[i];
  }
  return v;
}

void GradewRF::setReceive(int _rfInt)
{
  this->rfInt=_rfInt;
  this->syncState=0;
  this->stopReceiving=0;
  this->userDataAvailable=0;
  #ifdef RaspberryPi
  if(wiringPiSetupGpio()<0){ // Using Broadcom chip pin numbers
    fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
  }
  pinMode(this->rfInt, INPUT);
  if(wiringPiISR(this->rfInt, INT_EDGE_BOTH, &handleInterrupt)<0){
    fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
  }
  #else
  pinMode(this->rfInt, INPUT);
  attachInterrupt(this->rfInt, handleInterrupt, CHANGE);
  #endif
}



// TRANSMISSION
void GradewRF::generatePulse(byte p)
{
		if(p==0){
				signalData[signalIdx++]=PULSE_SHORT;
				signalData[signalIdx++]=PULSE_LONG;
		}else{
				signalData[signalIdx++]=PULSE_LONG;
				signalData[signalIdx++]=PULSE_SHORT;
		}
		if(signalIdx>=MAX_SIGNAL_DATA){
			signalIdx=0; // circular buffer; ugly, but prevents overflows
		}
		totalDuration+=PULSE_LONG+PULSE_SHORT;
}


void GradewRF::encodeByte(byte b)
{
		int i;
		byte mask=0x80;
		for(i=7;i>=0;i--){
				byte v=(b&mask)>>i;
				generatePulse(v);
				mask>>=1;
		}
}

void GradewRF::startSignal()
{
		lastVal=0;
		totalDuration=0;
		signalIdx=0;

		signalData[signalIdx++]=0;
		signalData[signalIdx++]=PULSE_SYNC1;
		signalData[signalIdx++]=PULSE_SYNC2;
		signalData[signalIdx++]=PULSE_SYNC3;
		signalData[signalIdx++]=PULSE_SYNC4;
		encodeByte(BYTE_SYNC); // 01001011
}

void GradewRF::encodeString(char *str)
{
		int i;
		for(i=0;i<strlen(str);i++){
				encodeByte(str[i]);
		}
}

void GradewRF::endSignal()
{
		signalData[signalIdx++]=PULSE_SHORT;
		signalData[signalIdx++]=PULSE_SHORT;
}


void GradewRF::setTransmit(int _rfPin)
{
		this->rfPin=_rfPin;
		#if defined(ARDUINO) && ARDUINO >= 100
		#else
				wiringPiSetupGpio(); // Using Broadcom chip pin numbers
		#endif
		this->txRepetitions=3;
		pinMode(this->rfPin, OUTPUT);
		digitalWrite(this->rfPin, LOW);
		delay(1);
}

byte GradewRF::calculateChecksum()
{
	return txData[0]^txData[1]^txData[2]^txData[3];
}

bool GradewRF::verifyChecksum()
{
	byte checksum=userData[0]^userData[1]^userData[2]^userData[3];
	if(checksum==GradewRF::rxChecksum)
		return true;
	return false;
}

void GradewRF::transmitData(unsigned long val)
{
		//
		txData[0]=val>>24;
		txData[1]=(val>>16)&0xff;
		txData[2]=(val>>8)&0xff;
		txData[3]=val&0xff;
		//
		byte txChecksum=calculateChecksum();
		startSignal();
		encodeByte(4); // length
		encodeByte(txChecksum); // checksum
		encodeByte(txData[0]);
		encodeByte(txData[1]);
		encodeByte(txData[2]);
		encodeByte(txData[3]);
		endSignal();

		for(int r=0;r<txRepetitions;r++){
			totalDuration=0;
			lastVal=0;
			if(signalData[0]>0)
				digitalWrite(this->rfPin, LOW);
			for(int i=0;i<signalIdx;i++){
				lastVal=!lastVal;
				delayMicroseconds(signalData[i]);
				digitalWrite(this->rfPin, lastVal?HIGH:LOW);
				//
				totalDuration+=signalData[i];
			}
			if(lastVal)digitalWrite(this->rfPin, LOW);
			delayMicroseconds(1000);
		}

}
