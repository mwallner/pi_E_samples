#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>

#define BCM_ULTRASONIC_TRIGGER 17
#define BCM_ULTRASONIC_ECHO 4

const double SPEED_SOUND = 343.0; // m/s

int us_nCounterEdgeDown = 0;
int us_nCounterEdgeUp   = 0;
struct timeval us_starttime, us_endtime;

void ISR_ECHO(void) {
    struct timeval now;
    int nInputState;

    gettimeofday(&now, NULL);
    nInputState = digitalRead(BCM_ULTRASONIC_ECHO);
    if (HIGH ==  nInputState) {
            us_starttime = now;
            us_nCounterEdgeUp++;
    } else {
            us_endtime = now;
            us_nCounterEdgeDown++;
    }
}

double getDistanceMM() {
    digitalWrite(BCM_ULTRASONIC_TRIGGER, HIGH);
    usleep(5000);
    digitalWrite(BCM_ULTRASONIC_TRIGGER, LOW);
    usleep(20000);
    
    double secs_elapsed = (us_endtime.tv_sec - us_starttime.tv_sec) + (us_endtime.tv_usec - us_starttime.tv_usec) / 1000000.0;
    double distance = SPEED_SOUND * 100 * 10 * secs_elapsed / 2; //mm
    printf("distance: %.1f mm (time elapsed: %g s)\n", distance, secs_elapsed);
    return distance;
}

int main(int argc, char* argv[]) {
    wiringPiSetupGpio(); // Initialize wiringPi -- using Broadcom pin numbers

    pinMode(BCM_ULTRASONIC_ECHO, INPUT);
    pinMode(BCM_ULTRASONIC_TRIGGER, OUTPUT);
    
    if (wiringPiISR(BCM_ULTRASONIC_ECHO, INT_EDGE_BOTH, &ISR_ECHO)  < 0) {
        printf("Unable to setup ISR (ULTRASONIC) for GPIO %d (%s)\n\n", BCM_ULTRASONIC_ECHO, strerror(errno));
        exit(1);
    }
    while (true) {
        usleep(100000);
        getDistanceMM();
    }
    return 0;
}

