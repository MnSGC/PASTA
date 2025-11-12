#include <lgpio.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <pthread.h>
#include <atomic>

#define AIN2 27
#define AIN1 17
#define PWMA 12
#define STBY 7

#define SERVO_GPIO 13                   

#define PWM_PERIOD_US 20000   // 20 ms = 50 Hz PWM period
#define MIN_PW 1000           // full reverse
#define MAX_PW 2000           // full forward
#define STOP_PW 1500          // stop


std::atomic<bool> servoRunning(true);  // thread control flag
std::atomic<int> currentSpeed(0);      // shared speed for servo thread


// Map speed (-100 to 100) to pulse width (1000 to 2000 µs)
int speedToPulseWidth(int speed) {
    if (speed < -100) speed = -100;
    if (speed > 100) speed = 100;
    return STOP_PW + ((MAX_PW - STOP_PW) * speed) / 100;
}

// Send one PWM pulse
void sendServoPulse(int h, int highTime_us) {
    int lowTime_us = PWM_PERIOD_US - highTime_us;
    std::cout << h << std::endl << SERVO_GPIO <<std::endl << lowTime_us <<std::endl;
    int result = lgTxPulse(h, SERVO_GPIO, highTime_us, lowTime_us, 0, 1);
    if (result < 0) {
        std::cerr << "lgTxPulse failed: " << result << std::endl;
    }
    usleep(PWM_PERIOD_US);
}

// Servo thread: keeps running while servoRunning = true
void* servoThreadFunc(void *arg) {
    int handle = *((int *) arg);  // retrieve handle from main
    while (servoRunning) {
        int pulseWidth = speedToPulseWidth(currentSpeed);
        std::cout << pulseWidth << std::endl;
        sendServoPulse(handle, pulseWidth);
    }
    return NULL;
}

// Set servo speed (continuous until stopped)

void setServoSpeed(int speed) {
    currentSpeed = speed;
}

int main() {
    // Open the GPIO chip
    int handle = lgGpiochipOpen(0);
    if (handle < 0) {
        std::cerr << "Failed to open GPIO chip\n";
        return 1;
    }

    // Claim the servo GPIO
    if (lgGpioClaimOutput(handle, 0, SERVO_GPIO, 0) != 0) {
        std::cerr << "Failed to claim SERVO_GPIO " << SERVO_GPIO << std::endl;
    }

    // Claim Actuator pins
    int pins[3] = {AIN2, AIN1, STBY};
    int levels[3] = {0, 0, 1};
    int status = lgGroupClaimOutput(handle, 0, 3, pins, levels);
    if (status != 0) {
        std::cerr << "actuator pin claim failed, status: " << status << std::endl;
    }

    if (lgGpioClaimOutput(handle, 0, PWMA, 1) != 0) {
        std::cerr << "Failed to claim PWMA pin" << std::endl;
    }

    // Start servo thread
    pthread_t servoThread;
    pthread_create(&servoThread, NULL, servoThreadFunc, (void *) (&handle));

    // ==== Example motion sequence ====
    std::cout << "Spin servo forward while extending actuator\n";
    setServoSpeed(75);  // start servo forward
    lgGpioWrite(handle, AIN1, 1); // extend
    lguSleep(5);

    std::cout << "Spin servo backward while retracting actuator\n";
    setServoSpeed(-75); // reverse servo
    lgGpioWrite(handle, AIN1, 0);
    lgGpioWrite(handle, AIN2, 1); // retract
    lguSleep(5);

    // Stop servo
    setServoSpeed(0);

    // Cleanup
    servoRunning = false;
    pthread_join(servoThread, NULL);

    lgGpioWrite(handle, AIN1, 0);
    lgGpioWrite(handle, AIN2, 0);
    lgGpioWrite(handle, SERVO_GPIO, 0);
    lgGpiochipClose(handle);

    return 0;
}
