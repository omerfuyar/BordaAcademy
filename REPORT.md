# Borda Academy Qualification Exam Report

**Author:** Omer Faruk Uyar

## 1. Introduction and Tools

- **MCU:** ESP32-WROOM-32
- **IDE / Build System:** VS Code with PlatformIO
- **Framework:** ESP-IDF (v5.x), utilizing FreeRTOS for concurrency
- **Bluetooth Stack:** Bluedroid Dual-mode / BLE only

## 2. Hardware and Sensors

The project utilizes the following sensors. The I2C sensors are configured with unique addresses to prevent bus collisions:

_(Note: Verify the Default I2C addresses against the ones you configured in your specific initialization code, e.g., `i2c_sensor_read()`, and update them below if necessary!)_

| Sensor Type         | Module  | Interface    | I2C Address            | DataSheet Link                                                                                                                 |
| :------------------ | :------ | :----------- | :--------------------- | :----------------------------------------------------------------------------------------------------------------------------- |
| **Light Sensor**    | BH1750  | I2C          | `0x23`                 | [BH1750 Datasheet](http://handsontec.com/dataspecs/sensor/BH1750%20Light%20Sensor.pdf)                                         |
| **Humidity / Temp** | AHT10   | I2C          | `0x38`                 | [AHT10 Datasheet](https://datasheet4u.com/pdf/1545327/AHT10.pdf)                                                               |
| **Pressure**        | BME280  | I2C          | `0x76` (or `0x77`)     | [BME280 Datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf)             |
| **Analog Sound**    | KY-038  | Analog (ADC) | N/A                    | [KY-038 Datasheet](https://eclass.uth.gr/modules/document/file.php/E-CE_U_269/Sensors/Sensors_%20Datasheets/KY-038-Joy-IT.pdf) |
| **Display**         | LCD1602 | I2C/Parallel | `0x27` (with backpack) | [LCD1602 Datasheet](https://www.waveshare.com/datasheet/LCD_en_PDF/LCD1602.pdf)                                                |

## 3. Algorithm and Flowchart

_(Insert your Flowchart image here. E.g., `![Flowchart](flowchart.png)`)_

- **Producer Task:** Wakes up every 1000ms (1Hz), reads all sensors over I2C/ADC, and attempts to push the packed data into the `sensorDataQueue`.
- **Consumer Task:** Blocks indefinitely until data arrives in the queue. Upon receiving, it pushes data through a moving median filter buffer, updates the LCD, and increments an internal counter.
- **BLE Broadcast:** Every 30 iterations (30 seconds), the Consumer compiles the queue's calculated statistics (Min, Max, Median, StdDev), packs and compresses them into a 23-byte struct, and triggers a BLE Advertisement update (`esp_ble_gap_start_advertising`).

## 4. Bluetooth Low Energy (BLE) Observation

The ESP32 acts as a BLE Peripheral (Broadcaster / Beacon). The calculated statistics are compressed into a 31-byte standard advertisement packet as `Manufacturer Specific Data`.

**Observed Raw BLE Payload via nRF Connect:**  
_(Insert your nRF Connect screen capture here!)_

`0x17FFFFFF181B1A00313232006F727100FFFFFF02CDCDCD00`

**Packet Decoding Breakdown:**

- **BLE Header (4 header bytes)**
  - `17` = Payload Length (23 bytes following this byte)
  - `FF` = AD Type (Manufacturer Specific Data)
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

## 5. RTOS Concurrency and Race Conditions (Bonus Part)

### 5.1 Defining and Solving the Race Condition

**Race Condition Definition:** A race condition occurs when two or more threads attempt to access and modify shared data simultaneously, leading to unpredictable states or memory corruption.

**Implemented Solution:** In this project, FreeRTOS queues (`xQueueSend` and `xQueueReceive`) inherently act as thread-safe circular buffers. Under the hood, FreeRTOS utilizes Interrupt Masking and Mutexes/Spinlocks to guarantee atomic read/write operations when adding or removing items from `sensorDataQueue`. Filter logic is entirely contained within the Consumer thread, avoiding the need for additional global variable mutexing.

### 5.2 Handling Producer/Consumer Speed Mismatch

**The Scenario:** Since the Producer writes to the queue at 1Hz, if the Consumer takes longer than 1 second to filter data, update the LCD, and construct BLE packets, it cannot keep up. Over time, the circular buffer (`sensorDataQueue`) will fill up completely.

**The Solution Implementation:** When the queue reaches its maximum capacity (`SENSOR_DATA_QUEUE_LENGTH`), calling `xQueueSend(..., 0)` with a `0` block-time will immediately fail and return `errQUEUE_FULL`.

**Fate of the Last Read Data:** I decided to **drop the newest measurement**. I chose this approach to guarantee the Producer never violates its strict real-time 1Hz deadline by blocking on a full queue. Old valid data continues to be drained gracefully by the consumer.

**Logging output:** The Producer outputs a warning to the serial monitor:  
`W (...) TEST_APP: Consumer is too slow, Queue is full. Dropping latest measurement.`
