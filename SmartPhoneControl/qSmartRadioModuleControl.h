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

    en_results SendData(QByteArray baDataForSend);

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
