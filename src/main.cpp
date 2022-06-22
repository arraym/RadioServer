#include <Arduino.h>
#include <ESP8266WiFi.h>
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

#define SENSOR_TIME 5000        // Time to wait for sensor response

void Send_Command(Command_TypeDef cmd, uint8_t address);
void Init_WiFi();
void Make_IFTTTRequest(String sensorName, float value1, float value2);

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

const char* ssid     = "your_ssid";         // Write your SSID
const char* password = "your_password"; //Write your Password
const char* IFTTT_URL = "/with/key/cIfUuE0QZEZnYxkhn3VLJmyheKM9wRcqNczNkfbZw2G";
const char* server = "maker.ifttt.com";
unsigned long startTime;
float tempValue = 0;
float hdtValue = 0;

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

    Init_WiFi();
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
        delay(2000);
        tft.setTextColor(ST7735_CYAN);
        Send_Command(GET_HUMIDITY, sensorArray[i].address); 
        Make_IFTTTRequest(sensorArray[i].name, tempValue, hdtValue);
        tempValue = 0;
        hdtValue = 0;        
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
            tft.setTextColor(ST7735_RED);
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
            tempValue = (float)newResponse.payload / 100;
            Serial.print("Temperature: ");
            Serial.print(tempValue, 2);
            Serial.println("*C");
            tft.print("Temp: ");
            tft.print(tempValue, 2);
            tft.println("*C");
            break;
        }
        case GET_HUMIDITY:
        {
            hdtValue = (float)newResponse.payload / 100;
            Serial.print("Humidity: ");
            Serial.print(hdtValue, 2);
            Serial.println("%");
            tft.print("Hdt: ");
            tft.print(hdtValue, 2);
            tft.println("%");
            break;
        }
        default:
            break;
        }
    }
    else
    {
        Serial.println("Read error!");
        tft.setTextColor(ST7735_ORANGE);
        tft.println("Read error!");
        tempValue = 0;
        hdtValue = 0;
    }
}

/**
  * @brief	Initializes and connects to WiFi
  * @param	None
  * @retval	None
  */
void Init_WiFi()
{
    Serial.print("Connecting "); 
    tft.setTextColor(ST7735_CYAN);
    tft.println("WiFi connecting...");
    WiFi.begin(ssid, password);  

    int timeout = 10;       // 10 seconds
    while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0))
    {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("");

    if(WiFi.status() != WL_CONNECTED)
    {
        Serial.println("Failed to connect!");
        tft.setTextColor(ST7735_RED);
        tft.println("Failed!");
        delay(2000);
        return;
    }

    Serial.print("WiFi connected in: "); 
    Serial.print(millis());
    Serial.print(", IP address: "); 
    Serial.println(WiFi.localIP());
    tft.setTextColor(ST7735_GREEN);
    tft.println("Connected!");
    delay(2000);
}

/**
  * @brief	Makes a IFTTT request sending data to google sheets
  * @param	sensorName - name of the sensor
  * @retval	None
  */
void Make_IFTTTRequest(String sensorName, float value1, float value2)
{
    if (WiFi.status () != WL_CONNECTED)
    {
        Serial.println("WiFi not connected!");
        return;
    }

    if (value1 == 0 or value2 == 0)
    {
        Serial.println("Bad sensor reading!");
        return;
    }
    
    
    Serial.print("Connecting to "); 
    Serial.print(server);

    WiFiClient client;
    int retries = 5;
    while(!!!client.connect(server, 80) && (retries-- > 0))
    {
        Serial.print(".");
    }
    Serial.println();
    if(!!!client.connected())
    {
        Serial.println("Failed to connect...");
    }

    Serial.print("Request resource: "); 
    Serial.println(IFTTT_URL);

    String jsonObject = String("{\"value1\":\"") + value1 + "\",\"value2\":\"" + value2 + "\"}";
                        
                        
    client.println(String("POST ") + "/trigger/DHT22_" + sensorName + IFTTT_URL + " HTTP/1.1");
    client.println(String("Host: ") + server); 
    client.println("Connection: close\r\nContent-Type: application/json");
    client.print("Content-Length: ");
    client.println(jsonObject.length());
    client.println();
    client.println(jsonObject);
        
    int timeout = 5 * 10; // 5 seconds             
    while(!!!client.available() && (timeout-- > 0))
    {
        delay(100);
    }
    if(!!!client.available())
    {
        Serial.println("No response...");
    }
    while(client.available())
    {
        Serial.write(client.read());
    }

    Serial.println("\nClosing Connection");
    client.stop(); 
}