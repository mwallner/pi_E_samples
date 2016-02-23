#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>
#include <wiringPi.h> // Include WiringPi library!

int nPinGate;
int nPinLED;

void ISR_GATE(void) {
    digitalWrite(nPinLED, 1);
    usleep(1000000);
    digitalWrite(nPinLED, 0);
}

int main(int argc, char* argv[]) {
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers

    nPinGate = atoi(argv[1]); //first argument is pin of IR-receiver
    nPinLED = atoi(argv[2]);  //second argument is pin of LED to activate

    printf("listening on bcm pin %d, activating LED on pin%d\n", nPinGate, nPinLED);	

    pinMode(nPinGate, INPUT);
    pinMode(nPinLED, OUTPUT);

    if (wiringPiISR(nPinGate, INT_EDGE_BOTH, &ISR_GATE)  < 0) {
        printf("Unable to setup ISR for GPIO %d (%s)\n\n", nPinGate, strerror(errno));
        exit(1);
    }

    while (true) {
            usleep(100);
    }
    return 0;
}
