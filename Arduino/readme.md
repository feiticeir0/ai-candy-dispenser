## Arduino Sketch and auxiliary files
### Uploading Code to the Seeed Studio Xiao ESP32S3 Sense

To run the Candy Machine on the Xiao ESP32S3 Sense, with microphone and the LCD (Xiao round display for XIAO), we need to configure the Arduino IDE development environment properly.

### ✅ Requirements

- Arduino IDE installed
- ESP32-S3 board added via **Boards Manager** (Espressif)
- Official `ESP32` library (v2.0.7 or higher recommended)

### Libraries
- `TFT_eSPI`
- `ArduinoJSON`
- `PNGdec`
- `Wire`

#### Custom libraries (included in the repository)
- `CHSC6X_Touch.h` **( Xiao Round Display Touch screen)**
- `cookie.h`       **(cookie image)**
- `no_cookies.h`   **(no cookie image)**

---

### ⚙️ Required Settings in Arduino IDE

Set these under **Tools**:

- **PSRAM**: ✔️ Enabled ✅ required for audio/transcription
- **Flash Size:**: "8MB (64Mb)"

![Options](./assets/arduino_options.jpg)

## ⚠️ Warning
You can't change much regarding Xiao Pins. Some are used by the Sense, byt the Round Display, I2C, etc.
There aren't many available. 

You can read more about it on Seeedstudio Wiki
[https://wiki.seeedstudio.com/xiao_esp32s3_pin_multiplexing/](https://wiki.seeedstudio.com/xiao_esp32s3_pin_multiplexing/)
[https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/](https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/)


## Options to change
You need to change some variables in the code. 

### WiFi Network

```c
// Change here your wifi password and network
const char* ssid = "<your_wifi_ssid>";
const char* password = "<your_wifi_network_passsword>";

// change here the IP of the server - change change the IP address, nothing else
const char* apiUrl = "http://<your_server_IP>:8000/analyze-audio"; 
```

### Recording
By default, we record a 5 seconds phrase. If you want to record more or less, you need to change the following variable
```c
#define MAX_WAV_SIZE (16000 * 2 * 5) // 5s a 16kHz mono 16-bit
```
Change the 5 for the number of seconds you want to record. 

### Motor rotation
By default, and after some testing for this motor, 1s is enough for a full rotation. If your motor takes longer or less to a full rotation (to make sure a candy exits), you need to adjust the following variable, that resides in the **cookieTask** function. The value is in miliseconds. 
```c
const unsigned long duration = 1000; // how long the motor turns
```

## Images
The displayed images are C flash arrays.

The instructions are inside the TFT_eSPI examples, Flash_PNG sketch. 
```
// Image files can be converted to arrays using the tool here:
// https://notisrac.github.io/FileToCArray/
// To use this tool:
//   1. Drag and drop file on "Browse..." button
//   2. Tick box "Treat as binary"
//   3. Click "Convert"
//   4. Click "Save as file" and move the header file to sketch folder
//   5. Open the sketch in IDE
//   6. Include the header file containing the array 
```
We have *cookie.h* and *no_cookies.h* files, that are C Flash Arrays . 


