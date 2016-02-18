#include "slipinterface.h"

SLIPInterface::SLIPInterface()
{
	InterfaceState = STATE_IDLE;
	
	nPackSize = 0;
	pPackData = &BufForSLIPData[0];
}

/**
    * @brief  Инициализация механизма поиска SLIP-пакета
	* @param  нет
	*
    * @note   Функция инициализирует параметры механизма
	*					поиска SLIP-пакета: устанавливает начальное
	*					состояние поиска, сбрасывает счетчик накопленных
	*					данных, устанавливает указатель данных на начало
	*					предвыделенного фрагмента памяти
	*
	* @retval void
  */
void SLIPInterface::WaitForPack()
{
	InterfaceState = STATE_WAIT_FOR_BEGIN_OF_PACK;
	
	nPackSize = 0;
	pPackData = &BufForSLIPData[0];	
}


/**
    * @brief  Функция поиска SLIP-пакета в потоковых данных
	*					и извлечения из найденного пакета	полезных данных
	*
	* @param  nStreamDataByte - входной символ потоковых данных;
    * @param	pPayloadPackData - указатель, по которому должны быть
	*					переданы полезные данные, извлеченные из найденного 
	*					SLIP-пакета
	* @param	nPayloadPackSize - размер полезных данных, байт;
	* @param	nCheckState - текущее состояние автомата поиска 
	*					данных SLIP-протокола:
	*						0 - idle-состояние;
	*						1 - поиск начала пакета;
	*						2 - поиск конца пакета;
	*						255 - ошибка в данных (превышен максимально 
	*						допустимый размер пакета или др.)
	*
    * @note   Функция побайтно анализирует потоковые данные на
	*					предмет нахождения в них SLIP-данных, которые 
	*					начинаются и заканчиваются специальными символами.
	*					В случае нахождения начала SLIP-пакета, функция
	*					автоматически начинает накапливать полезные данные.
	*					В процессе приема SLIP-данных функция выполняет
	*					операцию байт-стаффинга над ними и по окончанию
	*					приема возвращает полезные данные пакета вызывающей 
	*					функции
	*
	* @retval Результат поиска:
	*					0 - SLIP-пакет не принят;
	*					1 - SLIP-пакет принят полностью;
  */
uint8_t SLIPInterface::CheckForSLIPData(uint8_t nStreamDataByte, uint8_t* pPayloadPackData, uint16_t &nPayloadPackSize, uint8_t &nCheckState)
{
	uint16_t nPosEndOfPack;
	
	switch(InterfaceState)
	{
		case STATE_WAIT_FOR_BEGIN_OF_PACK:
			if(nStreamDataByte==FEND)	//Если принят символ начала пакета
			{
				//Изменяем состояние интерфейса на ожидание конца пакета
				InterfaceState = STATE_WAIT_FOR_END_OF_PACK;

				//Начинаем накопление данных
				nPackSize = 0;
				pPackData = &BufForSLIPData[0];
				
				*pPackData++ = nStreamDataByte;
				nPackSize++;
			}
			break;
		case STATE_WAIT_FOR_END_OF_PACK:
			//Проверка, не превысил ли счетчик накапливаемых данных допустимый размер пакета	
			if(nPackSize>=SIZE_BUF_FOR_SLIP_DATA-1)
			{
				nCheckState = 0xFF;
				return(0);
			}
		
			//Накапливаем данные
			*pPackData++ = nStreamDataByte;
			nPackSize++;
		
			if(nStreamDataByte==FEND)	//Если принят символ конца пакета
			{
				//Изменяем состоние интерфейса на бездействие
				InterfaceState = STATE_IDLE;				
				
				//Обрабатываем накопленные данные, выполняя операцию байт-стаффинга
				FindPackInData(&BufForSLIPData[0], nPackSize, pPayloadPackData, nPayloadPackSize, nPosEndOfPack);				
				
				nCheckState = InterfaceState;
				return(1);
			}				
			break;
		case STATE_IDLE:
			break;
	}
	
	nCheckState = InterfaceState;
	return(0);
}


/**
    * @brief  Функция поиска SLIP пакета в массиве данных
	*
	* @param  pData - указатель на массив входных данных;
	* @param  nDataSize - размер входного массива, в байтах;
	* @param  pPackData - указатель на данные найденного пакета;
	* @param  nPackSize - размер пакета, в байтах;
	* @param	nPackEndPosInData - позиция последнего символа найденного пакета
	*
    * @note   Функция ищет SLIP-пакет, начиная с первого символа входного массива,
	*					который начинается и заканчивается специальным символом FEND. 
	*					В пакете также 2-байтные ESC-последовательности заменяются на 
	*					соответствующие символы
	* @note		ВНИМАНИЕ! Память для pPayloadPackData должна быть выделена 
	*					предварительно
	*
	* @retval Результат поиска:
	*   			0 - пакет не найден в данных;
	*   			1 - найдено начало пакета, но не найден конец;
	*   			2 - пакет найден и успешно обработан;
	*   			0xFFFF - ошибка в формате найденного пакета (найден символ FESC, не входящий
	*   			в ESC-последовательность)
  */
uint8_t SLIPInterface::FindPackInData(uint8_t* pData, uint16_t nDataSize, uint8_t* pPayloadPackData, uint16_t& nPayloadPackSize, uint16_t& nPosEndOfPack)
{
    uint16_t cntBytes = nDataSize;
    enFindPackState stateFindAutom = PACK_NO_FOUND;
    nPayloadPackSize = 0;

    while(cntBytes--)
    {
        switch(*pData)
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
                    else if (*pData == TFESC)
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
				pData++;
    }   //end of while

    return(stateFindAutom);
}


/**
    * @brief  Функция формирования SLIP пакета
	*
	* @param  pPayloadData - указатель на массив полезной нагрузки пакета;
	* @param  nPayloadSize - размер полезной нагрузки пакета, в байтах;
	* @param  pPackData - указатель на данные сформированного SLIP-пакета;
	* @param  nPackSize - размер сформированного SLIP-пакета, в байтах;
	*
    * @note   Функция добавляет в начало и конец полезных данных символ FEND
	*					и выполняет
	* 				операцию байт-стаффинга над данными
	* @note		ВНИМАНИЕ! Память для pPackData должна быть выделена предварительно
	*
	* @retval Результат выполнения функции:
	*					0 - пакет успешно сформирован;
	*  				0xFFFF - при формировании пакета произошла ошибка, возможные ошибки:
	*						- один из указателей, переданных в функцию- нулевой;
	*			      - указанный максимальный размер пакета - меньше минимально допустимого (2);
	*  			    - при формировании превышен максимально допустимый размер пакета
  */
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
