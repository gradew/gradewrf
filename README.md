# gradewrf

## Transmission
```
rf.setTransmit(17); // !!!! ON A RASPBERRY, USE BROADCOM CHIP NUMBER !!!!
rf.transmitData(0x01020304);
```

## Reception
```
rf.setReceive(0); // !!!! MUST USE INTERRUPT NUMBER !!!!
// ... then, in a loop...
if(rf.isDataAvailable()){
  unsigned long val=rf.getData32();
  rf.resetData(); // get ready for the next transmission
}
```
