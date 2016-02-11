#include "qSmartRadioModuleControl.h"

QSmartRadioModuleControl::QSmartRadioModuleControl(QObject *parent) :
    QObject(parent)
{
    //Объект QSerialPort для работы с COM-портом, виртуальным (USB) или реальным
    serialPort = new QSerialPort(this);

    // Присоединяем к сигналу появления данных от устройства слот обработки данных, принятых из порта
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(ReadDataFromPort()));
}

QSmartRadioModuleControl::~QSmartRadioModuleControl()
{
    DisconnectDevice();     //если было подключено устройство, то отключаем его
    delete serialPort;
}

//-------------------------------------------------------------------------------------------------------------
// QSmartRadioModuleControl::isConnected()
//
// Описание:
//  Функция указывает, подключено ли какое-либо устройство
//
// Возвращаемое значение:
//  0 - не подключено;
//  1 - подключено
//-------------------------------------------------------------------------------------------------------------
bool QSmartRadioModuleControl::isConnected()
{
    return(serialPort->isOpen());
}


//-------------------------------------------------------------------------------------------------------------
// QSmartRadioModuleControl::portName()
//
// Описание:
//  Функция возвращает имя порта устройства, с которым в данный момент ведется работа
//
// Параметры:
//  нет
//
// Возвращаемое значение:
//  Имя порта в формате QString. Пример: "COM1"
//  Если порт не был открыт программой, функция возвращает пустую строку
//-------------------------------------------------------------------------------------------------------------
QString QSmartRadioModuleControl::portName()
{
    return(serialPort->portName());
}


//-------------------------------------------------------------------------------------------------------------
// QSmartRadioModuleControl::ConnectDevice()
//
// Описание:
//  Функция открывает порт, к которому подключено устройство. Вызывается в начале сеанса работы с радиостанцией
//
// Параметры:
//  ComPortName - строка с названием последовательного порта, к которому подключена радиостанция
//
// Возвращаемое значение:
//  Функция возвращает результат подключения устройства:
//  0x0000 – подключение выполнено успешно;
//  0xFFFF – ошибка связи с устройством / невозможно открыть порт с заданными параметрами
//-------------------------------------------------------------------------------------------------------------
en_results QSmartRadioModuleControl::ConnectDevice (QString ComPortName)
{
    serialPort->setPortName(ComPortName);
    if(serialPort->open(QIODevice::ReadWrite))  // открываем его
    {
        if(!serialPort->setBaudRate(QSerialPort::Baud57600))
        {
            qDebug() << "QRadioControl::ConnectDevice() Ошибка подключения к порту: setBaudRate error";
            return(RES_FAIL);
        }
        if(!serialPort->setDataBits(QSerialPort::Data8))
        {
            qDebug() << "QRadioControl::ConnectDevice() Ошибка подключения к порту: setDataBits error";
            return(RES_FAIL);
        }
        if(!serialPort->setStopBits(QSerialPort::OneStop))
        {
            qDebug() << "QRadioControl::ConnectDevice() Ошибка подключения к порту: setStopBits error";
            return(RES_FAIL);
        }
        if(!serialPort->setFlowControl(QSerialPort::NoFlowControl))
        {
            qDebug() << "QRadioControl::ConnectDevice() Ошибка подключения к порту: setFlowControl error";
            return(RES_FAIL);
        }
    }
    else
    {
        qDebug() << "QRadioControl::ConnectDevice() Ошибка при открытии порта";
        return(RES_FAIL);
    }

    qDebug() << "QRadioControl::ConnectDevice() Устройство успешно подключено";

    emit signalConnectDevice();

    return(RES_SUCCESS);

}


//-------------------------------------------------------------------------------------------------------------
// QSmartRadioModuleControl::DisconnectDevice()
//
// Описание:
//  Функция закрывает порт, к которому подключено устройство. Вызывается в конце сеанса работы с радиостанцией
//
// Параметры:
//  нет
//
// Возвращаемое значение:
//  Функция возвращает результат отключения устройства:
//  0x0000 – работа с устройством успешно закончена, порт закрыт;
//  0xFFFF – ошибка связи с устройством
//-------------------------------------------------------------------------------------------------------------
en_results QSmartRadioModuleControl::DisconnectDevice()
{
    if(isConnected())
    {
        serialPort->close();
        serialPort->setPortName("");
    }
    else
    {
        qDebug() << "Нет подключенных устройств. Нечего отключать";
        return(RES_FAIL);
    }

    qDebug() << "Устройство отключено";

    return(RES_SUCCESS);
}


// ------------------------------------------------------------------------------------------
// Функция-слот, предназначенная для обработки данных от устройства. Вызывается при появлении
// в буфере данных, принятых от устройства, новых данных
// ------------------------------------------------------------------------------------------
void QSmartRadioModuleControl::ReadDataFromPort()
{
    #ifdef DEBUG_QSRMODULE_SHOW_RAW_DATA
    QString strmes;
    int cntBytes;
    #endif

    baReceivedData.clear();
    baReceivedData = serialPort->readAll();

    #ifdef DEBUG_QSRMODULE_SHOW_RAW_DATA
        strmes = "QSmartRadioModuleControl::ReadDataFromPort() *** ";

        for (cntBytes = 0; cntBytes < baReceivedData.size(); cntBytes++)
            strmes += QString::number((uchar)baReceivedData[cntBytes], 16).toUpper() + " ";

        qDebug() << strmes;

        qDebug() << "QSmartRadioModuleControl::ReadDataFromPort() Размер сообщения: " << baReceivedData.size() << " байт";
    #endif

    if (!baReceivedData.size()) // Ничего не приняли
    {
        #ifdef DEBUG_QSRMODULE_SHOW_ERRORS
        //TODO Лушче бы это сделать через assert
        qDebug() << "QSmartRadioModuleControl::ReadDataFromPort() Сигнал был испущен, но ничего не прочитали из порта";
        #endif
        return;
    }

    emit signalReceiveData(baReceivedData);

    return;
}



//-------------------------------------------------------------------------------------------------------------
// QSmartRadioModuleControl::SendDataToPort()
//
// Описание:
//  Функция посылает пакет на устройство
//
// Параметры:
//  baDataForSend -- пакет в виде массива байт, который необходимо передать на устройство
//
// Возвращаемое значение:
//  Функция возвращает результат передачи пакета на устройство
//  0x0000 – пакет передан успешно;
//  0xFFFF – при попытке передачи пакета возникли какие-то траблы на физическом уровне интерфейса
//-------------------------------------------------------------------------------------------------------------
en_results QSmartRadioModuleControl::SendDataToPort(QByteArray baDataForSend)
{
    quint64 res;

    // Если устройство не подключено, сразу возвращаем ошибку
    if(!isConnected())
    {
        qDebug() << "QSmartRadioModuleControl::SendDataToPort() ERROR: Устройство не подключено";
        return(RES_FAIL);
    }

    // Посылаем данные в порт
    res = serialPort->write(baDataForSend);

    // Если не все данные переданы в порт, возвращаем ошибку
    if (res != quint64(baDataForSend.size()))
    {
        qDebug() << "QSmartRadioModuleControl::SendDataToPort() ERROR: Пакет не передан полностью!";
        return(RES_FAIL);
    }

    return(RES_SUCCESS);
}
