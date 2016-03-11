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

    QString GetFileName();

    quint32 GetFileSize();

    quint32 GetSizeOfSendedFileData();

    //Статус, указывающий текущее состояние процесса передачи файла:
    //Если статус - FILE_SEND_DENY, то см. errorStatus, расшифровывающий критическую
    //ошибку, из-за которой процесс передачи файла не может быть начат/продолжен/завершен
    enum en_transferStatuses
    {
        FILE_SEND_DENY,
        FILE_IS_READY_FOR_SEND,
        FILE_IS_IN_SEND_PROCESS,
        FILE_IS_SENDED
    } ;

    quint16 GetTransferStatus();

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
    void slotSendDataPack(QSmartRadioModuleControl* RadioDevice);

private slots:

private:
    QFile fileForSend;

    QString fileName;
    quint32 fileSize;

    //Максимальный размер файла, который может передаваться
    static const quint32 MAX_SIZE_OF_FILE = (1024*1024);   //1 мБ

    quint32 sizeOfSendedData;

    en_transferStatuses transferStatus;
    en_transmitterStatuses transmitterStatus;
    en_errorStatuses errorStatus;

    QByteArray baFileDataRestForSend;

    //Размер полезной нагрузки радиопакета
    static const quint16 SIZE_OF_RADIOPACK_PAYLOAD = 81;

    quint8 pSLIPPackData[SLIPInterface::MAX_SIZE_OF_PACK];
    quint16 nSLIPPackSize;

    quint8 noSPIMmsgs;
};

#endif // QFILETRANSFER_H


