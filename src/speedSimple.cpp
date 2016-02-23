#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>


#define BCM_FIRST_GATE 27
#define LED_FIRST_GATE 25
#define BCM_SECOND_GATE 22
#define LED_SECOND_GATE 24
#define GATE_DISTANCE 75.0 /*mm*/

struct timeval starttime, endtime;
int _ACTIVE_MEASUREMENT_ = 0;

void calcSpeed() {
    double secs_elapsed, speed;
    secs_elapsed = (endtime.tv_sec - starttime.tv_sec) + (endtime.tv_usec - starttime.tv_usec)/1000000.0;
    speed = GATE_DISTANCE/secs_elapsed;
    printf("\n");
    printf("distance: %lf mm\n", GATE_DISTANCE);
    printf("seconds elapsed: %lf sec\n", secs_elapsed);
    printf("speed: %lf mm/sec\n", speed);
    printf("\n");
}


void reset() {
    usleep(200000);
    digitalWrite(LED_FIRST_GATE, 0);
    digitalWrite(LED_SECOND_GATE, 0);
    _ACTIVE_MEASUREMENT_ = 0;
}


void FIRST_GATE_PASS(void) {
    if (_ACTIVE_MEASUREMENT_) {
        return;
    }
    _ACTIVE_MEASUREMENT_ = 1;
    digitalWrite(LED_FIRST_GATE, 1);
    struct timeval now;
    gettimeofday(&now, NULL);
    starttime = now;
    printf("FIRST_GATE_PASS\n");
}


void SECOND_GATE_PASS(void) {
    if (!_ACTIVE_MEASUREMENT_) {
        return;
    }
    digitalWrite(LED_SECOND_GATE, 1);
    struct timeval now;
    gettimeofday(&now, NULL);
    endtime = now;
    printf("SECOND_GATE_PASS\n");
    calcSpeed();
    reset();
}


int main(int argc, char* argv[]) {
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers

    pinMode(BCM_FIRST_GATE, INPUT);
    pinMode(LED_FIRST_GATE, OUTPUT);
    pinMode(BCM_SECOND_GATE, INPUT);
    pinMode(LED_SECOND_GATE, OUTPUT);
    
    if (wiringPiISR(BCM_FIRST_GATE, INT_EDGE_BOTH, &FIRST_GATE_PASS)  < 0) {
            printf("Unable to setup ISR for GPIO %d (%s)\n\n", BCM_FIRST_GATE, strerror(errno));
            exit(1);
    }
    if (wiringPiISR(BCM_SECOND_GATE, INT_EDGE_BOTH, &SECOND_GATE_PASS)  < 0) {
            printf("Unable to setup ISR for GPIO %d (%s)\n\n", BCM_SECOND_GATE, strerror(errno));
            exit(1);
    }
	
    while (true) {
            usleep(100);
    }

    return 0;
}

