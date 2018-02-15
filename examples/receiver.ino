#include <gradewrf.h>

GradewRF rf=GradewRF();

void setup() {
  Serial.begin(115200);
  rf.setReceive(0);
  Serial.println("Ready!");
}

void loop() {
  if(rf.isDataAvailable()){
    Serial.println("Received data");
    Serial.println(rf.getData32());
    rf.resetData();
  }
}
