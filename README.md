# NeuroVoice

NeuroVoice is a project that helps non-verbal individuals answer with YES or NO using EEG signals.

Two flickering areas appear on the screen: YES at 8 Hz and NO at 15 Hz. The person looks at the desired response, and the ESP32 analyzes the signal received via the ADS1115. The interface uses the responses to build and speak a message, for example: "I need water".

The components used are an ESP32, an ADS1115, an EEG amplifier, and electrodes.

## Documentation


  ### Used libraries

- [Adafruit ADS1X15](https://github.com/adafruit/Adafruit_ADS1X15)
- [arduinoFFT](https://github.com/kosme/arduinoFFT)
- [LiquidCrystal I2C](https://docs.arduino.cc/libraries/liquidcrystal-i2c/)
