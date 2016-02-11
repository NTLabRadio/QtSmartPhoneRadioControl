#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("SmartPhoneControl ver 1.0");

    RadioDevice = new QSmartRadioModuleControl;

    //Создание меню верхнего уровня
    CreateMenuBar();

    //Инициализируем процесс периодического контроля подключения/отключения COM-устройств
    InitPollingCOMDevices();

    // Присоединяем к сигналу успешного подключения устройства слот запроса ID устройства
    connect(RadioDevice, SIGNAL(signalConnectDevice()), this, SLOT(RequestR181GenParams()));

    // Обработка данных, принятых от устройства
    connect(RadioDevice, SIGNAL(signalReceiveData(QByteArray)),this, SLOT(ReceiveDataFromRadioModule(QByteArray)));

    //Создаем объект для работы с SLIP-интерфейсом
    objSLIPInterface = new SLIPInterface;

    ui->SLIPPayloadFormatError_label->setText("");
}

MainWindow::~MainWindow()
{
    delete RadioDevice;
    delete timerPollingCOMDevices;
    delete mnConnectDeviceSubMenu;
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

#define MAX_SIZE_OF_SLIP_PAYLOAD    (128+4)
quint8 pSLIPPayloadData[128+4];
quint16 nSLIPPayloadSize = 0;

#define MAX_SIZE_OF_SLIP_PACK   (2*MAX_SIZE_OF_SLIP_PAYLOAD)
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


bool MainWindow::ConvertHexIntToUTF8(quint8* pBufData, quint16 nSizeData)
{
    qint16 cntBytes;
    quint8 nByteHighValue, nByteLowValue;

    nSizeData*=2;

    for(cntBytes=nSizeData/2-1; cntBytes>=0; cntBytes--)
    {
        #ifdef QDEBUG_CONVERT_HEX_UTF8
        qDebug() << "pBufData[" << cntBytes << "] =" << pBufData[cntBytes];
        #endif

        nByteHighValue = (pBufData[cntBytes]&0xF0)>>4;
        nByteLowValue = pBufData[cntBytes]&0x0F;

        #ifdef QDEBUG_CONVERT_HEX_UTF8
        qDebug() << "hex[" << cntBytes << "]&0xF0>>4 =" << nByteLowValue;
        qDebug() << "hex[" << cntBytes << "]*0x0F  =" << nByteHighValue;
        #endif

        if(nByteLowValue <= 0x9)
            pBufData[2*cntBytes+1] = nByteLowValue + 0x30;
        else if((nByteLowValue >= 0xA) && (nByteLowValue <= 0xF))
            pBufData[2*cntBytes+1] = nByteLowValue + 0x37;
        else
            return 0;

        #ifdef QDEBUG_CONVERT_HEX_UTF8
        qDebug() << "char[" << 2*cntBytes+1 << "] =" << pBufData[2*cntBytes+1];
        #endif

        if(nByteHighValue <= 0x9)
            pBufData[2*cntBytes] = nByteHighValue + 0x30;
        else if((nByteHighValue >= 0xA) && (nByteHighValue <= 0xF))
            pBufData[2*cntBytes] = nByteHighValue + 0x37;
        else
            return 0;

        #ifdef QDEBUG_CONVERT_HEX_UTF8
        qDebug() << "char[" << 2*cntBytes << "] =" << pBufData[2*cntBytes];
        #endif
    }

    return 1;
}

void MainWindow::on_pushButton_clicked()
{
    QByteArray baDataForSend;

    //Определяем размер полезных данных для передачи
    nSLIPPayloadSize = ui->SLIPPayload_lineEdit->text().length();

    //Читаем строку, из которой надо сформировать SLIP-пакет, записываем в pSLIPPayloadData в UTF8-формате
    //Строка должна содержать только hex-символы ('0'-'9','A'-'F','a'-'f')
    memcpy(pSLIPPayloadData, ui->SLIPPayload_lineEdit->text().toUtf8().data(), nSLIPPayloadSize);

    //Преобразуем строку в масcив чисел 0-F
    if(!ConvertUTF8ToHexInt(pSLIPPayloadData,nSLIPPayloadSize))
    {
        //Ошибка при разборе сообщения для передачи. Выходим из функции, сообщаем пользователю об ошибке
        ui->SLIPPayloadFormatError_label->setText("Неверный формат данных! Строка должна состоять из четного числа символов\n и содержать только hex-символы (0-9,A-F,a-f)");
        return;
    }
    else
        ui->SLIPPayloadFormatError_label->setText("");

    memset(pSLIPPackData,0,MAX_SIZE_OF_SLIP_PACK);

    //Формируем SLIP-пакет
    objSLIPInterface->FormPack((uint8_t*)pSLIPPayloadData, (uint16_t)nSLIPPayloadSize/2, (uint8_t*)pSLIPPackData, (uint16_t&)nSLIPPackSize, 256);


    //Проверяем, подключено ли устройство
    if(!RadioDevice->isConnected())
    {
        ui->SLIPPayloadFormatError_label->setText("Данные не могут быть переданы. Устройство не подключено");
        return;
    }

    baDataForSend.append((const char*)pSLIPPackData, nSLIPPackSize);

    //Передаем данные в порт
    RadioDevice->SendDataToPort(baDataForSend);



    //Преобразуем массив чисел в строку из hex-символов ('0'-'9','A'-'F')
    ConvertHexIntToUTF8(pSLIPPackData,nSLIPPackSize);

    //Показываем пользователю данные, которые переданы в порт
    ui->SLIPPack_lineEdit->setText ((const char*)pSLIPPackData);


}
