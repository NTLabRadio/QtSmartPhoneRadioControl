#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>

#include "qSmartRadioModuleControl.h"

QT_USE_NAMESPACE


#define DE9943_PRODUCT_ID          (0x0585)
#define DE9943_VENDOR_ID            (0x1747)

#define PERIOD_MS_POLLING_COM_DEVICES (1000)   //период опроса COM-устройств с целью отслеживания их присутствия в системе, мс

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

private:
    Ui::MainWindow *ui;

    QTimer *timerPollingCOMDevices;                 // Таймер для периодического контроля подключения/отключения устройств

    QSmartRadioModuleControl* RadioDevice;    // Объект для работы с устройством

    void CreateMenuBar();                                 // Создание меню верхнего уровня
    QMenu* mnConnectDeviceSubMenu;            // Меню верхнего уровня. Пункт "Подключить"
};

#endif // MAINWINDOW_H
