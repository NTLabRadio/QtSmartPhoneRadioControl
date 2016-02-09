#include "slipinterface.h"

SLIPInterface::SLIPInterface()
{

}


//---------------------------------------------------------------------------------------------------
// Функция поиска SLIP пакета в массиве данных
//
// Функция ищет SLIP-пакет, начиная с первого символа входного массива, который
// начинается и заканчивается специальным символом FEND. В пакете также 2-байтные
// ESC-последовательности заменяются на соответствующие символы
//
// Параметры:
//  pData - указатель на массив входных данных;
//  nDataSize - размер входного массива, в байтах;
//  pPackData - указатель на данные найденного пакета;
//  nPackSize - размер пакета, в байтах;
//  nPackEndPosInData - позиция последнего символа найденного пакета
//
// Возвращаемое значение:
//  0 - пакет не найден в данных;
//  1 - найдено начало пакета, но не найден конец;
//  2 - пакет найден и успешно обработан;
//  0xFFFF - ошибка в формате найденного пакета (найден символ FESC, не входящий
//  в ESC-последовательность)
//
// ВНИМАНИЕ! Память для pPayloadPackData должна быть выделена предварительно
//---------------------------------------------------------------------------------------------------
uint8_t SLIPInterface::FindPackInData(uint8_t* pData, uint16_t nDataSize, uint8_t* pPayloadPackData, uint16_t nPayloadPackSize, uint16_t& nPosEndOfPack)
{
    uint16_t cntBytes = nDataSize;
    enFindPackState stateFindAutom = PACK_NO_FOUND;
    nPayloadPackSize = 0;

    while(cntBytes--)
    {
        switch(*pData++)
        {
            case FEND:
                if(stateFindAutom==PACK_NO_FOUND)
                {
                    stateFindAutom = PACK_BEGIN_FOUND;
                }
                else if(stateFindAutom==PACK_BEGIN_FOUND)
                {
                    nPosEndOfPack  = nDataSize - cntBytes;      //определяем позицию конца пакета, чтобы вернуть ее из функции
                    stateFindAutom = PACK_FOUND;
                    cntBytes = 0;   //пакет найден и обработан - можно выходить из цикла и успешно завершать выполнение функции
                    break;
                }
                break;
            case FESC:
                if(stateFindAutom==PACK_BEGIN_FOUND)    //обрабатываем символ FESC только, если найдено начало пакета и идет обработка его данных
                {
                    //Символ FESC должен быть первым символом 2-байтной ESC-последовательности
                    //Для определения ESC-последовательности принимаем следующий байт
                    cntBytes--;
                    if(!cntBytes)   //Если закончились данные
                    {
                        #ifdef QTDEBUG_SLIP_PRINTOUT_EXCEPTIONS
                        qDebug() << "WARNING! ::FindSLIPPackInData() В принятых данных найдено начало, но не найдено конца SLIP пакета";
                        #endif
                        return(stateFindAutom); //Значит, все плохо, пакет не обработан до конца. Завершаем выполнение функции
                    }

                    //иначе можем смещать указатель на следующий символ данных
                    pData++;

                    //Делаем замену ESC-последовательности на один байт (FEND или FESC)
                    if(*pData == TFEND)
                    {
                        *pPayloadPackData++ = FEND;
                        nPayloadPackSize++;
                    }
                    else if (*pData == TFEND)
                    {
                        *pPayloadPackData++ = FESC;
                        nPayloadPackSize++;
                    }
                    else
                    {
                        #ifdef QTDEBUG_SLIP_PRINTOUT_EXCEPTIONS
                        qDebug() << "WARNING! ::FindSLIPPackInData() В обрабатываемом SLIP-пакете найден одинокий символ FESC, не входящий в ESC-последовательность";
                        #endif
                        return(RES_FAIL);
                    }
                }
                break;
            default:
                //Если обрабатываем пакет и принятый символ - не FEND и FESC, то просто копируем его
                if(stateFindAutom==PACK_BEGIN_FOUND)
                {
                    *pPayloadPackData++ = *pData;
                    nPayloadPackSize++;
                }
                break;
        }   //end of switch
    }   //end of while

    return(stateFindAutom);
}


//---------------------------------------------------------------------------------------------------
// Функция формирования SLIP пакета
//
// Функция добавляет в начало и конец полезных данных символ FEND и выполняет
// операцию байт-стаффинга над данными
//
// Параметры:
//  pPayloadData - указатель на массив полезной нагрузки пакета;
//  nPayloadSize - размер полезной нагрузки пакета, в байтах;
//  pPackData - указатель на данные сформированного SLIP-пакета;
//  nPackSize - размер сформированного SLIP-пакета, в байтах;
//
// Возвращаемое значение:
//  0 - пакет успешно сформирован;
//  0xFFFF - при формировании пакета произошла ошибка, возможные ошибки:
//      - один из указателей, переданных в функцию- нулевой;
//      - указанный максимальный размер пакета - меньше минимально допустимого (2);
//      - при формировании превышен максимально допустимый размер пакета
//
// ВНИМАНИЕ! Память для pPackData должна быть выделена предварительно
//---------------------------------------------------------------------------------------------------
uint8_t SLIPInterface::FormPack(uint8_t* pPayloadData, uint16_t nPayloadSize, uint8_t* pPackData, uint16_t& nPackSize, uint16_t nMaxPackSize)
{
    nPackSize = 0;

    //Проверка указателей, переданных в функцию
    if(!pPayloadData || !pPackData)
        return(RES_FAIL);

    //Пакет должен состоять не менее чем из 2х символов FEND
    if(nMaxPackSize<2)
        return(RES_FAIL);

    //Первый символ пакета - FEND
    *pPackData++ = FEND;
    nPackSize++;

    while(nPayloadSize--)
    {
        switch(*pPayloadData)
        {
            case FEND:
            case FESC:
                //В пакет необходимо записать не менее 3 символов: ESC-последовательность + символ конца пакета (FEND)
                //Проверяем, не превысит ли размер допустиый
                if(nPackSize>nMaxPackSize-3)
                {
                    #ifdef QTDEBUG_SLIP_PRINTOUT_EXCEPTIONS
                    qDebug() << "WARNING! ::FormSLIPPack() Размер формируемого SLIP пакета превысил допустимый";
                    #endif
                    return(RES_FAIL);
                }

                //В пакет записываем ESC-последовательность
                nPackSize+=2;
                *pPackData++ = FESC;
                if(*pPayloadData==FEND)
                    *pPackData++ = TFEND;
                else
                    *pPackData++ = TFESC;

                pPayloadData++;
                break;
            default:
                //В пакет необходимо записать не менее 2 символов: символ данных + символ конца пакета (FEND)
                //Проверяем, не превысит ли размер допустиый
                if(nPackSize>nMaxPackSize-2)
                {
                    #ifdef QTDEBUG_SLIP_PRINTOUT_EXCEPTIONS
                    qDebug() << "WARNING! ::FormSLIPPack() Размер формируемого SLIP пакета превысил допустимый";
                    #endif
                    return(RES_FAIL);
                }

                //В пакет непосредственно записываем символ полезной нагрузки
                nPackSize++;
                *pPackData++ = *pPayloadData;

                pPayloadData++;
                break;
        }   //end of switch
    }   //end of while

    //NO: При добавлении последнего символа в пакет максимально допустимый размер пакета точно не будет превышен,
    //проверки были проведены ранее

    //Последний символ пакета - FEND
    *pPackData++ = FEND;
    nPackSize++;

    return(RES_SUCCESS);
}
