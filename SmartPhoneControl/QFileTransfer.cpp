#include "QFileTransfer.h"

QFileTransfer::QFileTransfer(QObject *parent) :
    QObject(parent)
{
    fileName = "";
    fileNameReduced = "";
    fileSize = 0;
    sizeOfSendedData = 0;

    rcvFileNameReduced = "";
    sizeOfRcvData = 0;
    expectSizeRcvFile = 0;

    sendFileStatus = FILE_SEND_DENY;
    rcvFileStatus=FILE_RCV_WAITING;
    errorStatus = ERROR_FILE_IS_NOT_CHOSEN;
    transmitterStatus = TRANSMITTER_FREE;
}


QFileTransfer::~QFileTransfer()
{

}


quint16 QFileTransfer::SetFileForSend(QString name)
{
    //Пока не открыли и не прочитали файл, считаем, что файл не выбран
    sendFileStatus = FILE_SEND_DENY;
    errorStatus = ERROR_FILE_IS_NOT_CHOSEN;

    fileName = name;

    fileForSend.setFileName(name);

    fileNameReduced = fileName.right(fileName.size() - fileName.lastIndexOf("/") - 1);

    //Определяем размер файла
    fileSize = fileForSend.size();

    if(fileSize>MAX_SIZE_OF_FILE)
    {
        qDebug() << "WARNING! QFileTransfer::SetFileForSend() Файл не может быть открыт, поскольку слишком большой";
        errorStatus = ERROR_FILE_SIZE_IS_TOO_BIG;
        return(0);
    }

    //Устанавливаем размер переданных данных нулевым, поскольку пока ничего не передали
    sizeOfSendedData = 0;

    if (!fileForSend.open(QIODevice::ReadOnly))    //открываем файл
    {
        qDebug() << "WARNING! QFileTransfer::SetFileForSend() Файл не может быть открыт";
        errorStatus = ERROR_FILE_OPEN;
        return(0);
    }

    //Читаем все данные из файла
    baFileDataRestForSend = fileForSend.readAll();

    //Закрываем файл, он нам больше не нужен
    fileForSend.close();

    //Теперь мы готовы к передаче файла
    sendFileStatus = FILE_IS_READY_FOR_SEND;
    errorStatus = ERROR_NONE;
    transmitterStatus = TRANSMITTER_FREE;

    return(1);
}


quint16 QFileTransfer::ResetFileForSend()
{
    //Пока не открыли и не прочитали файл, считаем, что файл не выбран
    sendFileStatus = FILE_SEND_DENY;
    errorStatus = ERROR_FILE_IS_NOT_CHOSEN;

    //Устанавливаем размер переданных данных нулевым, поскольку пока ничего не передали
    sizeOfSendedData = 0;

    if (!fileForSend.open(QIODevice::ReadOnly))    //открываем файл
    {
        qDebug() << "WARNING! QFileTransfer::SetFileForSend() Файл не может быть открыт";
        errorStatus = ERROR_FILE_OPEN;
        return(0);
    }

    //Читаем все данные из файла
    baFileDataRestForSend = fileForSend.readAll();

    //Закрываем файл, он нам больше не нужен
    fileForSend.close();

    //Теперь мы готовы к передаче файла
    sendFileStatus = FILE_IS_READY_FOR_SEND;
    errorStatus = ERROR_NONE;
    transmitterStatus = TRANSMITTER_FREE;

    return(1);
}


quint16 QFileTransfer::PrepareToRcvNewFile()
{
    rcvFileNameReduced = "";
    sizeOfRcvData = 0;
    expectSizeRcvFile = 0;

    rcvFileStatus=FILE_RCV_WAITING;

    return(1);
}


QString QFileTransfer::GetFileName()
{
    return(fileName);
}

quint32 QFileTransfer::GetFileSize()
{
    return(fileSize);
}

quint32 QFileTransfer::GetSizeOfSendedFileData()
{
    return(sizeOfSendedData);
}

quint32 QFileTransfer::GetSizeOfRcvFile()
{
    return(expectSizeRcvFile);
}

quint32 QFileTransfer::GetSizeOfRcvdFileData()
{
    return(sizeOfRcvData);
}

quint16 QFileTransfer::GetSendFileStatus()
{
    return(sendFileStatus);
}


quint16 QFileTransfer::GetRcvFileStatus()
{
    return(rcvFileStatus);
}


void QFileTransfer::SetTransmitterStatus(quint16 status)
{
    transmitterStatus = (en_transmitterStatuses)status;

    return;
}

quint16 QFileTransfer::GetTransmitterStatus()
{
    return(transmitterStatus);
}

quint16 QFileTransfer::GetErrorStatus()
{
    return(errorStatus);
}

void QFileTransfer::slotSendFileDataPack(QSmartRadioModuleControl* RadioDevice)
{
    if(!RadioDevice->isConnected())
    {
        sendFileStatus = FILE_SEND_DENY;
        errorStatus = ERROR_DEVICE_CONNECTION;
        return;
    }

    if(sendFileStatus==FILE_IS_READY_FOR_SEND)
    {
        SPIMMessage SPIMmsgForSend;
        quint16 nSPIMMsgBodySize = sizeof(RadioDataHeader);
        uint8_t pSPIMMsgBody[nSPIMMsgBodySize];

        RadioDataHeader structRadioDataHeader;
        structRadioDataHeader.radioDataType = RADIO_DATA_TYPE_FILE;
        structRadioDataHeader.fileSize = fileSize;

        memset(structRadioDataHeader.fileName, 0, MAX_SIZE_OF_FILE_NAME);
        QByteArray baFileName=fileNameReduced.toLatin1();
        strcpy((char*)structRadioDataHeader.fileName, baFileName.data());

        memcpy(pSPIMMsgBody, &structRadioDataHeader, sizeof(RadioDataHeader));

        //Составляем заголовок сообщения
        SPIMmsgForSend.setHeader(nSPIMMsgBodySize, SPIMMessage::SPIM_ADDR_STM32, noSPIMmsgs, SPIM_CMD_SEND_DATA_FRAME);
        //TODO Надо где-то инкрементировать noSPIMmsgs, а также обрабатывать ответ от радиомодуля, в т.ч. отсутствие ответа и
        //ответ о том, что приняты данные с неверным CRC

        //Вставляем в сообщение тело
        SPIMmsgForSend.setBody(pSPIMMsgBody,nSPIMMsgBodySize);

        //Расчитываем и вставляем в сообщение контрольную сумму
        SPIMmsgForSend.setCRC();

        //Передаем сообщение на устройство
        if(!SendSPIMMsgToDevice(RadioDevice,SPIMmsgForSend))
            sendFileStatus=FILE_SEND_DENY;

        sendFileStatus=FILE_IS_IN_SEND_PROCESS;

        return;
    }

    if(sendFileStatus==FILE_IS_IN_SEND_PROCESS)
    {
        //Размер данных, которые еще необходимо передать
        quint32 sizeOfRestData = fileSize-sizeOfSendedData;

        if(sizeOfRestData)
        {
            SPIMMessage SPIMmsgForSend;
            quint16 nSPIMMsgBodySize;

            if(sizeOfRestData>SIZE_OF_RADIOPACK_PAYLOAD)
                nSPIMMsgBodySize = SIZE_OF_RADIOPACK_PAYLOAD;
            else
                nSPIMMsgBodySize = (quint16)sizeOfRestData;

            //Составляем заголовок сообщения
            SPIMmsgForSend.setHeader(nSPIMMsgBodySize, SPIMMessage::SPIM_ADDR_STM32, noSPIMmsgs, SPIM_CMD_SEND_DATA_FRAME);
            //TODO Надо где-то инкрементировать noSPIMmsgs, а также обрабатывать ответ от радиомодуля, в т.ч. отсутствие ответа и
            //ответ о том, что приняты данные с неверным CRC

            //Вставляем в сообщение тело
            SPIMmsgForSend.setBody((uint8_t*)baFileDataRestForSend.data(),nSPIMMsgBodySize);

            //Расчитываем и вставляем в сообщение контрольную сумму
            SPIMmsgForSend.setCRC();

            //Данные, которые были вставлены в пакет, можно удалить из общей очереди на передачу
            baFileDataRestForSend.remove(0,nSPIMMsgBodySize);
            sizeOfSendedData+=nSPIMMsgBodySize;

            //Передаем сообщение на устройство
            if(!SendSPIMMsgToDevice(RadioDevice,SPIMmsgForSend))
                sendFileStatus=FILE_SEND_DENY;
        }
        else
            sendFileStatus=FILE_IS_SENDED;
    }
}


void QFileTransfer::slotProcessFileDataPack(SPIMMessage* SPIMmsg)
{
    quint8* pBodyData = SPIMmsg->Body;
    quint16 nSizeBody = SPIMmsg->getSizeBody();

    if(rcvFileStatus==FILE_RCV_WAITING)
    {
        RadioDataHeader structRadioDataHeader;
        memcpy(&structRadioDataHeader,  pBodyData, sizeof(structRadioDataHeader));

        if(structRadioDataHeader.radioDataType!=RADIO_DATA_TYPE_FILE)
            return;

        sizeOfRcvData = 0;
        expectSizeRcvFile = structRadioDataHeader.fileSize;

        baRcvFileData.clear();

        rcvFileNameReduced = ( const char *)structRadioDataHeader.fileName;

        rcvFileStatus = FILE_IS_IN_RCV_PROCESS;

        return;
    }

    if(rcvFileStatus==FILE_IS_IN_RCV_PROCESS)
    {
        uint16_t nSizeFileDataInPack = (expectSizeRcvFile-sizeOfRcvData > nSizeBody ? nSizeBody : expectSizeRcvFile-sizeOfRcvData);

        //TODO Тут реализовано скрытие потенциальной ошибки! В файл надо писать именно столько данных, сколько приходит
        //от радиоустройства, но в последнем пакете размер часто бывает некорректным. Это должно быть подправлено со
        //стороны устройства
        baRcvFileData.append((char*)pBodyData,nSizeFileDataInPack);
        sizeOfRcvData+=nSizeFileDataInPack;

        if(sizeOfRcvData>=expectSizeRcvFile)
            rcvFileStatus = FILE_IS_RCVD;
    }

    if(rcvFileStatus==FILE_IS_RCVD)
    {
        fileRcvd.setFileName(rcvFileNameReduced);

        if (!fileRcvd.open(QIODevice::WriteOnly))    //открываем файл
        {
            qDebug() << "WARNING! QFileTransfer::slotProcessFileDataPack() Файл не может быть открыт для записи";
            errorStatus = ERROR_FILE_OPEN;
            return;
        }

        //Читаем все данные из файла
        fileRcvd.write(baRcvFileData);

        //Закрываем файл, он нам больше не нужен
        fileRcvd.close();
    }

    return;
}

quint16 QFileTransfer::SendSPIMMsgToDevice(QSmartRadioModuleControl* RadioDevice, SPIMMessage SPIMmsgForSend)
{
    //Теперь сообщение SPIMmsgForSend упаковываем в SLIP-протокол
    quint8 pSLIPPackData[SLIPInterface::MAX_SIZE_OF_PACK];
    quint16 nSLIPPackSize;

    memset(pSLIPPackData,0,SLIPInterface::MAX_SIZE_OF_PACK);

    //Формируем SLIP-пакет
    SLIPInterface::FormPack((uint8_t*)SPIMmsgForSend.Data, (uint16_t)SPIMmsgForSend.Size, (uint8_t*)pSLIPPackData, (uint16_t&)nSLIPPackSize);

    QByteArray baUSBPackForSend;
    baUSBPackForSend.append((const char*)pSLIPPackData, nSLIPPackSize);

    //Передаем данные в порт
    if(RadioDevice->SendDataToPort(baUSBPackForSend))
        return(false);
    else
        return(true);
}

