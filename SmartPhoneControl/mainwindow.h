#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>

#include "qSmartRadioModuleControl.h"
#include "slipinterface.h"

QT_USE_NAMESPACE


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void CheckCOMDevices();
    quint8 CloseConnectedDevice();
    void ConnectCOMDevice();

private slots:
    void ShowAvailPortsInMenu();

    void on_pushButton_clicked();

    void ReceiveDataFromRadioModule(QByteArray baRcvdData);

private:
#define DE9943_PRODUCT_ID          (0x0585)             //ID продукта макетной платы радиоблока
#define DE9943_VENDOR_ID            (0x1747)             //ID производителя макетной платы радиоблока

#define PERIOD_MS_POLLING_COM_DEVICES (1000)   //период опроса COM-устройств с целью отслеживания их присутствия в системе, мс


    Ui::MainWindow *ui;

    QTimer *timerPollingCOMDevices;                 // Таймер для периодического контроля подключения/отключения устройств

    QSmartRadioModuleControl* RadioDevice;    // Объект для работы с устройством

    SLIPInterface* objSLIPInterface;                    //Объект для работы с SLIP-интерфейсом

    QMenu* mnConnectDeviceSubMenu;            // Меню верхнего уровня. Пункт "Подключить"

    void CreateMenuBar();
    void InitPollingCOMDevices();

    bool ConvertUTF8ToHexInt(quint8* pBufData,quint16 nSizeData);
    bool ConvertHexIntToUTF8(quint8* pBufData, quint16 nSizeData);
    bool ConvertHexIntToUTF8(quint8* pBufData, quint16 nSizeData, QString& strMes);
};

#endif // MAINWINDOW_H
