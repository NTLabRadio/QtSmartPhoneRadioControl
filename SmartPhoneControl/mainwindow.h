#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QtSerialPort/QSerialPortInfo>
#include <QPalette>

#include "QFileTransfer.h"
#include "QRadioModuleSettings.h"
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

signals:
    void signalSendNextFilePack(QSmartRadioModuleControl*);
    void signalRcvFilePack(SPIMMessage* SPIMmsg);

private slots:
    void ShowAvailPortsInMenu();

    void on_SendTestData_Button_clicked();

    void ReceiveDataFromRadioModule(QByteArray baRcvdData);

    void on_SendTestFile_Button_clicked();

    void SendWavFrame();

    void on_TestFileTransfer_Button_clicked();

    void on_SendSPIMData_Button_clicked();

    void on_FileTransfer_Button_clicked();

    void on_SendFile_Button_clicked();

    void SendFilePackToTransceiver();

    void CheckFileTransferStatus();

    void CheckFileReceiveStatus();

    void on_Volume_spinBox_valueChanged(int arg1);

    void on_MicSens_spinBox_valueChanged(int arg1);

    void ReqRadioModuleParams();

    void ClearRadioModuleParams();

    void on_FreqTx_SpinBox_valueChanged(double arg1);

    void on_FreqRx_SpinBox_valueChanged(double arg1);

    void on_flagSimplex_checkBox_clicked(bool checked);

    void on_OpMode_comboBox_currentIndexChanged(int index);

private:
    static const quint16  DE9943_PRODUCT_ID = 0x0585;           //ID продукта макетной платы радиоблока
    static const quint16  DE9943_VENDOR_ID = 0x1747;             //ID производителя макетной платы радиоблока

    Ui::MainWindow *ui;

    QSmartRadioModuleControl* RadioDevice;    // Объект для работы с устройством

    QRadioModuleSettings* RadioModuleSettings;

    SLIPInterface* objSLIPInterface;                    //Объект для работы с SLIP-интерфейсом

    SPIMMessage* objSPIMmsgTx;                     //Объект для формирования сообщений SPIM протокола

    QMenu* mnConnectDeviceSubMenu;            // Меню верхнего уровня. Пункт "Подключить"

    void CreateMenuBar();

    bool ConvertUTF8ToHexInt(quint8* pBufData,quint16 nSizeData);
    bool ConvertHexIntToUTF8(quint8* pBufData, quint16 nSizeData, QString& strMes);
    bool ConvertUTF8StringToHexInt(QString strMes, quint8* pHexData, quint16& nSizeData);

    void SetTestFileSendProgressBarFail();
    void SetTestFileSendProgressBarSuccess();

    void ShowRcvdSLIPPack(QByteArray baRcvdData);
    void ShowRcvdSPIMMsg(SPIMMessage SPIMBackCmd);

    static const quint16  PERIOD_MS_POLLING_COM_DEVICES  = 1000;   //период опроса COM-устройств с целью отслеживания их присутствия в системе, мс
    QTimer *timerPollingCOMDevices;                 // Таймер для периодического контроля подключения/отключения устройств
    void InitPollingCOMDevices();

    static const quint16 PERIOD_MS_WAV_FRAME_SEND = 10;    //период посылки звуковых данных на устройство
    QTimer *timerWavSend;                               // Таймер для периодической посылки звуковых данных на устройство
    void InitWavSendTimer();

    void ClearDebugSLIPStatuses();

    enum   enStateSendFile
    {
        STATE_IDLE_FILE_SEND,
        STATE_RUNNING_FILE_SEND,
        STATE_END_FILE_SEND
    };

    static const quint16 SIZE_OF_WAV_FRAME_SAMPLES = 80;

    QString NameOfTestfFile;
    QFile TestFile;
    quint32 lSizeTestFile;
    quint32 lSizeTestFileRestToSend;
    QByteArray baDataTestFile;

    enStateSendFile nStateSendTestFile;

    QFileTransfer FileTransmitter;

    void ShowFileTransferError(QString strError);
    void ShowFileTransferSuccess(QString strSuccessMsg);
    void ClearFileTransferError();

    // Таймер периодического контроля процесса передачи файла на устройство
    QTimer *timerFileSend =  NULL;
    //Период проверки состояния радиомодуля в процессе передачи файла на устройство
    static const quint16 PERIOD_MS_CHECK_FILE_SEND_STATUS = 200;
    quint16 TimeIntervalMsSendRadioPack;

    // Таймер периодического контроля процесса приема файла от устройства
    QTimer *timerFileRcv =  NULL;
    //Период проверки состояния радиомодуля в процессе приема файла
    static const quint16 PERIOD_MS_CHECK_FILE_RCV_STATUS = 200;
    quint16 TimeIntervalMsCheckFileRcvStatus;

    quint32 CurSizeOfRcvdFileData;
    quint32 TimeMsFileRcvNoActivity;
    static const quint32 MAX_TIME_NO_RCV_FILE_ACTIVITY_MS = 2000;

    quint32 timeTransmitterBusyMs;

    void InitFileSendTimer();
    void DeinitFileSendTimer();

    void InitFileRcvTimer();
    void DeinitFileRcvTimer();

    void SetFileSendProgressBarFail();
    void SetFileSendProgressBarSuccess();
    void SetFileSendProgressBarInitProcess();

    void ShowFileSendErrorStatus();

    void ProcessDataPackFromRadioModule(SPIMMessage* pSPIMMsgRcvd);
    void ProcessCmdFromRadioModule(SPIMMessage* SPIMCmd);
    void ProcessCurParamFromRadioModule(SPIMMessage* SPIMCmdRcvd);

    void SendSPIMMsg(uint8_t nIDCmd);

    void ShowRadioModuleParams();

    bool IsAllowSetRadioModuleParams();
    void DenySetRadioModuleParams();
    void AllowSetRadioModuleParams();

    bool flAllowSetRadioModuleParams;

    void SetTimeIntervalsForCheckFileTransferStatus();


};

#endif // MAINWINDOW_H
