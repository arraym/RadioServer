/* Includes ------------------------------------------------------------------*/
#include "messages.h"
#include <string.h>
#include "CRC16.h"

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
   crc = (uint16_t)(data[2] << 8) | data[3];
   memcpy(commandArray, data, COMMAND_SIZE);
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
   cmd = c;
   destAddress = address;
   commandArray[0] = c;
   commandArray[1] = address;
   crc = Set_CRC((uint8_t*)&commandArray, COMMAND_SIZE);
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
   crc = (uint16_t)(data[5] << 8) | data[6];
   memcpy(responseArray, data, RESPONSE_SIZE);
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
   rsp = r;
   payload = load;
   responseArray[0] = r;
   memcpy((uint8_t*)&responseArray[1], (uint8_t*)load, PAYLOAD_SIZE);
   crc = Set_CRC((uint8_t*)&responseArray, RESPONSE_SIZE);
 }

 /**
  * @brief	Calculates CRC for a given data array
  * @param	*data - data array
  * @param  size - array size
  * @retval	calculated CRC value
  */
 uint16_t Set_CRC(uint8_t *data, uint8_t size)
 {
   uint16_t crcValue = 0;
   CRC16 crc;

   crc.setPolynome(0x1021);
   crc.add(data, (uint16_t)(size - CRC_SIZE));
   crcValue = crc.getCRC();

   data[2] = crcValue >> 8;
   data[3] = crcValue & 0xFF;

   return crcValue;
 }

  /**
  * @brief	Verifies if received CRC value is correct
  * @param	*data - data array
  * @param  size - array size
  * @retval	true calculated CRC corespondes to received CRC, false otherwise
  */
 bool Verify_CRC(uint8_t *data, uint8_t size)
 {
   uint16_t crcValue = 0;
   uint16_t receivedCrc = (uint16_t)(data[size - 2] << 8) | data[size - 1];
   CRC16 crc;

   crc.setPolynome(0x1021);
   crc.add(data, (uint16_t)(size - CRC_SIZE));
   crcValue = crc.getCRC();

   if(crcValue != receivedCrc)
      return false;

   return true;
 }