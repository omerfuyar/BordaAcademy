# Borda Academy Qualification Exam Report

**Author:** Omer Faruk Uyar
**GitHub Repo:** https://github.com/omerfuyar/BordaAcademy

## Tools

- **MCU:** ESP32-WROOM-32
- **IDE / Build System:** VS Code with PlatformIO
- **Framework / Toolchain:** ESP-IDF (v5.x) + freeRTOS
- **Bluetooth:** Bluedroid Dual-mode / BLE only

I chose ESP32 as my MCU (although I prefer ARM platforms like STM32) because it has wireless capabilities on board.

## Hardware

- **ESP-32:** [ESP32 DataSheet](https://documentation.espressif.com/esp32_datasheet_en.pdf)
- **Light Sensor:** [BH1750 Datasheet](http://handsontec.com/dataspecs/sensor/BH1750%20Light%20Sensor.pdf)
- **Humidity / Temperature Sensor:** [AHT10 Datasheet](https://datasheet4u.com/pdf/1545327/AHT10.pdf)
- **Pressure Sensor:** [BME280 Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf)
- **Analog Sound Sensor:** [KY-038 Datasheet](https://eclass.uth.gr/modules/document/file.php/E-CE_U_269/Sensors/Sensors_%20Datasheets/KY-038-Joy-IT.pdf)
- **LCD Display:** [LCD1602 Datasheet](https://www.waveshare.com/datasheet/LCD_en_PDF/LCD1602.pdf)

I currently dont have and sensor unit that uses I2C protocol, so I first started the project using analog sound sensor and I2C LCD display. At the end I wanted to keep them instead of excluding.

## Algorithm

- **Producer Task:** Loops with a frequency of 1Hz, reads all sensors over I2C/ADC, and tries pushing the packed data into the `sharedDataQueue`. (Has a higher priority than the Consumer task.)
- **Consumer Task:** Blocks indefinitely until data arrives in the queue. On receiving data, it pushes data through a moving median filter buffer, updates the LCD, and increments an internal BLE counter.
- **BLE Broadcast:** Every 30 iterations (30 seconds), the Consumer calculates the queues' calculated statistics (Min, Max, Median, StdDev), packs and compresses them into a 23-byte struct, and advertises it over BLE.

The flowchart representation can be found both in resources folder and [here](https://canva.link/ka4ovn1f75h7q53). 

## Observations

**Packet Data:**

`0x17FFFFFF181B1A00313232006F727100FFFFFF02CDCDCD00`

- **BLE Header (4 header bytes)**
  - `17` = Payload Length (23 bytes following this byte)
  - `FF` = AD Type (`0xFF`)
  - `FF FF` = Company ID (`0xFFFF`)
- **Temperature (4 signed bytes)**
  - `18` (Min: 24°C), `1B` (Max: 27°C), `1A` (Med: 26°C), `00` (StdDev: 0)
- **Humidity (4 unsigned bytes)**
  - `31` (Min: 49%), `32` (Max: 50%), `32` (Med: 50%), `00` (StdDev: 0)
- **Pressure (4 unsigned bytes) _[-900 hPa offset applied]_:**
  - `6F` (Min: 1011hPa), `72` (Max: 1014hPa), `71` (Med: 1013hPa), `00` (StdDev: 0)
- **Light (4 bytes) _[scaled to 255]_:**
  - `FF` (Min: 255), `FF` (Max: 255), `FF` (Med: 255), `02` (StdDev: 2)
- **Sound (4 bytes):**
  - `CD` (Min: 205), `CD` (Max: 205), `CD` (Med: 205), `00` (StdDev: 0)

Screenshots of the scanner app can be found in the resources folder. Data history shows that data is transmitted continuously and values change once every 30 seconds as expected.

## RTOS Concurrency and Race Conditions

### Defining and Solving the Race Condition

**Race Condition:** A race condition is the data corruption that is occurs when two distinct processing units tries to access to the same memory location at the same time.

**Solution:** In this project, freeRTOS queues (`xQueueSend` and `xQueueReceive`) are naturally thread-safe circular buffers. Inside both of the functions, freeRTOS uses Mutex Locks / Semaphores to guarantee atomic read/write operations when adding or removing items from `sharedDataQueue`. Filter logic is entirely contained within the Consumer thread, avoiding the need for additional global Mutex variables.

### Handling Producer/Consumer Speed Mismatch

**Scenario:** Since the Producer writes to the queue at 1Hz, if the Consumer takes longer than 1 second to filter data, update the LCD, and construct BLE packets, it cannot keep up. Over time, the circular buffer (`sharedDataQueue`) will fill up completely.

**Solution:** When the queue reaches its maximum capacity (`SHARED_DATA_QUEUE_LENGTH (16)`), calling `xQueueSend(..., 0)` with a `0` block-time will immediately fail and return `errQUEUE_FULL`. Then producer drops the oldest measurement explicitly and pushes the new measurement. That way the application will not panic and the system will continue to operate as expected and predictable.

**Logging output:** The Producer outputs a warning to the serial monitor:  
`W (...) APP: Consumer is too slow, Queue is full. Dropping the oldest measurement.`
