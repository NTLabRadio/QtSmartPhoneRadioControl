#include "SPIMMessage.h"


SPIMMessage::SPIMMessage()
{
    SPIMHeaderData = SPIMmsgData;
    SPIMbodyData = SPIMmsgData + SIZE_OF_HEADER;

    //Инициализируем нулями всю память, отведенную под сообщение, в т.ч. заголовок и CRC
    memset(SPIMmsgData,0,MAX_SIZE_OF_MSG);

    //По умолчанию создаем сообщение с телом нулевой длины
    Size = MIN_SIZE_OF_MSG;
    SPIMbodySize = 0;

    //Указатель на поле CRC устанавливаем сразу за заголовком, поскольку тело имеет нулевую длину
    SPIMCRC = SPIMmsgData + SIZE_OF_HEADER;

    Data = SPIMmsgData;
}


SPIMMessage::SPIMMessage(const uint8_t* pMsgData, uint16_t msgSize)
{
    if(!pMsgData)
        return;

    if((msgSize < MIN_SIZE_OF_MSG) || (msgSize > MAX_SIZE_OF_MSG))
        return;

    SPIMHeaderData = SPIMmsgData;
    SPIMbodyData = SPIMmsgData + SIZE_OF_HEADER;

    //Инициализируем нулями всю память, отведенную под сообщение
    memset(SPIMmsgData,0,MAX_SIZE_OF_MSG);

    //Копируем данные в тело сообщения
    memcpy((void*)SPIMmsgData,(void*)pMsgData,msgSize);

    Size = msgSize;
    SPIMbodySize = Size - (SIZE_OF_HEADER + SIZE_OF_CRC);

    //Устанавливаем указатель на поле CRC
    SPIMCRC = SPIMmsgData + SIZE_OF_HEADER + SPIMbodySize;

    Data = SPIMmsgData;

    return;
}


SPIMMessage::~SPIMMessage()
{

}


/**
    * @brief  Составление заголовка сообщения и копирование его
    *           непосредственно в сообщение
    *
    * @param  bodySize - длина тела, байт
    * @param  address - адресат сообщения
    * @param  noMsg - порядковый номер сообщения (по модулю 4)
    * @param  IDcmd - идентификатор команды
    *
    * @note   Функция составляет заголовок из отдельных полей,
    *               переданных как входные параметры, и копирует его
    *               в сообщение
    *
    * @retval Результат выполнения функции:
    *               0 - заголовок успешно составлен и скопирован в
    *               сообщение;
    *               не 0 - заголовок не может быть составлен
    *               (некорректные входные данные)
  */
uint8_t SPIMMessage::setHeader(uint8_t bodySize, uint8_t address, uint8_t noMsg, uint8_t IDcmd)
{
    structSPIMMsgHeader SPIMMsgHeader;

    if(bodySize <= MAX_SIZE_OF_BODY)
        SPIMMsgHeader.bodySize = bodySize;
    else
        return 1;

    SPIMMsgHeader.adress = address;

    SPIMMsgHeader.noMsg = noMsg;

    SPIMMsgHeader.IDCmd = IDcmd;

    memcpy((void*)SPIMHeaderData,(void*)&SPIMMsgHeader,SIZE_OF_HEADER);

    return 0;
}

/**
    * @brief  Копирование тела сообщения непосредственно в сообщение
    *
    * @param  pBodyData - указатель на данные тела;
    * @param  bodySize - длина тела, байт
    *
    * @retval Результат выполнения функции:
    *               0 - тело успешно скопировано в сообщение;
    *               не 0 - тело не может быть скопировано в сообщение:
    *               передан нулевой указатель или размер входных данных
    *               превышает максимально допустимый
  */
uint8_t SPIMMessage::setBody(uint8_t* pBodyData, uint8_t bodySize)
{
    if((!pBodyData) && bodySize)
        return(1);

    if(bodySize > MAX_SIZE_OF_BODY)
        return(1);

    memcpy((void*)SPIMbodyData,(void*)pBodyData,bodySize);

    SPIMbodySize = bodySize;
    Size = SPIMbodySize + SIZE_OF_HEADER + SIZE_OF_CRC;

    //Устанавливаем указатель на поле CRC
    SPIMCRC = SPIMmsgData + SIZE_OF_HEADER + SPIMbodySize;

    return(0);
}


/**
    * @brief  Скопировать данные сообщения в SPIMMessage
    *
    * @param  pMsgData - указатель на данные сообщения;
    * @param  msgSize - длина сообщения, байт
    *
    * @retval Результат выполнения функции:
    *               0 - данные успешно скопированы;
    *               не 0 - данные не могут быть скопированы в сообщение:
    *               передан нулевой указатель или некорректный размер
    *               входных данных
  */
uint8_t SPIMMessage::setMsg(uint8_t* pMsgData, uint8_t msgSize)
{
    if(!pMsgData)
        return 1;

    if((msgSize < MIN_SIZE_OF_MSG) || (msgSize > MAX_SIZE_OF_MSG))
        return 1;

    memcpy((void*)SPIMmsgData,(void*)pMsgData,msgSize);

    Size = msgSize;
    SPIMbodySize = Size - (SIZE_OF_HEADER + SIZE_OF_CRC);

    //Устанавливаем указатель на поле CRC
    SPIMCRC = SPIMmsgData + SIZE_OF_HEADER + SPIMbodySize;

    return 0;
}

/**
    * @brief  Записать контрольную сумму в SPIMMessage, расчитанную
    *             на основе данных сообщения
    *
  */
uint8_t SPIMMessage::setCRC()
{
    uint8_t nCRC = CRC_Calc(SPIMmsgData, Size-1);

    *SPIMCRC = nCRC;

    return(nCRC);
}

/**
    * @brief  Функция возвращает данные заголовка сообщения
    *
    * @param pHeaderData - указатель, по которому должны
    *               быть переданы данные заголовка
    *
    * @note Функция не выделяет память для возвращаемых
    *           данных; она должна быть выделена предварительно
    *           в достаточном объеме вызывающей функцией
    *
    * @retval Возможные возвращаемые значения:
    *               не 255 - размер данных заголовка;
    *               255 - ошибка выполнения функции (передан
    *               нулевой указатель)
  */
uint8_t SPIMMessage::getHeader(uint8_t* pHeaderData)
{
    if(!pHeaderData)
        return(0xFF);

    memcpy(pHeaderData,SPIMHeaderData,SIZE_OF_HEADER);

    return(SIZE_OF_HEADER);
}

/**
    * @brief  Функция возвращает данные тела сообщения
    *
    * @param pHeaderData - указатель, по которому должны
    *               быть переданы данные тела
    *
    * @note Функция не выделяет память для возвращаемых
    *           данных; она должна быть выделена предварительно
    *           в достаточном объеме вызывающей функцией
    *
    * @retval Возможные возвращаемые значения:
    *               0-128 - размер данных тела;
    *               255 - ошибка выполнения функции (передан
    *               нулевой указатель)
  */
uint8_t SPIMMessage::getBody(uint8_t* pBodyData)
{
    if(!pBodyData)
        return(0xFF);

    if(!SPIMbodySize)
        return(0);

    memcpy(pBodyData,SPIMbodyData,SPIMbodySize);

    return(SPIMbodySize);
}

/**
    * @brief  Функция возвращает значение поля CRC
    *
    * @retval Значение поля CRC
  */
uint8_t SPIMMessage::getCRC()
{
    return(*SPIMCRC);
}

/**
    * @brief  Функция проверки корректности контрольной суммы,
    *           записанной в соответствующем поле SPIMMessage
    *
    * @retval   0 - значение поля некорректно (не совпадает с
    *               расчитанной контрольной суммой),
    *               1 - значение поля корректно
  */
uint8_t SPIMMessage::checkCRC()
{
    if(*SPIMCRC == CRC_Calc(SPIMmsgData, Size-1))
        return(1);
    else
        return(0);
}

/**
    * @brief  Функция читает значение адреса из соответствующего
    *           поля заголовка сообщения SPIMMessage
    *
    * @retval Значение поля адреса
  */
uint8_t SPIMMessage::getAddress()
{
    structSPIMMsgHeader *SPIMMsgHeader;

    SPIMMsgHeader = (structSPIMMsgHeader*)SPIMHeaderData;

    return(SPIMMsgHeader->adress);
}

/**
    * @brief  Функция читает значение номера сообщения
    *           из соответствующего поля заголовка сообщения
    *           SPIMMessage
    *
    * @retval Значение поля номера сообщения
  */
uint8_t SPIMMessage::getNoMsg()
{
    structSPIMMsgHeader *SPIMMsgHeader;

    SPIMMsgHeader = (structSPIMMsgHeader*)SPIMHeaderData;

    return(SPIMMsgHeader->noMsg);
}

/**
    * @brief  Функция читает значение длины тела из
    *           соответствующего поля заголовка сообщения
    *           SPIMMessage
    *
    * @retval Значение поля длины тела
  */
uint8_t SPIMMessage::getSizeBody()
{
    structSPIMMsgHeader *SPIMMsgHeader;

    SPIMMsgHeader = (structSPIMMsgHeader*)SPIMHeaderData;

    return(SPIMMsgHeader->bodySize);
}

/**
    * @brief  Функция читает значение идентификатора команды
    *           из соответствующего поля заголовка сообщения
    *           SPIMMessage
    *
    * @retval Значение поля идентификатора команды
  */
uint8_t SPIMMessage::getIDCmd()
{
    structSPIMMsgHeader *SPIMMsgHeader;

    SPIMMsgHeader = (structSPIMMsgHeader*)SPIMHeaderData;

    return(SPIMMsgHeader->IDCmd);
}

/**
    * @brief  Функция вычисления 8-битного CRC на основе XOR
    *
    * @retval Значение CRC
  */
uint8_t SPIMMessage::CRC_Calc(uint8_t* pData, uint8_t sizeData)
{
    uint8_t nCRC = 0;

    while(sizeData--)
        nCRC ^= *pData++;

    return(nCRC);
}
