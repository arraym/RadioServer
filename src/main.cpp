#include <Arduino.h>
#include "messages.h"
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <RH_ASK.h>
#include <SPI.h>                // Not actually used but needed to compile

// Radio driver defines
#define RX_PIN      5           // D1
#define TX_PIN      4           // D2
#define PTT_PIN     15          // D8 (unused)

// TFT display defines
#define TFT_RST     -1
#define TFT_CS      2           // D4
#define TFT_DC      0           // D3

#define DEFAULT_ROT 2           // Default TFT rotation

// Sensors destination addreses (more can be added)
#define SENSOR1     0x01        // Sensor1 destination address
#define SENSOR2     0x02        // Sensor2 destination address
#define SENSOR3     0x03

#define SENSOR_TIME 1000        // Time to wait for sensor response

void Send_Command(Command_TypeDef cmd, uint8_t address);

typedef struct
{
    String          name;
    uint8_t         address;
} Sensor_HandleTypeDef;

Sensor_HandleTypeDef sensorArray[] = {
    {"Sensor1", SENSOR1},
    {"Sensor2", SENSOR2},
    {"Sensor3", SENSOR3},
};    

RH_ASK driver(2000, RX_PIN, TX_PIN, PTT_PIN);  
                                // ESP8266 or ESP32: do not use pin 11 or 2
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
unsigned long startTime;

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

    tft.initR(INITR_144GREENTAB);
    tft.setTextWrap(false);
    tft.fillScreen(ST7735_BLACK);
    tft.setTextSize(1);
    tft.setRotation(DEFAULT_ROT);
}

void loop()
{
    uint8_t sensorsCount = sizeof(sensorArray) / sizeof(Sensor_HandleTypeDef);
    
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(0, 0);

    for (uint8_t i = 0; i < sensorsCount; i++)
    {
        Serial.println(sensorArray[i].name);
        tft.println();
        tft.setTextColor(ST7735_BLUE);
        tft.setTextSize(2);
        tft.println(sensorArray[i].name);
        tft.setTextColor(ST7735_CYAN);
        tft.setTextSize(1);
        Send_Command(GET_TEMPERATURE, sensorArray[i].address);
        delay(SENSOR_TIME);
        tft.setTextColor(ST7735_CYAN);
        Send_Command(GET_HUMIDITY, sensorArray[i].address);           
    }

    delay(2000);
}

/**
  * @brief	Sends a command to sensor and process the response
  * @param	cmd - command code
  * @param  address - sensor address
  * @retval	None
  */
void Send_Command(Command_TypeDef cmd, uint8_t address)
{
    uint8_t buf[RESPONSE_SIZE];
    uint8_t buflen = sizeof(buf);
    Command newCommand(cmd, address);

    driver.send(newCommand.commandArray, COMMAND_SIZE);
    driver.waitPacketSent();

    startTime = millis();
    while (!driver.waitAvailableTimeout(SENSOR_TIME))
    {
        if (millis() - startTime > SENSOR_TIME - 1)
        {
            Serial.println("Unavailable!");
            tft.setTextColor(ST7735_ORANGE);
            tft.println("Unavailable!");
            return;
        }
        
    }
    driver.recv(buf, &buflen);
    Response newResponse(buf);   
    if (newResponse.rsp == ACK)
    {
        switch (cmd)
        {
        case GET_TEMPERATURE:
        {
            float temperature = (float)newResponse.payload / 100;
            Serial.print("Temperature: ");
            Serial.print(temperature, 2);
            Serial.println("*C");
            tft.print("Temp: ");
            tft.print(temperature, 2);
            tft.println("*C");
            break;
        }
        case GET_HUMIDITY:
        {
            float humidity = (float)newResponse.payload / 100;
            Serial.print("Humidity: ");
            Serial.print(humidity, 2);
            Serial.println("%");
            tft.print("Hdt: ");
            tft.print(humidity, 2);
            tft.println("%");
            break;
        }
        default:
            break;
        }
    }
    else
    {
        Serial.println("Error!");
    }
}