#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QtSerialPort/QSerialPortInfo>

#include "qSmartRadioModuleControl.h"
#include "slipinterface.h"
#include "SPIMMessage.h"

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

    void on_SendTestData_Button_clicked();

    void ReceiveDataFromRadioModule(QByteArray baRcvdData);

    void on_SendTestFile_Button_clicked();

    void SendWavFrame();

    void on_TestFileTransfer_Button_clicked();

    void on_SendSPIMData_Button_clicked();

private:
#define DE9943_PRODUCT_ID          (0x0585)             //ID продукта макетной платы радиоблока
#define DE9943_VENDOR_ID            (0x1747)             //ID производителя макетной платы радиоблока

    Ui::MainWindow *ui;

    QSmartRadioModuleControl* RadioDevice;    // Объект для работы с устройством

    SLIPInterface* objSLIPInterface;                    //Объект для работы с SLIP-интерфейсом

    SPIMMessage* objSPIMmsgTx;                     //Объект для формирования сообщений SPIM протокола

    QMenu* mnConnectDeviceSubMenu;            // Меню верхнего уровня. Пункт "Подключить"

    void CreateMenuBar();

    bool ConvertUTF8ToHexInt(quint8* pBufData,quint16 nSizeData);
    bool ConvertHexIntToUTF8(quint8* pBufData, quint16 nSizeData, QString& strMes);
    bool ConvertUTF8StringToHexInt(QString strMes, quint8* pHexData, quint16& nSizeData);

    void SetTestFileSendProgressBarFail();
    void SetTestFileSendProgressBarSuccess();

#define PERIOD_MS_POLLING_COM_DEVICES (1000)   //период опроса COM-устройств с целью отслеживания их присутствия в системе, мс
QTimer *timerPollingCOMDevices;                 // Таймер для периодического контроля подключения/отключения устройств
    void InitPollingCOMDevices();

#define PERIOD_MS_WAV_FRAME_SEND    (10)    //период посылки звуковых данных на устройство
QTimer *timerWavSend;                               // Таймер для периодической посылки звуковых данных на устройство
    void InitWavSendTimer();
};

#endif // MAINWINDOW_H
