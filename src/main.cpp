#include <Arduino.h>
#include "messages.h"
#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h>                // Not actually used but needed to compile
#endif

// Sensors destination addreses (more can be added)
#define SENSOR1     0x01        // Sensor1 destination address
#define SENSOR2     0x02        // Sensor2 destination address

RH_ASK driver(2000, 4, 5, 0);   // ESP8266 or ESP32: do not use pin 11 or 2

void setup()
{
#ifdef RH_HAVE_SERIAL
    Serial.begin(115200);	    // Debugging only
#endif
    if (!driver.init())
#ifdef RH_HAVE_SERIAL
         Serial.println("init failed");
#else
	;
#endif
}

void loop()
{
    Command myCommand(GET_TEMPERATURE, SENSOR1);

    driver.send(myCommand.commandArray, COMMAND_SIZE);
    driver.waitPacketSent();
    delay(1000);
}