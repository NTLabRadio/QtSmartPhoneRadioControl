#include "QFileTransfer.h"

QFileTransfer::QFileTransfer(QObject *parent) :
    QObject(parent)
{
    fileName = "";
    fileSize = 0;

    sizeOfSendedData = 0;

    transferStatus = FILE_SEND_DENY;
    errorStatus = ERROR_FILE_IS_NOT_CHOSEN;
    transmitterStatus = TRANSMITTER_FREE;
}


QFileTransfer::~QFileTransfer()
{

}


quint16 QFileTransfer::SetFileForSend(QString name)
{
    //Пока не открыли и не прочитали файл, считаем, что файл не выбран
    transferStatus = FILE_SEND_DENY;
    errorStatus = ERROR_FILE_IS_NOT_CHOSEN;

    fileName = name;

    fileForSend.setFileName(name);

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
    transferStatus = FILE_IS_READY_FOR_SEND;
    errorStatus = ERROR_NONE;
    transmitterStatus = TRANSMITTER_FREE;

    return(1);
}


quint16 QFileTransfer::ResetFileForSend()
{
    //Пока не открыли и не прочитали файл, считаем, что файл не выбран
    transferStatus = FILE_SEND_DENY;
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
    transferStatus = FILE_IS_READY_FOR_SEND;
    errorStatus = ERROR_NONE;
    transmitterStatus = TRANSMITTER_FREE;

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


quint16 QFileTransfer::GetTransferStatus()
{
    return(transferStatus);
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

void QFileTransfer::slotSendDataPack(QSmartRadioModuleControl* RadioDevice)
{
    if(!RadioDevice->isConnected())
    {
        transferStatus = FILE_SEND_DENY;
        errorStatus = ERROR_DEVICE_CONNECTION;
        return;
    }

    if((transferStatus==FILE_IS_READY_FOR_SEND) || (transferStatus==FILE_IS_IN_SEND_PROCESS))
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
                transferStatus=FILE_SEND_DENY;
        }
        else
            transferStatus=FILE_IS_SENDED;
    }
}
