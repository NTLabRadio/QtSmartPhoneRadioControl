/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** Этот файл описывает класс SPIMMessage, включающий в себя набор основных
** методов и свойств для реализации логического интерфейса SPIM (Smart Phone InterModule)
** Класс позволяет формировать и обрабатывать сообщения формата, определенного
** интерфейсом SPIM
**
****************************************************************************/

#ifndef SPIMMESSAGE_H
#define SPIMMESSAGE_H

#include <stdint.h>

class SPIMMessage
{
public:
    SPIMMessage();
    SPIMMessage(const uint8_t* baMsg, uint16_t nMsgSize);
    ~SPIMMessage();

    uint8_t* msgData;
    uint16_t msgSize;
    uint16_t bodySize;

    uint8_t setHeader(uint16_t typeMsg, uint16_t typeCmd, uint16_t parID, uint16_t cntMsg, uint16_t lenBody);
    uint8_t setBody(uint8_t* baMsg, uint16_t nMsgSize);
    uint8_t setCRC();
    uint8_t setMes(uint8_t* baMsg, uint16_t nMsgSize);

    uint8_t* getHeader();
    uint8_t* getBody();
    uint8_t* getCRC();

    uint8_t CheckCRC();

    uint8_t getParID();
    uint8_t getTypeCmd();
    uint8_t getSizeBody();

};

#endif // SPIMMESSAGE_H
