#include "../gradewrf.h"

int main(int argc, char *argv[])
{
        GradewRF rf;
        rf.setReceive(17); // Use Broadcom chip 17
	while(1){
		if(rf.isDataAvailable()){
			printf("Received: %lu\n", rf.getData32());
			rf.resetData();
		}
		delay(1000);
	}
        return 0;
}
