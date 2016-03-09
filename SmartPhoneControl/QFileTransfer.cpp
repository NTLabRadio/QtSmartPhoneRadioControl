#include "QFileTransfer.h"

QFileTransfer::QFileTransfer(QObject *parent) :
    QObject(parent)
{
    fileName = "";
    fileSize = 0;

    sizeOfSendedData = 0;
}


QFileTransfer::~QFileTransfer()
{

}


quint16 QFileTransfer::SetFileForSend(QString name)
{
    //Пока не открыли и не прочитали файл, считаем, что файл не выбран
    transferStatus = FILE_IS_NOT_CHOSEN;

    fileName = name;

    fileForSend.setFileName(name);

    //Определяем размер файла
    fileSize = fileForSend.size();

    if(fileSize>MAX_SIZE_OF_FILE)
    {
        qDebug() << "WARNING! QFileTransfer::SetFileForSend() Файл не может быть открыт, поскольку слишком большой";
        //TODO изменить transferStatus
        //"Данные не могут быть переданы. Файл не может быть открыт"
        return(0);
    }

    //Устанавливаем размер переданных данных нулевым, поскольку пока ничего не передали
    sizeOfSendedData = 0;

    if (!fileForSend.open(QIODevice::ReadOnly))    //открываем файл
    {
        qDebug() << "WARNING! QFileTransfer::SetFileForSend() Файл не может быть открыт";
        //TODO изменить transferStatus
        //"Данные не могут быть переданы. Файл не может быть открыт"
        return(0);
    }

    //Читаем все данные из файла
    baDataForSend = fileForSend.readAll();

    //Закрываем файл, он нам больше не нужен
    fileForSend.close();

    //Теперь мы готовы к передаче файла
    transferStatus = FILE_IS_READY_FOR_TX;

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

void QFileTransfer::slotSendDataPack(QSmartRadioModuleControl* RadioDevice)
{
    if((transferStatus==FILE_IS_READY_FOR_TX) || (transferStatus==FILE_IS_IN_TX_PROCESS))
    {
        //Размер данных, которые еще необходимо передать
        quint32 sizeOfRestData = fileSize-sizeOfSendedData;

        quint8 pPackPayloadData[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
        quint16 nPackPayloadSize;

        if(sizeOfRestData)
        {
            if(sizeOfRestData>SIZE_OF_RADIOPACK_PAYLOAD)
                nPackPayloadSize = SIZE_OF_RADIOPACK_PAYLOAD;
            else
                nPackPayloadSize = (quint16)sizeOfRestData;
            memcpy(pPackPayloadData, baDataForSend.data(), nPackPayloadSize);
            baDataForSend.remove(0,nPackPayloadSize);

            sizeOfSendedData+=nPackPayloadSize;

            memset(pPackPayloadData,0,MAX_SIZE_OF_SLIP_PACK_PAYLOAD);

            //Формируем SLIP-пакет
            SLIPInterface::FormPack((uint8_t*)pPackPayloadData, (uint16_t)nPackPayloadSize, (uint8_t*)pSLIPPackData, (uint16_t&)nSLIPPackSize);

            baDataForSend.append((const char*)pSLIPPackData, nSLIPPackSize);

            #ifdef DEBUG_LOG_SIZE_OF_UART_TX_PACK
            qDebug() << "Передаем в порт SLIP-пакет размером" << nSLIPPackSize << "байт";
            qDebug() << "Размер полезной нагрузки SLIP пакета:" << nSLIPPayloadSize << "байт";
            #endif

            //Передаем данные в порт
            if(RadioDevice->SendDataToPort(baDataForSend))
                transferStatus=DEVICE_EXCHANGE_FAIL;
        }
        else
            transferStatus=FILE_IS_TRANSMITED;
    }
}
