/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** Этот файл описывает класс SPIMMessage, включающий в себя набор основных
** методов и свойств для реализации логического интерфейса SPIM (Smart Phone 
** InterModule). Класс позволяет формировать и обрабатывать сообщения формата, 
** определенного интерфейсом SPIM
**
****************************************************************************/

#ifndef SPIMMESSAGE_H
#define SPIMMESSAGE_H

#include <stdint.h>
#include <string.h>

enum en_SPIMcmds
{
	SPIM_CMD_NOP										=0x00,
	SPIM_CMD_NOP_BACK								=0x01,
	SPIM_CMD_SET_MODE								=0x02,
	SPIM_CMD_SET_MODE_BACK					=0x03,
	SPIM_CMD_SEND_DATA_FRAME				=0x04,
	SPIM_CMD_SEND_DATA_FRAME_BACK		=0x05,
    SPIM_CMD_TAKE_DATA_FRAME                 =0x06,
	SPIM_CMD_TAKE_DATA_FRAME_BACK		=0x07,
	SPIM_CMD_REQ_CURRENT_PARAM 			=0x08,
	SPIM_CMD_REQ_CURRENT_PARAM_BACK =0x09,
	SPIM_CMD_SOFT_VER								=0x0A,
	SPIM_CMD_SOFT_VER_BACK					=0x0B
};



class SPIMMessage
{
public:
	
	SPIMMessage();
	SPIMMessage(const uint8_t* pMsgData, uint16_t msgSize);
	~SPIMMessage();

	uint8_t* Data;
	uint8_t Size;

	uint8_t* Body;

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

    //Максимальный размер тела сообщения, байт
    static const uint8_t MAX_SIZE_OF_BODY = 128;
	
	uint8_t IDBackCmd(uint8_t IDCmd);
	
	static void ParseOpModeCode(uint8_t opModeCode, uint8_t& RadioChanType, uint8_t& SignalPower, uint8_t& ARMPowerMode);
	static void ParseAudioCode(uint8_t audioCode, uint8_t& AudioOutLevel, uint8_t& AudioInLevel);
	
	class CmdReqParam
	{
		public:
			static uint8_t OpModeCode(uint8_t RadioChanType, uint8_t SignalPower, uint8_t ARMPowerMode);
			static uint8_t AudioCode(uint8_t AudioOutLevel, uint8_t AudioInLevel);
			
			void SetPointerToMessage(SPIMMessage* mes);
		
			uint8_t MaskReqParam();
		
			uint8_t isOpModeReq();
			uint8_t isAudioReq();
			uint8_t isRxFreqReq();
			uint8_t isTxFreqReq();
			uint8_t isRSSIReq();
			uint8_t isChanStateReq();
		
            //Маски запрашиваемых параметров
            static const uint8_t OPMODE_MASK_IN_REQ = (1<<1);
            static const uint8_t AUDIO_MASK_IN_REQ = (1<<2);
            static const uint8_t TXFREQ_MASK_IN_REQ = (1<<3);
            static const uint8_t RXFREQ_MASK_IN_REQ = (1<<4);
            static const uint8_t RSSI_MASK_IN_REQ = (1<<5);
            static const uint8_t CHANSTATE_MASK_IN_REQ = (1<<5);

		private:
			SPIMMessage* objSPIMMessage;			
	} cmdReqParam;	

    enum en_SPIMaddrs
    {
        SPIM_ADDR_STM32								=0x1,		//контроллер STM32 целевого устройства (радимодул¤)
        SPIM_ADDR_EXTDEV								=0x2		//внешнее управл¤ющее устройство (процессор NT1004, ѕ  или др.)
    };

    enum en_SPIMReqTypes
    {
        SPIM_REQTYPE_SINGLE							=0,			//одиночный синхронный запрос - запрос, ответ на который должен быть выслан ведомым
                                                                                    //устройством однократно в момент получени¤ запроса
        SPIM_REQTYPE_ASYNC							=1			//асинхронный запрос - запрос, в ответ на который параметр (например, RSSI) высылаетс¤
                                                                                    //ведомым устройством в произвольный момент времени, при изменении его значени¤
    };

private:

	//Размер заголовка, байт
	static const uint8_t SIZE_OF_HEADER = 3;
	//Размер поля CRC, байт
	static const uint8_t SIZE_OF_CRC = 1;

	//Максимальный размер всего сообщения, байт
	static const uint8_t MAX_SIZE_OF_MSG = (SIZE_OF_HEADER + MAX_SIZE_OF_BODY + SIZE_OF_CRC);
	//Минимальный размер всего сообщения, байт
	static const uint8_t MIN_SIZE_OF_MSG = (SIZE_OF_HEADER + SIZE_OF_CRC);

	struct structSPIMMsgHeader {
		uint8_t bodySize;               // размер тела, байт
		uint8_t adress          :4;     // адресат сообщения
		uint8_t noMsg          	:4;     // счетчик сообщений (по модулю 4)
		uint8_t IDCmd;                  // идентификатор команды
	};
	
	uint8_t SPIMmsgData[MAX_SIZE_OF_MSG];
	uint8_t* SPIMHeaderData;
	uint8_t* SPIMbodyData;
	uint8_t SPIMbodySize;
	uint8_t* SPIMCRC;

	uint8_t CRC_Calc(uint8_t* pData, uint8_t sizeData);

	//Коды параметров, используемые в командах SET_MODE и REQ_CURRENT_PARAM_BACK
	
	//--------- Код рабочего режима ---------------
	//Тип радиоканала
    static const uint8_t SHIFT_RADIOCHANTYPE_IN_OPMODECODE = (0);
    static const uint8_t MASK_RADIOCHANTYPE_IN_OPMODECODE = (3);
	//Мощность сигнала передатчика
    static const uint8_t SHIFT_SIGNALPOWER_IN_OPMODECODE = (3);
    static const uint8_t MASK_SIGNALPOWER_IN_OPMODECODE = (1);
	//Режим энергосбережения ARM
    static const uint8_t SHIFT_ARMPOWERMODE_IN_OPMODECODE = (4);
    static const uint8_t MASK_ARMPOWERMODE_IN_OPMODECODE = (1);

	//------ Код настроек аудиопараметров ---------
	//Усиление звукового выхода
    static const uint8_t SHIFT_OUTLEVEL_IN_AUDIOCODE = (0);
    static const uint8_t MASK_OUTLEVEL_IN_AUDIOCODE = (7);
	//Усиление звукового входа
    static const uint8_t SHIFT_INLEVEL_IN_AUDIOCODE = (3);
    static const uint8_t MASK_INLEVEL_IN_AUDIOCODE = (7);

	
};

#endif // SPIMMESSAGE_H
