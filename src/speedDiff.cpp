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

#define BCM_ULTRASONIC_TRIGGER 17
#define BCM_ULTRASONIC_ECHO 4

const double SPEED_SOUND = 343.0; // m/s
#define GATE_DISTANCE 75.0 /*mm*/

int _ACTIVE_MEASUREMENT_ = 0;

//////////////////////////////// ULTRASONIC_SENSOR ////////////////////////////////
int us_nCounterEdgeDown = 0;
int us_nCounterEdgeUp   = 0;
struct timeval us_starttime, us_endtime;

void ISR_ECHO(void) {
    if (!_ACTIVE_MEASUREMENT_) {
        return;
    }
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

double _getDistanceMM(double *pDTimeWastedMSec) {
    digitalWrite(BCM_ULTRASONIC_TRIGGER, HIGH);
    usleep(5000);
    digitalWrite(BCM_ULTRASONIC_TRIGGER, LOW);
    usleep(20000);
    if (pDTimeWastedMSec)
        *pDTimeWastedMSec += 25;
    
    double secs_elapsed = (us_endtime.tv_sec - us_starttime.tv_sec) + (us_endtime.tv_usec - us_starttime.tv_usec) / 1000000.0;
    double distance = SPEED_SOUND * 100 * 10 * secs_elapsed / 2; //mm
    if (distance < 30) {
	printf("sensor returned invalid distance!\n");
	return 0;
    }
    printf("distance: %.1f mm (time elapsed: %g s)\n", distance, secs_elapsed);
    return distance;
}

double getUltrasonicSpeed() {
    double dMea1 = _getDistanceMM(0);
    double dTimeBetweenMeasurementMSec = 10;
    usleep(dTimeBetweenMeasurementMSec*1000);
    double dTimeWastedMSec = 0.0;
    double dMea2 = _getDistanceMM(&dTimeWastedMSec);
    double dDistance = dMea1 - dMea2;
    if (dDistance < 0) {
        printf("invalid measurement from ultrasonic sensor!\n");
        return 0.0;
    }
    double dTimeTotal = dTimeWastedMSec/1000.0 + dTimeBetweenMeasurementMSec/1000.0;
    printf("us_distance: %lf mm\n", dDistance);
    printf("us_seconds elapsed: %lf sec\n", dTimeTotal);
    double dSpeed = dDistance/dTimeTotal;
    printf("us_speed: %lf mm/sec\n", dSpeed);
    
    return dSpeed;
}

//////////////////////////////////// IR-GATES /////////////////////////////////////
struct timeval starttime, endtime;

double getGateSpeed() {
    double secs_elapsed, speed;
    secs_elapsed = (endtime.tv_sec - starttime.tv_sec) + (endtime.tv_usec - starttime.tv_usec)/1000000.0;
    speed = GATE_DISTANCE/secs_elapsed;
    printf("gate_distance: %lf mm\n", GATE_DISTANCE);
    printf("gate_seconds elapsed: %lf sec\n", secs_elapsed);
    printf("gate_speed: %lf mm/sec\n", speed);
    return speed;
}

void calcSpeed() {
    printf("\n");
    double dGateSpeed = getGateSpeed();
    printf("\n");
    double dUSSpeed = getUltrasonicSpeed();
    printf("\n");
    printf("diff: %lf\n", dUSSpeed-dGateSpeed);
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
    pinMode(BCM_ULTRASONIC_ECHO, INPUT);
    pinMode(BCM_ULTRASONIC_TRIGGER, OUTPUT);
    
    if (wiringPiISR(BCM_FIRST_GATE, INT_EDGE_BOTH, &FIRST_GATE_PASS)  < 0) {
        printf("Unable to setup ISR (FIRST_GATE) for GPIO %d (%s)\n\n", BCM_FIRST_GATE, strerror(errno));
        exit(1);
    }
    if (wiringPiISR(BCM_SECOND_GATE, INT_EDGE_BOTH, &SECOND_GATE_PASS)  < 0) {
        printf("Unable to setup ISR (SECOND_GATE) for GPIO %d (%s)\n\n", BCM_SECOND_GATE, strerror(errno));
        exit(1);
    }
    if (wiringPiISR(BCM_ULTRASONIC_ECHO, INT_EDGE_BOTH, &ISR_ECHO)  < 0) {
        printf("Unable to setup ISR (ULTRASONIC) for GPIO %d (%s)\n\n", BCM_ULTRASONIC_ECHO, strerror(errno));
        exit(1);
    }
	
    while (true) {
            usleep(100);
    }
    return 0;
}

