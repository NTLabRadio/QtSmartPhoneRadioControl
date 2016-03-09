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

//Максимальный размер полезной нагрузки SLIP-пакетов
#define MAX_SIZE_OF_SLIP_PACK_PAYLOAD	(128+4)		// В соответствие с межмодульным протоколом SPIM (Smart Phone InterModule)
                                                                                        // - максимальная длина полезных данных - 128 байт;
                                                                                        // - размер служебных данных в пакете (заголовок, CRC) - 4 байта

class SLIPInterface
{
public:
  SLIPInterface();

    static uint8_t FindPackInData(uint8_t* pData, uint16_t nDataSize, uint8_t* pPayloadPackData, uint16_t& nPayloadPackSize, uint16_t& nPosEndOfPack);
    static uint8_t FormPack(uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t* pPackData, uint16_t& nPackSize, uint16_t nMaxPackSize = USHRT_MAX);

	uint8_t CheckForSLIPData(uint8_t nByte, uint8_t* pPayloadPackData, uint16_t &nPayloadPackSize, uint8_t &nCheckState);

    void WaitForPack();

    //Максимальный размер SLIP-пакета
    //Протокол SLIP операцией байт-стафинга может увеличивать размер данных в 2 раза + 2 символа FEND
    static const uint16_t  MAX_SIZE_OF_PACK = (2*MAX_SIZE_OF_SLIP_PACK_PAYLOAD + 2);

private:
	//Специальные символы протокола SLIP
    static const uint8_t FEND = (0xC0);      /* "Frame End" symbol, i.e. begin/end of packet */
    static const uint8_t FESC = (0xDB);      /* "Frame Escape" symbol */
    static const uint8_t TFEND = (0xDC);    /* "Transposed Frame End" */
    static const uint8_t TFESC = (0xDD);    /* "Transposed Frame Escape" */

	//Возможные результаты выполнения функций
    static const uint8_t RESULT_SUCCESS = (0);
    static const uint8_t RESULT_FAIL = (UCHAR_MAX);

	//Возможные состояния автомата поиска SLIP-пакета в массиве данных (используется FindPackInData())
  enum en_FindPackStates
  {
      PACK_NO_FOUND,         //пакет не найден
      PACK_BEGIN_FOUND,    //найден старт-байт пакета
      PACK_FOUND                //найден весь пакет
  };
	
	//Возможные состояния интерфейса SLIPс
    enum en_InterfaceStates
	{
		STATE_IDLE,
		STATE_WAIT_FOR_BEGIN_OF_PACK,
		STATE_WAIT_FOR_END_OF_PACK
	};

	//Состояние интерфейса SLIP, анализирующего поток данных побайтно
    en_InterfaceStates InterfaceState;
	
    uint8_t BufForSLIPData[MAX_SIZE_OF_PACK];
	
	uint8_t* pPackData;	//Данные пакета, накопленные интерфейсом SLIP
	uint16_t nPackSize;	//Размер данных пакета, накопленных интерфейсом SLIP
};

#endif // SLIPINTERFACE_H
