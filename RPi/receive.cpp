#include "../gradewrf.h"

int main(int argc, char *argv[])
{
        GradewRF rf;
        rf.setReceive(24); // Use Broadcom chip 24
	while(1){
		if(rf.isDataAvailable()){
			printf("Received: %lu\n", rf.getData32());
			rf.resetData();
		}
		delay(1);
	}
        return 0;
}
