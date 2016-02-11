/****************************************************************************
**
** Copyright (C) 2016 "NTLab"
**
** Этот файл описывает класс QSmartRadioModuleControl, включающий в себя набор основных
** методов и свойств для реализации функций обмена с COM-устройствами, и служит для
** упрощения интерфейса, предоставляемого QtSerialPort
**
****************************************************************************/

#ifndef QSMARTRADIOMODULECONTROL_H
#define QSMARTRADIOMODULECONTROL_H

#include <QObject>

#include <QtSerialPort/QtSerialPort>
#include <QtSerialPort/QSerialPortInfo>


enum en_results
{
    RES_SUCCESS,
    RES_FAIL        = (0xFFFF)
};

class QSmartRadioModuleControl : public QObject
{
    Q_OBJECT
public:
    explicit QSmartRadioModuleControl(QObject *parent = 0);
    ~QSmartRadioModuleControl();

    en_results ConnectDevice (QString ComPortName);
    en_results DisconnectDevice ( );

    quint16 CheckRadioModule ( );

    bool isConnected();
    QString portName();

    en_results SendDataToPort(QByteArray baDataForSend);

signals:
    void signalReceiveData(QByteArray baData);
    void signalConnectDevice();

public slots:

private slots:
    void ReadDataFromPort();

private:
    QSerialPort *serialPort;

    // данные, принятые от устройства
    QByteArray baReceivedData;
};

#endif // QSMARTRADIOMODULECONTROL_H
