# Project
## Description
An environmental sensing device will be designed in this project. Sensor device includes MCU, I2C sensors and BLE functionality. Project should include bunch of sensors for environmental monitoring. (Temperature, humidity, CO2 etc.) I2C peripheral of the MCU will be used to read environmental sensor data and BLE peripheral of the MCU will be used to send measurements in BLE packet. The packets will be read from BLE scanner device. (Mobile phone, BLE MCU, etc.)

* Project consists of several steps. In case you struggle in any task, you may pass, and use given answers to move to the next step. To advance to the next step, you can use given function prototypes by assuming these functions work perfectly.
* You can use any MCU, any IDE, any API and any I2C sensor to perform given tasks. But please provide brief information about your tools and datasheet of the sensor.
* If you don’t have any I2C sensors for environmental monitoring or don’t want to buy them, you can simulate mock environmental sensor data in the code with random values.

## Requirements
* When powered up, the device should start taking samples.
* Measurement sample frequency should be 1 Hz.
* The BLE packet advertisement interval should be 30 seconds.
* Draw a flow chart that represents your algorithm. (You can use any flow chart application such as draw.io, Miro etc.)

## Tasks

1. Design and write firmware for environmental sensing project.
* You will choose 3 different I2C sensor devices. (Temperature, humidity, CO2 etc.) Each sensor must have a unique I2C address. You should consult the datasheet of each sensor to determine the address options and select a unique address for each sensor.
+ Add links for sensor datasheets in your project report.
* To communicate with the I2C sensors, you will implement a C function that reads data from the sensors. You can use this function for each sensor if you cannot achieve previous part.
+ Bare metal programming or libraries can be used for peripheral.
+ If you could not answer this question, move to the next one and use “float i2c_sensor_read(uint8_t device_address, enum sensor_t sensor_type);”.
* Use a moving median filter to filter sensor reading results.
+ Write a moving median filter function to filter sensor reading results.
+ If you could not answer this question, move to next one and use “float filter_sensor_value(float raw_sensor_value, uint8_t window_size);”.
* Filtered data must be stored in a circular buffer. Queue filtered sensors’ data in the buffer.
+ Store desired amount of data in a static allocated buffer.
+ If overflow occurs, discard the oldest data.
+ If you could not answer this question, move to next one and use “int buffer_get_value(struct buf_handle_t *p_handle, float *p_sensor_data);”. (Returns 0 on success)
* By specifying an interval value for period, send the standard deviation, max, min and median value of the measurement results stored in the circular buffer using Bluetooth Low Energy.
+ The device should act as a peripheral (broadcaster) device that is responsible for sending the BLE Packets.
+ Standard deviation, max, min and median value of the collected measurements for each sensor should be calculated.
+ Calculated results for each sensor’s data should be stored in the same BLE packet to be sent. 
+ Observe the BLE packet from your Bluetooth scanner device. (Phone, BLE MCU, etc.) Share the results in your report.

2. Bonus Part
* Under RTOS, define two concurrent tasks (Producer and Consumer Threads)
+ Producer thread waits a condition variable which indicates that the queue includes at least one empty entry. If there is empty space then it takes mutex, puts the sensor data into circular queue, release mutex and sets a condition variable indicates that there are at least one data entry in the queue.
+ Consumer thread waits the condition variable which indicates that there are at least one entry in the circular queue. If the variable is set then takes mutex, read data, release mutex and set condition variable which indicates that there are at least one entry in the circular queue. After this it can apply and filter to the data.
* Define race condition between two tasks and implement solution.
* Define a case that consumer can't reach the speed of the producer. Generate a log for this case, decide the fate of the last read sensor data, explain your solution and reasons for this case and finally implement your solution.

## Note
You can freely increase the number of tasks if your design needs it.

## Embedded Software Engineers’ Toolbox 
This part includes interview discussion topics, we want you to do some research about them since they will be the second main discussion point after the project solutions. Even if you are not selected to the interview part, we think that these might prove useful in your embedded career as well, so it is also a kind of suggestion list for the starters

* RTOS
+ Semaphore, Mutex, Scheduling Algorithms, Race Condition, Multi-threading
* Microprocessor Architecture
* Embedded C
* What does Compiler & Linker do?
* Startup Code, LinkerFile, MapFile
* Stack and Heap as sections of RAM
* Journey of a Firmware from the compiling stage to main function’s first line.
* Common Serial Communication Protocols
* Bluetooth Low Energy and BLE 5.1 Standards (just the basics)
* NB-IoT, LTE-M, LoRa (just the basic)