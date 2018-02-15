#include "../gradewrf.h"

int main(int argc, char *argv[])
{
        GradewRF rf;
        rf.setTransmit(17); // Use Broadcom chip 17
        rf.transmitData(0x01020304);
        return 0;
}
