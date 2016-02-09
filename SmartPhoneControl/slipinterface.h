/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** Этот файл описывает класс SLIPInterface, включающий в себя набор основных
** методов и свойств для реализации интерфейса SLIP согласно стандарту RFC 1055
**
****************************************************************************/

#ifndef SLIPINTERFACE_H
#define SLIPINTERFACE_H

#include <stdint.h>
#include <limits.h>


class SLIPInterface
{
public:
    SLIPInterface();

    uint8_t FindPackInData(uint8_t* pData, uint16_t nDataSize, uint8_t* pPayloadPackData, uint16_t nPayloadPackSize, uint16_t& nPosEndOfPack);
    uint8_t FormPack(uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t* pPackData, uint16_t& nPackSize, uint16_t nMaxPackSize = USHRT_MAX);


private:
    #define FEND        (0xC0)  /* "Frame End" symbol, i.e. begin/end of packet */
    #define FESC        (0xDB)  /* "Frame Escape" symbol */
    #define TFEND      (0xDC)  /* "Transposed Frame End" */
    #define TFESC      (0xDD)  /* "Transposed Frame Escape" */

  enum enFindPackState
  {
      PACK_NO_FOUND,            //пакет не найден
      PACK_BEGIN_FOUND,       //найден старт-байт пакета
      PACK_FOUND                   //найден весь пакет
  };

    #define RES_SUCCESS     (0)
    #define RES_FAIL             (UCHAR_MAX)
};

#endif // SLIPINTERFACE_H
