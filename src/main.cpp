#include <Arduino.h>
#include "messages.h"
#include <RH_ASK.h>
#ifdef RH_HAVE_HARDWARE_SPI
#include <SPI.h>                // Not actually used but needed to compile
#endif

// Sensors destination addreses (more can be added)
#define SENSOR1     0x01        // Sensor1 destination address
#define SENSOR2     0x02        // Sensor2 destination address

#define DELAY_INT   3000        // Time delay between commands

RH_ASK driver(2000, 5, 4, 0);   // ESP8266 or ESP32: do not use pin 11 or 2

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
    uint8_t buf[RESPONSE_SIZE];
    uint8_t buflen = sizeof(buf);

    Command myCommand(GET_HUMIDITY, SENSOR1);

    driver.send(myCommand.commandArray, COMMAND_SIZE);
    driver.waitPacketSent();

    if(driver.waitAvailableTimeout(DELAY_INT))
    {
        driver.recv(buf, &buflen);
        driver.printBuffer("Got: ", buf, buflen);
        Response recvResponse(buf);
        if(recvResponse.rsp == ACK)
        {
            Serial.print("Umiditate: ");
            Serial.print(recvResponse.payload, DEC);
            Serial.println("%");
        }
        else
        {
            Serial.println("Error, resending command!");
        }
    }
    delay(2000);
}