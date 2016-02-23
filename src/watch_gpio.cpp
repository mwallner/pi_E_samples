#include <stdio.h>    
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h> // Include WiringPi library!



int main(int argc, char* argv[]) {
    	wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers

	int nPin = atoi(argv[1]);
	printf("listening on bcm pin %d\n", nPin);	
	
	int nState = 0, nLastState = 0;

	while (true) {
		nState = digitalRead(nPin);
		if (nState != nLastState) {
			printf("state changed! -> %d\n", nState);
			nLastState = nState;
		}	
		usleep(100);
		nState = nLastState;
	}	


	return 0;
}

