#include <stdio.h>    
#include <stdlib.h>
#include <unistd.h>
#include <wiringPi.h> // Include WiringPi library!


int main(int argc, char* argv[]) {
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers

    int nPinGate = atoi(argv[1]); //first argument is pin of IR-receiver
    int nPinLED = atoi(argv[2]);  //second argument is pin of LED to activate

    printf("listening on bcm pin %d, activating LED on pin%d\n", nPinGate, nPinLED);	

    pinMode(nPinGate, INPUT);
    pinMode(nPinLED, OUTPUT);

    int nState = 0, nLastState = 0;
    while (true) {
            nState = digitalRead(nPinGate);
            if (nState != nLastState) {
                    printf("state changed! -> %d\n", nState);
                    digitalWrite(nPinLED, 1);
                    usleep(1000000);
                    digitalWrite(nPinLED, 0);
                    nLastState = digitalRead(nPinGate);
            }
            usleep(10);
    }
    return 0;
}

