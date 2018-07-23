#include <gradewrf.h>

GradewRF rf=GradewRF();

void setup() {
  Serial.begin(115200);
  rf.setTransmit(11);
  Serial.println("Ready!");
}

void loop() {
  Serial.print("Transmitting...");
  rf.transmitData(1234);
  Serial.println("done!");
  delay(5000);
}
