all: transmit receive

transmit: transmit.cpp ../gradewrf.cpp ../gradewrf.h
	@echo "Building transmit.cpp..."
	@g++ -lwiringPi -o transmit transmit.cpp ../gradewrf.cpp

receive: receive.cpp ../gradewrf.cpp ../gradewrf.h
	@echo "Building receive.cpp..."
	@g++ -lwiringPi -o receive receive.cpp ../gradewrf.cpp

clean:
	@\rm -f transmit receive

