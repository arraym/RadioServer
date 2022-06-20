/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MESSAGES_h
#define MESSAGES_h

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/* Exported constants --------------------------------------------------------*/
#define COMMAND_SIZE    0x04    // 4 bytes
#define RESPONSE_SIZE   0x07    // 7 bytes
#define PAYLOAD_SIZE    0x04    // 4 bytes
#define CRC_SIZE        0x02    // 2 bytes

/* Exported classes ----------------------------------------------------------*/
class Command
{
    public:
        typedef enum
        {
            GET_TEMPERATURE     = ((uint8_t)0xA0),
            GET_HUMIDITY        = ((uint8_t)0xA1)
        } Command_TypeDef;

        Command_TypeDef cmd;
        uint8_t destAddress;
        uint16_t crc;

        Command(uint8_t *data);
        Command(Command_TypeDef c, uint8_t address);
};

class Response
{
    public:
        typedef enum
        {
            ACK                 = ((uint8_t)0x0A), 
            NACK                = ((uint8_t)0x0F)
        } Response_TypeDef;

        Response_TypeDef rsp;
        uint32_t payload;
        uint16_t crc;

        Response(uint8_t *data);
        Response(Response_TypeDef r, uint32_t load);
};

#endif  /* MESSAGES_H */