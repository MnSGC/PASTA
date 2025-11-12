# pixy2_for_pi_using_steppermotor
 a modified version of pixy2 Pan_tilt using a stepper motor on a pi

# Setup
This code assumes you have pixy2 library and wiringPi downloaded onto your Raspberry Pi. To properly install the pixy2 library onto your Pi as a header file for C++ use
```
cp pixy2_example/scripts/install_libpixyusb2.sh 
cd pixy2/scripts/ cd pixy2/scripts/ 
sudo ./install_libpixyusb2.sh
```
These instruction and a lot of the build files come from Manuel Meraz's pixy2_example repsitory. Please refer to the repository to properly understand how the build works.



# Stepper motor
The stepper motor is connected to the Pi using an sparkfun easydriver motor controller connected to the Raspberry Pi using the GPIO pins. If you have the pixy2 and stepper motor connected to the Raspberry pi, you can move on to running the ```pan_tilt_demo``` program. 



# Running pan_tilt_demo
To run it, open the terminal and type
```
cd pixy2_for_pi_using_steppermotor/build/examples
./pan_tilt_demo

```
This will allow the pixy to trigger a searching algorithm until it finds the object it wants to track. Once it finds the object to track, it will stop searching and try its best to keep track of the desired object.


# editing the code
To modify the code for the ```pan_tilt_demo``` or any other program in the ```build/examples``` directory, you can edit the C++ files in ```pixy2_for_pi_using_steppermotor/examples``` directory.
To make the files change to the ```build``` directory and write the commands below

```
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j6
```

This should make all the files necessary, you can ignore the warnings as they are associated with unused variables for the tilt commands which haven't been added yet.

# References
Manuel Meraz's repository 

https://github.com/ManuelMeraz/pixy2_example
