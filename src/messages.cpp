/* Includes ------------------------------------------------------------------*/
#include "messages.h"
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
uint16_t Set_CRC(uint8_t *data, uint8_t size);
bool Verify_CRC(uint8_t *data, uint8_t size);
/* Exported member functions -------------------------------------------------*/

/**
  * @brief	One parameter Command class constructor
  * @param	*data - received data array
  * @retval	None
  */
Command::Command(uint8_t *data)
{
    cmd = (Command_TypeDef)data[0];
    destAddress = data[1];
    memcpy((uint8_t*)&crc, (uint8_t*)&data[2], CRC_SIZE);
    if(!Verify_CRC(data, COMMAND_SIZE))
        cmd = (Command_TypeDef)0xFF;
}

/**
  * @brief	Two parameters Command class constructor
  * @param	c - command
  * @param  address - destination address
  * @retval	None
  */
 Command::Command(Command_TypeDef c, uint8_t address)
 {
    uint16_t data = (uint16_t)(c << 8) | address;

    cmd = c;
    destAddress = address;
    crc = Set_CRC((uint8_t*)&data, COMMAND_SIZE);
 }

/**
  * @brief	One parameter Response class constructor
  * @param	*data - received data array
  * @retval	None
  */
 Response::Response(uint8_t *data)
 {
    rsp = (Response_TypeDef)data[0];
    memcpy((uint8_t*)&payload, (uint8_t*)&data[1], PAYLOAD_SIZE);
    memcpy((uint8_t*)&crc, (uint8_t*)&data[5], CRC_SIZE);
    if(!Verify_CRC(data, RESPONSE_SIZE))
        rsp = NACK;
 }

 /**
  * @brief	Two parameters Response class constructor
  * @param	r - response
  * @param  load - payload
  * @retval	None
  */
 Response::Response(Response_TypeDef r, uint32_t load)
 {
    uint8_t data[RESPONSE_SIZE - CRC_SIZE];

    data[0] = r;
    memcpy((uint8_t*)&data[1], (uint8_t*)&load, PAYLOAD_SIZE);
    rsp = r;
    payload = load;
    crc = Set_CRC((uint8_t*)&data, RESPONSE_SIZE);
 }

 /**
  * @brief	Calculates CRC for a given data array
  * @param	*data - data array
  * @param  size - array size
  * @retval	calculated CRC value
  */
 uint16_t Set_CRC(uint8_t *data, uint8_t size)
 {
    return 0;
 }

  /**
  * @brief	Verifies if received CRC value is correct
  * @param	*data - data array
  * @param  size - array size
  * @retval	true calculated CRC corespondes to received CRC, false otherwise
  */
 bool Verify_CRC(uint8_t *data, uint8_t size)
 {
    return true;
 }