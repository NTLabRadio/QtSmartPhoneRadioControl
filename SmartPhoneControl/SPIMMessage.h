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
#include <string.h>

class SPIMMessage
{
public:
    SPIMMessage();
    SPIMMessage(const uint8_t* pMsgData, uint16_t msgSize);
    ~SPIMMessage();

    uint8_t* Data;
    uint8_t Size;

    uint8_t setHeader(uint8_t bodySize, uint8_t address, uint8_t noMsg, uint8_t IDcmd);
    uint8_t setBody(uint8_t* pBodyData, uint8_t bodySize);
    uint8_t setCRC();
    uint8_t setMsg(uint8_t* pMsgData, uint8_t msgSize);

    uint8_t getHeader(uint8_t* pHeaderData);
    uint8_t getBody(uint8_t* pBodyData);
    uint8_t getCRC();

    uint8_t checkCRC();

    uint8_t getAddress();
    uint8_t getNoMsg();
    uint8_t getSizeBody();
    uint8_t getIDCmd();

    enum SPIMcmd
    {
        SPIM_NOP											=0x00,
        SPIM_NOP_BACK								=0x01,
        SPIM_SET_MODE								=0x02,
        SPIM_SET_MODE_BACK					=0x03,
        SPIM_SEND_DATA_FRAME					=0x04,
        SPIM_SEND_DATA_FRAME_BACK		=0x05,
        SPIM_TAKE_DATA_FRAME					=0x06,
        SPIM_TAKE_DATA_FRAME_BACK		=0x07,
        SPIM_REQ_CURRENT_PARAM 			=0x08,
        SPIM_REQ_CURRENT_PARAM_BACK 	=0x09,
        SPIM_SOFT_VER								=0x0A,
        SPIM_SOFT_VER_BACK						=0x0B
    };


private:

    //Максимальный размер тела сообщения, байт
    #define MAX_SIZE_OF_BODY     (128)
    //Размер заголовка, байт
    #define SIZE_OF_HEADER          (3)
    //Размер поля CRC, байт
    #define SIZE_OF_CRC                (1)

    //Максимальный размер всего сообщения, байт
    #define MAX_SIZE_OF_MSG     (SIZE_OF_HEADER + MAX_SIZE_OF_BODY + SIZE_OF_CRC)
    //Минимальный размер всего сообщения, байт
    #define MIN_SIZE_OF_MSG     (SIZE_OF_HEADER + SIZE_OF_CRC)

    struct structSPIMMsgHeader {
        uint8_t bodySize;               // размер тела, байт
        uint8_t adress          :4;     // адресат сообщения
        uint8_t noMsg          :4;     // счетчик сообщений (по модулю 4)
        uint8_t IDCmd;                  // идентификатор команды
    };

    uint8_t SPIMmsgData[MAX_SIZE_OF_MSG];
    uint8_t* SPIMHeaderData;
    uint8_t* SPIMbodyData;
    uint8_t SPIMbodySize;
    uint8_t* SPIMCRC;

    uint8_t CRC_Calc(uint8_t* pData, uint8_t sizeData);
};

#endif // SPIMMESSAGE_H
