#include "SeeedGroveMP3.h"
#include "KT403A_Player.h"

#ifdef __AVR__
    #include <SoftwareSerial.h>
    SoftwareSerial SSerial(2, 3); // RX, TX
    #define COMSerial SSerial
    #define ShowSerial Serial

    KT403A<SoftwareSerial> Mp3Player;
#endif

#ifdef ARDUINO_SAMD_VARIANT_COMPLIANCE
    #define COMSerial Serial1
    #define ShowSerial SerialUSB

    KT403A<Uart> Mp3Player;
#endif

#ifdef ARDUINO_ARCH_STM32F4
    #define COMSerial Serial
    #define ShowSerial SerialUSB

    KT403A<HardwareSerial> Mp3Player;
#endif

void setup() {
  ShowSerial.begin(9600);
    COMSerial.begin(9600);
    while (!ShowSerial);
    while (!COMSerial);
    Mp3Player.init(COMSerial);
}

void loop() {
  Mp3Player.playSongSpecify(01,001);
  delay(11000);
}