#include "mainwindow.h"
#include "ui_mainwindow.h"

#define DEBUG_SHOW_RCVD_SPIM_MSG


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("SmartPhoneControl ver 1.1");

    RadioDevice = new QSmartRadioModuleControl;

    //Создание меню верхнего уровня
    CreateMenuBar();

    //Инициализируем процесс периодического контроля подключения/отключения COM-устройств
    InitPollingCOMDevices();

    // Присоединяем к сигналу успешного подключения устройства слот запроса ID устройства
    connect(RadioDevice, SIGNAL(signalConnectDevice()), this, SLOT(RequestR181GenParams()));

    // Обработка данных, принятых от устройства
    connect(RadioDevice, SIGNAL(signalReceiveData(QByteArray)),this, SLOT(ReceiveDataFromRadioModule(QByteArray)));

    connect(this, SIGNAL(signalSendNextFilePack(QSmartRadioModuleControl*)),&FileTransmitter, SLOT(slotSendDataPack(QSmartRadioModuleControl*)));

    //Создаем объект для формирования сообщений SPIM протокола
    objSPIMmsgTx = new SPIMMessage;

    ui->FileTxStatus_label->setText("");

    ui->SLIPTxStatus_label->setText("");
    ui->SLIPRxStatus_label->setText("");
    ui->SLIPFileTxStatus_label->setText("");
    ui->SPIMTxStatus_label->setText("");
    ui->SPIMRxStatus_label->setText("");

    InitWavSendTimer();

    nStateSendTestFile = STATE_IDLE_FILE_SEND;

    timeTransmitterBusyMs = 0;

}

MainWindow::~MainWindow()
{
    delete RadioDevice;
    delete timerPollingCOMDevices;
    delete mnConnectDeviceSubMenu;
    delete objSPIMmsgTx;
    delete ui;
}

//-------------------------------------------------------------------------------------------------------------
//Функция создания основного меню верхнего уровня
//-------------------------------------------------------------------------------------------------------------
void MainWindow::CreateMenuBar()
{
    // Пункт меню "Устройство"
    QMenu * mnDevice = new QMenu("Устройство");     // Создаем
    ui->menuBar->addMenu(mnDevice);                 // Добавляем в menuBar

    //-------- Создаем подпункты меню "Устройство" -----------
    // Подпункт "Подключить"
    //mnDevice->addAction("&Подключить", mnDevice, SLOT(showDevices()), Qt::CTRL+Qt::Key_C);
    mnConnectDeviceSubMenu = new QMenu("&Подключить", mnDevice);
    mnDevice->addMenu(mnConnectDeviceSubMenu);
    connect(mnConnectDeviceSubMenu, SIGNAL(aboutToShow()), this, SLOT(ShowAvailPortsInMenu()));

    // Подпункт "Отключить"
    QAction *actDisconnect = new QAction("Отключить",mnDevice);
    connect(actDisconnect, SIGNAL(triggered()), this, SLOT(CloseConnectedDevice()));
    mnDevice->addAction(actDisconnect);
}

//-------------------------------------------------------------------------------------------------------------
//Поиск в системе COM-портов и их отображение в меню верхнего уровня
//
// TODO Эта функция не должна непосредственно использовать класс QSerialPortInfo, определять
// свойства COM-устройств, сравнивать ID производителя и продукта с заданными. Это должны делать
// функции другого уровня абстракции. Данная функция должна лишь выводить текстовую
// информацию, предоставленную классом для работы с COM-устройствами. Класс MainWindow не стоит
// усложнять
//-------------------------------------------------------------------------------------------------------------
void MainWindow::ShowAvailPortsInMenu()
{
    QAction* actDevice;

    mnConnectDeviceSubMenu->clear();

    //Для каждого найденного в системе устройства
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // Создаем подпункт в меню "Подключить"
        // При если идентификатор продукта и производителя совпадет с идентификаторами, используемыми
        // по умолчанию радиостанциями, то это отражается в названии подпункта
        if(info.productIdentifier()==DE9943_PRODUCT_ID && info.vendorIdentifier()==DE9943_VENDOR_ID)
            actDevice = mnConnectDeviceSubMenu->addAction(info.portName() + " (DE9943)");
        else
            actDevice = mnConnectDeviceSubMenu->addAction(info.portName());

        //К пункту меню как к объекту пристегиваем свойство, чтобы при вызове слота его идентифицировать
        actDevice->setProperty("SelectedDevice",info.portName());

        //К сигналу выбора подпункта (устройства) подключаем слот, выполняющий открытие соответствующего порта
        connect(actDevice, SIGNAL(triggered()), this, SLOT(ConnectCOMDevice()));

        //Включаем возможность "чекать" пункт меню
        actDevice->setCheckable(true);

        //И отмечаем устройство, если оно уже подключено
        if((RadioDevice->isConnected())&&(RadioDevice->portName()==info.portName()))
            actDevice->setChecked(true);
        else
            actDevice->setChecked(false);
    }
}

//-------------------------------------------------------------------------------------------------------------
// Функция подключает устройство, выбранное в меню верхнего уровня
//-------------------------------------------------------------------------------------------------------------
void MainWindow::ConnectCOMDevice()
{
    // По свойству объекта, который вызвал слот, определяем, какой порт необходимо открыть
    QString DeviceName = sender()->property("SelectedDevice").toString();

    // Если ранее был открыт другой порт, то закрываем его
    if((RadioDevice->isConnected())&&(RadioDevice->portName()!=DeviceName))
        CloseConnectedDevice();

    // Если выбранный порт не был открыт ранее,
    if(!RadioDevice->isConnected())
        RadioDevice->ConnectDevice(DeviceName);     //то подключаемся к этому порту для соединения с устройством
}

//-------------------------------------------------------------------------------------------------------------
// Функция закрывает порт, который использовался для соединения с устройством
//-------------------------------------------------------------------------------------------------------------
quint8 MainWindow::CloseConnectedDevice()
{
    return(RadioDevice->DisconnectDevice());
}

//-------------------------------------------------------------------------------------------------------------
// Функция инициализации процесса периодического контроля подключения/отключения
// COM-устройств
//
// TODO Эта функция должна принадлежать классу QSmartRadioModuleControl или другому
// того же уровня абстракции. Класс MainWindow не стоит усложнять
//-------------------------------------------------------------------------------------------------------------
void MainWindow::InitPollingCOMDevices()
{
    //Объект QTimer для периодического контроля подключения/отключения устройств
    timerPollingCOMDevices = new QTimer(this);
    connect(timerPollingCOMDevices,SIGNAL(timeout()),this,SLOT(CheckCOMDevices()));
    timerPollingCOMDevices->setInterval(PERIOD_MS_POLLING_COM_DEVICES);
    timerPollingCOMDevices->start();

    return;
}


//-------------------------------------------------------------------------------------------------------------
// Функция отображает все COM-устройства, которые есть в системе, в поле textBrowser тестовой вкладки
//
// TODO Эта функция не должна непосредственно использовать класс QSerialPortInfo и определять свойства
// COM-устройств. Это должны делать функции другого уровня абстракции. Данная функция должна лишь
// выводить текстовую информацию, предоставленную классом для работы с COM-устройствами,
// в textBrowser. Класс MainWindow не стоит усложнять
//-------------------------------------------------------------------------------------------------------------
void MainWindow::CheckCOMDevices()
{
    ui->Devices_textBrowser->clear();

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
            QString s = QObject::tr("Port: ") + info.portName() + "\n"
                        + QObject::tr("Vendor Identifier: ") + (info.hasVendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : QString()) + "\n"
                        + QObject::tr("Product Identifier: ") + (info.hasProductIdentifier() ? QString::number(info.productIdentifier(), 16) : QString()) + "\n"
                        + QObject::tr("Busy: ") + (info.isBusy() ? QObject::tr("Yes") : QObject::tr("No"));

            ui->Devices_textBrowser->append(s);
            ui->Devices_textBrowser->append("------------------------------------------------------");
    }
}


quint8 pSLIPPayloadData[MAX_SIZE_OF_SLIP_PACK_PAYLOAD];
quint16 nSLIPPayloadSize = 0;

#define MAX_SIZE_OF_SLIP_PACK   (2*MAX_SIZE_OF_SLIP_PACK_PAYLOAD)
quint8 pSLIPPackData[MAX_SIZE_OF_SLIP_PACK];
quint16 nSLIPPackSize = 0;

//--------------------------------------------------------------------------------------------------------------
// Функция разбирает масcив, состоящий из hex-символов ('0'-'9','A'-'F','a'-'f') в формате UTF8
// и преобразует его в массив бинарных данных, упаковывая по 2 символа в байт
//
// Параметры:
//  pBufData - указатель на данные (входные и выходные);
//  nSizeData - размер входных данных, байт;
//
// NO:
// Размер выходных данных - в 2 раза меньше размера входных в вижу упаковки данных
// Написано наскоро и с жестким дублированием кода (да, мне стыдно)
//--------------------------------------------------------------------------------------------------------------
bool MainWindow::ConvertUTF8ToHexInt(quint8* pBufData,quint16 nSizeData)
{
    quint16 cntBytes;

    if(nSizeData%2)
        return 0;

    for(cntBytes=0; cntBytes<nSizeData/2; cntBytes++)
    {
        if((pBufData[2*cntBytes]>=0x30)&&(pBufData[2*cntBytes]<=0x39))
        {
            pBufData[2*cntBytes]-=0x30;
            pBufData[cntBytes] = pBufData[2*cntBytes]<<4;
        }
        else if((pBufData[2*cntBytes]>=0x41)&&(pBufData[2*cntBytes]<=0x46))
        {
            pBufData[2*cntBytes]-=0x37;
            pBufData[cntBytes] = pBufData[2*cntBytes]<<4;
        }
        else if((pBufData[2*cntBytes]>=0x61)&&(pBufData[2*cntBytes]<=0x66))
        {
            pBufData[2*cntBytes]-=0x57;
            pBufData[cntBytes] = pBufData[2*cntBytes]<<4;
        }
        else
            return 0;

        if((pBufData[2*cntBytes+1]>=0x30)&&(pBufData[2*cntBytes+1]<=0x39))
        {
            pBufData[2*cntBytes+1]-=0x30;
            pBufData[cntBytes] |= pBufData[2*cntBytes+1];
        }
        else if((pBufData[2*cntBytes+1]>=0x41)&&(pBufData[2*cntBytes+1]<=0x46))
        {
            pBufData[2*cntBytes+1]-=0x37;
            pBufData[cntBytes] |= pBufData[2*cntBytes+1];
        }
        else if((pBufData[2*cntBytes+1]>=0x61)&&(pBufData[2*cntBytes+1]<=0x66))
        {
            pBufData[2*cntBytes+1]-=0x57;
            pBufData[cntBytes] |= pBufData[2*cntBytes+1];
        }
        else
            return 0;
    }

    return 1;
}


bool MainWindow::ConvertHexIntToUTF8(quint8* pBufData, quint16 nSizeData, QString& strMes)
{
    for (quint16 r = 0; r < nSizeData; r++)
        strMes += QString::number((uchar)pBufData[r], 16).toUpper() + " ";

    return 1;
}

bool MainWindow::ConvertUTF8StringToHexInt(QString strMes, quint8* pHexData, quint16& nSizeData)
{
    bool ok;
    QString fragmStrMes;

    if(strMes.size()%2)
        return(0);
    else
        nSizeData = strMes.size()/2;

    for (quint16 r = 0; r < nSizeData; r++)
    {
        fragmStrMes = strMes.mid(2*r,2);
        pHexData[r] = fragmStrMes.toInt(&ok, 16);

        if(!ok)
            return(0);
    }

    return(1);
}

void MainWindow::on_SendTestData_Button_clicked()
{
    QByteArray baDataForSend;
    QString strMes;

    //Определяем размер полезных данных для передачи
    nSLIPPayloadSize = ui->SLIPTxPayload_lineEdit->text().length();

    //Читаем строку, из которой надо сформировать SLIP-пакет, записываем в pSLIPPayloadData в UTF8-формате
    //Строка должна содержать только hex-символы ('0'-'9','A'-'F','a'-'f')
    memcpy(pSLIPPayloadData, ui->SLIPTxPayload_lineEdit->text().toUtf8().data(), nSLIPPayloadSize);

    //Преобразуем строку в масcив чисел 0-F
    if(!ConvertUTF8ToHexInt(pSLIPPayloadData,nSLIPPayloadSize))
    {
        //Ошибка при разборе сообщения для передачи. Выходим из функции, сообщаем пользователю об ошибке
        ui->SLIPTxStatus_label->setText("Неверный формат данных! Строка должна состоять из четного числа символов\n и содержать только hex-символы (0-9,A-F,a-f)");
        return;
    }
    else
        ui->SLIPTxStatus_label->setText("");

    memset(pSLIPPackData,0,MAX_SIZE_OF_SLIP_PACK);

    //Формируем SLIP-пакет
    SLIPInterface::FormPack((uint8_t*)pSLIPPayloadData, (uint16_t)nSLIPPayloadSize/2, (uint8_t*)pSLIPPackData, (uint16_t&)nSLIPPackSize, 256);


    //Проверяем, подключено ли устройство
    if(!RadioDevice->isConnected())
    {
        ui->SLIPTxStatus_label->setText("Данные не могут быть переданы. Устройство не подключено");
        return;
    }

    baDataForSend.append((const char*)pSLIPPackData, nSLIPPackSize);

    //Передаем данные в порт
    RadioDevice->SendDataToPort(baDataForSend);


    //Преобразуем массив чисел в строку из hex-символов ('0'-'9','A'-'F')
    ConvertHexIntToUTF8(pSLIPPackData,nSLIPPackSize,strMes);

    //Показываем пользователю данные, которые переданы в порт
    ui->SLIPTxData_lineEdit->setText (strMes);

}


void MainWindow::ReceiveDataFromRadioModule(QByteArray baRcvdData)
{
    #ifdef DEBUG_SHOW_RCVD_SLIP_PACK
    ShowRcvdSLIPPack(QByteArray baRcvdData);
    #endif

    uint8_t* pSPIMmsgData;
    uint16_t SPIMmsgSize;
    uint16_t nPosEndOfSLIPPack;

    //Создаем сообщение SPIM протокола
    SPIMMessage SPIMBackCmd;

    //Получаем указатель на данные сообщения
    pSPIMmsgData = SPIMBackCmd.Data;

    //Ищем в принятых данных SLIP-пакет.
    if(SLIPInterface::FindPackInData((uint8_t*)baRcvdData.data(), (uint16_t)baRcvdData.size(), pSPIMmsgData, SPIMmsgSize, nPosEndOfSLIPPack)!=2)
        return;

    //Если найден SLIP пакет, записываем его полезные данные в SPIM сообщение
    SPIMBackCmd.setMsg(pSPIMmsgData,SPIMmsgSize);

    #ifdef DEBUG_SHOW_RCVD_SPIM_MSG
    ShowRcvdSPIMMsg(SPIMBackCmd);
    #endif


}


void MainWindow::ShowRcvdSLIPPack(QByteArray baRcvdData)
{
    QString strMes;

    //Преобразуем принятые бинарные данные в строку из hex-символов ('0'-'9','A'-'F') для отображения
    ConvertHexIntToUTF8((quint8*)baRcvdData.data(), baRcvdData.size(), strMes);

    //Показываем пользователю данные, принятые из порта
    ui->SLIPRxData_lineEdit->setText (strMes);
}


void MainWindow::ShowRcvdSPIMMsg(SPIMMessage RcvdSPIMMsg)
{
    uint8_t bodySize = RcvdSPIMMsg.getSizeBody();
    ui->SPIM_RX_LEN_lineEdit->setText(QString::number(bodySize,16).toUpper());

    uint8_t address = RcvdSPIMMsg.getAddress();
    ui->SPIM_RX_ADR_lineEdit->setText(QString::number(address,16).toUpper());

    uint8_t noMsg = RcvdSPIMMsg.getNoMsg();
    ui->SPIM_RX_NO_lineEdit->setText(QString::number(noMsg,16).toUpper());

    uint8_t IDcmd = RcvdSPIMMsg.getIDCmd();
    ui->SPIM_RX_ID_lineEdit->setText(QString::number(IDcmd,16).toUpper());

    uint8_t* pSPIMbodyData;
    pSPIMbodyData = RcvdSPIMMsg.Body;
    QString strSPIMbodyHexData;

    ConvertHexIntToUTF8(pSPIMbodyData, bodySize, strSPIMbodyHexData);
    ui->SPIM_RX_DATA_lineEdit->setText(strSPIMbodyHexData);

    uint8_t CRCmsg = RcvdSPIMMsg.getCRC();
    ui->SPIM_RX_CRC_lineEdit->setText(QString::number(CRCmsg,16).toUpper());

    QPalette palCRClabel = ui->SPIM_RX_CRC_label->palette();
    if(RcvdSPIMMsg.checkCRC())        
        //CRC верна. Раскрашиваем CRC_label зеленым цветом
        palCRClabel.setColor(QPalette::WindowText, QColor("green"));
    else
        //CRC не верна. Раскрашиваем CRC_label красным цветом
        palCRClabel.setColor(QPalette::WindowText, QColor("red"));
}


void MainWindow::InitWavSendTimer()
{
    //Объект QTimer для периодической посылки звуковых данных на устройство для теста
    timerWavSend = new QTimer(this);
    connect(timerWavSend,SIGNAL(timeout()),this,SLOT(SendWavFrame()));
    timerWavSend->setInterval(PERIOD_MS_WAV_FRAME_SEND);
    timerWavSend->start();

    return;
}



void MainWindow::on_SendTestFile_Button_clicked()
{
    if(TestFile.fileName()=="")
    {
        ui->SLIPFileTxStatus_label->setText("Данные не могут быть переданы. Не выбран файл");
        return;
    }

    if (!TestFile.open(QIODevice::ReadOnly))    //открываем его
    {
        qDebug() << "WARNING! Тестовый файл не может быть открыт";
        ui->SLIPFileTxStatus_label->setText("Данные не могут быть переданы. Файл не может быть открыт");
        return;
    }

    ui->SLIPFileTxStatus_label->setText("");

    lSizeTestFile = TestFile.size();
    lSizeTestFileRestToSend = lSizeTestFile;

    baDataTestFile = TestFile.readAll();

    TestFile.close();

    nStateSendTestFile = STATE_RUNNING_FILE_SEND;
}




void MainWindow::SendWavFrame()
{
    QByteArray baDataForSend;

    if(nStateSendTestFile==STATE_RUNNING_FILE_SEND)
    {
        //Проверяем, подключено ли устройство
        if(!RadioDevice->isConnected())
        {
            ui->SLIPFileTxStatus_label->setText("Данные не могут быть переданы. Устройство не подключено");
            nStateSendTestFile=STATE_IDLE_FILE_SEND;
            return;
        }

        if(lSizeTestFileRestToSend)
        {
            if(lSizeTestFileRestToSend>sizeof(qint16)*SIZE_OF_WAV_FRAME_SAMPLES)
                nSLIPPayloadSize = sizeof(qint16)*SIZE_OF_WAV_FRAME_SAMPLES;
            else
                nSLIPPayloadSize = (quint16)lSizeTestFileRestToSend;
            memcpy(pSLIPPayloadData, baDataTestFile.data(), nSLIPPayloadSize);
            baDataTestFile.remove(0,nSLIPPayloadSize);

            lSizeTestFileRestToSend-=nSLIPPayloadSize;
            ui->TestFileSend_progressBar->setValue(100*(lSizeTestFile-lSizeTestFileRestToSend)/lSizeTestFile);

            memset(pSLIPPackData,0,MAX_SIZE_OF_SLIP_PACK);

            //Формируем SLIP-пакет
            SLIPInterface::FormPack((uint8_t*)pSLIPPayloadData, (uint16_t)nSLIPPayloadSize, (uint8_t*)pSLIPPackData, (uint16_t&)nSLIPPackSize, 256);

            baDataForSend.append((const char*)pSLIPPackData, nSLIPPackSize);

            #ifdef DEBUG_LOG_SIZE_OF_UART_TX_PACK
            qDebug() << "Передаем в порт SLIP-пакет размером" << nSLIPPackSize << "байт";
            qDebug() << "Размер полезной нагрузки SLIP пакета:" << nSLIPPayloadSize << "байт";
            #endif

            //Передаем данные в порт
            if(RadioDevice->SendDataToPort(baDataForSend))
            {
                ui->SLIPFileTxStatus_label->setText("Данные не могут быть переданы. Ошибка обмена с устройством");
                SetTestFileSendProgressBarFail();
            }
        }
        else
        {
            nStateSendTestFile=STATE_END_FILE_SEND;
            SetTestFileSendProgressBarSuccess();
        }
    }

}

void MainWindow::on_TestFileTransfer_Button_clicked()
{
    NameOfTestfFile = QFileDialog::getOpenFileName(this);
    QFileInfo finfo(NameOfTestfFile);
    QString fileName = finfo.fileName();

    lSizeTestFile = finfo.size();

    if (NameOfTestfFile == "")
    {
         ui->SLIPFileTxStatus_label->setText("Файл не выбран");
        return;
    }
    else
        TestFile.setFileName(NameOfTestfFile);

    ui->TestFileTransfer_lineEdit->setText(NameOfTestfFile);
}



void MainWindow::SetTestFileSendProgressBarFail()
{
    QPalette pal = ui->TestFileSend_progressBar->palette();
    pal.setColor(QPalette::Highlight, QColor("red"));
    ui->TestFileSend_progressBar->setPalette(pal);
    ui->TestFileSend_progressBar->repaint();
}

void MainWindow::SetTestFileSendProgressBarSuccess()
{
    ui->TestFileSend_progressBar->setValue(100);
    QPalette pal = ui->TestFileSend_progressBar->palette();
    pal.setColor(QPalette::Highlight, QColor("green"));
    ui->TestFileSend_progressBar->setPalette(pal);
    ui->TestFileSend_progressBar->repaint();
}


void MainWindow::on_SendSPIMData_Button_clicked()
{
    bool ok;

    uint8_t pBodyData[SPIMMessage::MAX_SIZE_OF_BODY];   //Данные тела сообщения SPIM
    uint16_t nBodySize;         //Размер тела сообщения SPIM

    //Читаем строку, из которой надо сформировать тело сообщения
    //Строка должна содержать только hex-символы ('0'-'9','A'-'F','a'-'f')
    if(!ConvertUTF8StringToHexInt(ui->SPIM_DATA_lineEdit->text(), pBodyData, nBodySize))
    {
        ui->SPIMTxStatus_label->setText("Неверный формат данных! Строка должна состоять из четного числа символов\n и содержать только hex-символы (0-9,A-F,a-f)");
        return;
    }
    else
        ui->SPIMTxStatus_label->setText("");

    if(nBodySize > SPIMMessage::MAX_SIZE_OF_BODY)
    {
        ui->SPIMTxStatus_label->setText("Размер тела сообщения превышает максимально допустимое знаение");
        return;
    }

    //Отображаем для пользователя значение поля LEN, соответствующее размеру тела сообщения SPIM
     ui->SPIM_LEN_lineEdit->setText(QString::number(nBodySize,16).toUpper());

    //Определяем адресата сообщения SPIM
    uint8_t nAddress = ui->SPIM_ADR_lineEdit->text().toInt(&ok, 16);
    if(!ok)
    {
        ui->SPIMTxStatus_label->setText("Некорректный формат поля ADR");
        return;
    }

    //Определяем порядковый номер сообщения SPIM
    uint8_t nNoMsg = ui->SPIM_NO_lineEdit->text().toInt(&ok, 16);
    if(!ok)
    {
        ui->SPIMTxStatus_label->setText("Некорректный формат поля NO");
        return;
    }

    //Определяем идентификатор команды сообщения SPIM
    uint8_t nIDcmd = ui->SPIM_ID_lineEdit->text().toInt(&ok, 16);
    if(!ok)
    {
        ui->SPIMTxStatus_label->setText("Некорректный формат поля ID");
        return;
    }

    //Составляем заголовок сообщения
    objSPIMmsgTx->setHeader(nBodySize,nAddress,nNoMsg,nIDcmd);

    //Вставляем в сообщение тело
    objSPIMmsgTx->setBody(pBodyData,nBodySize);

    //Расчитываем и вставляем в сообщение контрольную сумму
    objSPIMmsgTx->setCRC();

    uint8_t nCRC = objSPIMmsgTx->getCRC();
    ui->SPIM_CRC_lineEdit->setText(QString::number(nCRC,16).toUpper());


    //Проверяем, подключено ли устройство
    if(!RadioDevice->isConnected())
    {
        ui->SPIMTxStatus_label->setText("Данные не могут быть переданы. Устройство не подключено");
        return;
    }

    memset(pSLIPPackData,0,MAX_SIZE_OF_SLIP_PACK);

    //Формируем SLIP-пакет
    SLIPInterface::FormPack((uint8_t*)objSPIMmsgTx->Data, (uint16_t)objSPIMmsgTx->Size, (uint8_t*)pSLIPPackData, (uint16_t&)nSLIPPackSize, 256);

    QByteArray baDataForSend;
    baDataForSend.append((const char*)pSLIPPackData, nSLIPPackSize);

    //Передаем данные в порт
    RadioDevice->SendDataToPort(baDataForSend);


}

/**
   * @brief  Обработка нажатия на кнопку "..." выбора файла для передачи
   */
void MainWindow::on_FileTransfer_Button_clicked()
{
    QString NameOfFileForSend = QFileDialog::getOpenFileName(this);

    if(NameOfFileForSend == "")
    {
         ShowFileTxError("Файл не выбран");
        return;
    }

    ui->FileTransfer_lineEdit->setText(NameOfFileForSend);

    if(!FileTransmitter.SetFileForSend(NameOfFileForSend))
    {
        switch(FileTransmitter.GetErrorStatus())
        {
            case QFileTransfer::ERROR_FILE_OPEN:
                ShowFileTxError("Файл не может быть передан. Ошибка открытия файла");
                break;
            case QFileTransfer::ERROR_FILE_SIZE_IS_TOO_BIG:
                ShowFileTxError("Файл не может быть передан. Превышен максимальный размер файла");
                break;
        }
    }
    else
       ClearFileTxError();
}


/**
   * @brief  Обработка нажатия на кнопку "Передать файл"
   */
void MainWindow::on_SendFile_Button_clicked()
{
    switch(FileTransmitter.GetTransferStatus())
    {
        case QFileTransfer::FILE_IS_READY_FOR_SEND:
            ClearFileTxError();
            //Стартуем таймер, контролирующий процесс передачи файла
            InitFileSendTimer();
            break;
        case QFileTransfer::FILE_IS_IN_SEND_PROCESS:
            ShowFileTxError("Этот файл уже передается");
            break;
        case QFileTransfer::FILE_IS_SENDED:
            FileTransmitter.ResetFileForSend();

            ClearFileTxError();
            //Стартуем таймер, контролирующий процесс передачи файла
            InitFileSendTimer();
            break;
        case QFileTransfer::FILE_SEND_DENY:
            ShowFileSendErrorStatus();
            break;
    }

}


void MainWindow::InitFileSendTimer()
{
    if(!timerFileSend)
    {
        timerFileSend = new QTimer(this);
        connect(timerFileSend,SIGNAL(timeout()),this,SLOT(CheckFileTransferStatus()));
        timerFileSend->setInterval(PERIOD_MS_CHECK_FILE_SEND_STATUS);
    }

    timerFileSend->start();

    return;
}

void MainWindow::DeinitFileSendTimer()
{
    timerFileSend->stop();
    delete timerFileSend;
    timerFileSend = NULL;
}


void MainWindow::CheckFileTransferStatus()
{
    //Если файл передан, убиваем таймер, рисуем зеленый ProgressBar
    if(FileTransmitter.GetTransferStatus()==QFileTransfer::FILE_IS_SENDED)
    {
        DeinitFileSendTimer();
        ShowFileTxError("Данные успешно переданы");
        SetFileSendProgressBarSuccess();
        return;
    }

    //Если возникло состояние критической ошибки интерфейса радиомодуля, то прекращаем передачу, убиваем таймер,
    //рисуем красный ProgressBar
    if(FileTransmitter.GetTransferStatus()==QFileTransfer::FILE_SEND_DENY)
    {
        DeinitFileSendTimer();
        ShowFileSendErrorStatus();
        SetFileSendProgressBarFail();
        return;
    }

#ifdef CHECK_TRANSMITTER_BUSY_IN_FILE_SENDING
    //Если передатчик занят, ожидаем освобождения и возможности передать следующий пакет
    if(FileTransmitter.GetTransmitterStatus()==QFileTransfer::TRANSMITTER_BUSY)
    {

        //Если очень долго не меняется состояние радимодуля, то прекращаем передачу, убиваем таймер,
        //рисуем красный ProgressBar
        if(timeTransmitterBusyMs > QFileTransfer::MAX_TIME_TRANSMITTER_BUSY_MS)
        {
            DeinitFileSendTimer();
            ShowFileTxError("Данные не могут быть переданы. Передатчик слишком долго не отвечает");
            SetFileSendProgressBarFail();
            FileTransmitter.ResetFileForSend();
        }
        else
            timeTransmitterBusyMs+=PERIOD_MS_CHECK_FILE_SEND_STATUS;

        return;
    }

    timeTransmitterBusyMs = 0;
#endif

    //Поскольку передатчик освободился, значит, пакет был передан, изменился размер переданных данных - стоит
    //перерисовать ProgressBar
    ui->FileSend_progressBar->setValue(100 * FileTransmitter.GetSizeOfSendedFileData() / FileTransmitter.GetFileSize());

    //Радимодуль свободен, можно ему послать следующий пакет данных для передачи в эфир
    SendFilePackToTransceiver();

    return;
}


void MainWindow::SendFilePackToTransceiver()
{
    emit signalSendNextFilePack(RadioDevice);

    FileTransmitter.SetTransmitterStatus(QFileTransfer::TRANSMITTER_BUSY);
}

/**
   * @brief  Вывод в пользовательское окно ошибки передачи файла
   */
void MainWindow::ShowFileTxError(QString strError)
{
    ui->FileTxStatus_label->setText(strError);
}

/**
   * @brief  Очистка в пользовательском окне области ошибки передачи файла
   */
void MainWindow::ClearFileTxError()
{
    ui->FileTxStatus_label->setText("");
}

/**
   * @brief  Изменение состояния ProgressBar на состояние ошибки передачи
   * (окрашивание статусной полоски в красный цвет)
   */
void MainWindow::SetFileSendProgressBarFail()
{
    QPalette pal = ui->FileSend_progressBar->palette();
    pal.setColor(QPalette::Highlight, QColor("red"));
    ui->FileSend_progressBar->setPalette(pal);
    ui->FileSend_progressBar->repaint();
}

/**
   * @brief  Изменение состояния ProgressBar на состояние успешной передачи
   * (окрашивание статусной полоски в зеленый цвет)
   */
void MainWindow::SetFileSendProgressBarSuccess()
{
    ui->FileSend_progressBar->setValue(100);
    QPalette pal = ui->FileSend_progressBar->palette();
    pal.setColor(QPalette::Highlight, QColor("green"));
    ui->FileSend_progressBar->setPalette(pal);
    ui->FileSend_progressBar->repaint();
}

/**
   * @brief  Отображение ошибки, которая произошла в процессе передачи файла
   */
void MainWindow::ShowFileSendErrorStatus()
{
    switch(FileTransmitter.GetErrorStatus())
    {
        case QFileTransfer::ERROR_DEVICE_CONNECTION:
            ShowFileTxError("Данные не могут быть переданы. Устройство не подключено");
            break;
        case QFileTransfer::ERROR_FILE_IS_NOT_CHOSEN:
            ShowFileTxError("Данные не могут быть переданы. Не выбран файл");
            break;
        case QFileTransfer::ERROR_NONE:
            break;
        default:
            ShowFileTxError("Данные не могут быть переданы. Неизвестная ошибка");
    }
}
