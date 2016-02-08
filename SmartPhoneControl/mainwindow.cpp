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

    //Объект QTimer для периодического контроля подключения/отключения устройств
    timerPollingCOMDevices = new QTimer(this);
    connect(timerPollingCOMDevices,SIGNAL(timeout()),this,SLOT(CheckCOMDevices()));
    timerPollingCOMDevices->setInterval(PERIOD_MS_POLLING_COM_DEVICES);
    timerPollingCOMDevices->start();

    // Присоединяем к сигналу успешного подключения устройства слот запроса ID устройства
    connect(RadioDevice, SIGNAL(signalConnectDevice()), this, SLOT(RequestR181GenParams()));

    // Обработка данных, принятых от устройства
    connect(RadioDevice, SIGNAL(signalReceiveData(QByteArray)),this, SLOT(ReceiveDataFromRadioModule(QByteArray)));
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
//-------------------------------------------------------------------------------------------------------------
void MainWindow::ShowAvailPortsInMenu()
{
    QAction* actDevice;

    mnConnectDeviceSubMenu->clear();

    //Для каждого найденного в системе устройства
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        // Создаем подпункт в меню "Подключить"
        // При если идентифиактор продукта и производителя совпадет с идентификаторами, используемыми
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
// Функция отображает все COM-устройства, которые есть в системе, в поле textBrowser тестовой вкладки
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
