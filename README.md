# ESP WiFi Extender Using Network Address Port Translation (NAPT)

## Known working methods:

- For ESP8266
  - Add board https://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Create a new file `env.h` using the `env.h.example` template

- For ESP32
  - Add board https://github.com/tasmota/arduino-esp32/releases/latest/download/package_esp32_index.json
  - Download https://github.com/tasmota/arduino-esp32/releases/latest/download/framework-arduinoespressif32.zip
  - Extract to ~/.arduino15/packages/esp32/hardware/esp32/2.0.6/ or equivalent directory
  - Copy ~/.arduino15/packages/esp32/tools/esptool_py/ to ~/.arduino15/packages/esp32/hardware/esp32/2.0.6/tools/esptool/ or equivalent directory
  - Use `DOIT ESP32 DEVKIT V1` device in Arduino IDE
  - Create a new file `env.h` using the `env.h.example` template
