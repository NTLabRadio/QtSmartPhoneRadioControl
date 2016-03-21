/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** Этот файл описывает класс QFileTransfer, включающий в себя набор основных
** методов и свойств для реализации функции передачи файлов через радиоинтерфейс
**
****************************************************************************/

#ifndef QFILETRANSFER_H
#define QFILETRANSFER_H

#include <QObject>
#include <QFile>
#include <QDebug>

#include "SLIPinterface.h"
#include "SPIMMessage.h"
#include "qSmartRadioModuleControl.h"

class QFileTransfer : public QObject
{
    Q_OBJECT

public:
    explicit QFileTransfer(QObject *parent = 0);
    ~QFileTransfer();

    quint16 SetFileForSend(QString name);

    quint16 ResetFileForSend();

    quint16 PrepareToRcvNewFile();

    QString GetFullSendFileName();

    QString GetSendFileName();

    quint32 GetFileSize();

    quint32 GetSizeOfSendedFileData();

    QString GetRcvFileName();

    quint32 GetSizeOfRcvFile();

    quint32 GetSizeOfRcvdFileData();

    //Статус, указывающий текущее состояние процесса передачи файла:
    //Если статус - FILE_SEND_DENY, то см. errorStatus, расшифровывающий критическую
    //ошибку, из-за которой процесс передачи файла не может быть начат/продолжен/завершен
    enum en_sendFileStatuses
    {
        FILE_SEND_DENY,
        FILE_IS_READY_FOR_SEND,
        FILE_IS_IN_SEND_PROCESS,
        FILE_IS_SENDED
    };

    enum en_rcvFileStatuses
    {
        FILE_RCV_ERROR,
        FILE_RCV_WAITING,
        FILE_IS_IN_RCV_PROCESS,
        FILE_IS_RCVD
    };

    quint16 GetSendFileStatus();
    quint16 GetRcvFileStatus();

    enum en_transmitterStatuses
    {
        TRANSMITTER_BUSY,
        TRANSMITTER_FREE
    } ;

    quint16 GetTransmitterStatus();
    void SetTransmitterStatus(quint16 status);

    //Статус ошибки, указывающий тип критической ошибки, в результате которой
    //процесс передачи прерван/не может быть начат
    enum en_errorStatuses
    {
        ERROR_NONE,
        ERROR_FILE_IS_NOT_CHOSEN,
        ERROR_FILE_SIZE_IS_TOO_BIG,
        ERROR_FILE_OPEN,
        ERROR_DEVICE_CONNECTION,
        ERROR_DEVICE_EXCHANGE
    };

    quint16 GetErrorStatus();

    static const quint32 MAX_TIME_TRANSMITTER_BUSY_MS = 500;

signals:

public slots:
    void slotSendFileDataPack(QSmartRadioModuleControl* RadioDevice);
    void slotProcessFileDataPack(SPIMMessage* SPIMmsg);

private slots:

private:
    QFile fileForSend;
    QFile fileRcvd;

    QString fileName;
    QString fileNameReduced;    //только имя файла, без полного пути
    quint32 fileSize;

    quint32 sizeOfRcvData;
    quint32 expectSizeRcvFile;
    QString rcvFileNameReduced; //имя принимаемого файла (только имя, без полного пути)

    //Максимальный размер файла, который может передаваться
    static const quint32 MAX_SIZE_OF_FILE = (1024*1024);   //1 мБ

    quint32 sizeOfSendedData;

    en_sendFileStatuses sendFileStatus;
    en_rcvFileStatuses rcvFileStatus;
    en_transmitterStatuses transmitterStatus;
    en_errorStatuses errorStatus;

    //Данные файла, которые предстоит передать
    QByteArray baFileDataRestForSend;

    //Данные принимаемго файла
    QByteArray baRcvFileData;

    //Размер полезной нагрузки радиопакета
    static const quint16 SIZE_OF_RADIOPACK_PAYLOAD = 81;

    quint8 pSLIPPackData[SLIPInterface::MAX_SIZE_OF_PACK];
    quint16 nSLIPPackSize;

    quint8 noSPIMmsgs;

    static const quint16 MAX_SIZE_OF_FILE_NAME = 32;

    enum en_RadioDataTypes
    {
        RADIO_DATA_TYPE_NONE,
        RADIO_DATA_TYPE_FILE,
        NUM_RADIO_DATA_TYPES
    };

    struct RadioDataHeader
    {
        quint16 radioDataType;
        quint8 fileName[MAX_SIZE_OF_FILE_NAME];
        quint32 fileSize;
    };

    quint16 SendSPIMMsgToDevice(QSmartRadioModuleControl* RadioDevice, SPIMMessage SPIMmsgForSend);
};

#endif // QFILETRANSFER_H


