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
#include "qSmartRadioModuleControl.h"

class QFileTransfer : public QObject
{
    Q_OBJECT
public:
    explicit QFileTransfer(QObject *parent = 0);
    ~QFileTransfer();

    quint16 SetFileForSend(QString name);

    QString GetFileName();

    quint32 GetFileSize();

    quint32 GetSizeOfSendedFileData();

    quint16 GetTransferStatus();

    enum transferStatuses
    {
        FILE_IS_NOT_CHOSEN,
        FILE_IS_READY_FOR_TX,
        FILE_IS_IN_TX_PROCESS,
        FILE_IS_TRANSMITED,
        DEVICE_EXCHANGE_FAIL
    };

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

    quint16 transferStatus;

    QByteArray baDataForSend;

    //Размер полезной нагрузки радиопакета
    static const quint16 SIZE_OF_RADIOPACK_PAYLOAD = 81;

    quint8 pSLIPPackData[SLIPInterface::MAX_SIZE_OF_PACK];
    quint16 nSLIPPackSize;
};

#endif // QFILETRANSFER_H


