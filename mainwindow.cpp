#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serialportlist.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->serialPort = new QSerialPort;

    addQComboBox();

    connect(ui->openSerialButton, SIGNAL(clicked()), this, SLOT(openSerialPort()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearWindows()));
    connect(ui->abnormalButton, SIGNAL(currentIndexChanged(int)), this, SLOT(comboxChangeHandle(int)));
    connect(ui->abnDataButton, SIGNAL(currentIndexChanged(int)), this, SLOT(comboxChangeHandle(int)));

    // 延时更新串口
    SerialPortList *portList = new SerialPortList(200);
    connect(portList, SIGNAL(onNewSerialPort(QStringList)),
            this, SLOT(onNewPortList(QStringList)));
    portList->ScanStart();

    connect(this->serialPort, SIGNAL(readyRead()), this, SLOT(recvMsg()));
    connect(ui->btnSend, &QPushButton::clicked, [=](){
        sendMsg(ui->message->toPlainText());
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addQComboBox() {

    QStringList *strList = new QStringList;
    strList->append("选择类型");
    strList->append("#1");
    strList->append("#2");
    strList->append("#3");
    strList->append("#4");
    ui->abnDataButton->addItems(*strList);

    strList = new QStringList;
    strList->append("选择site");
    for (int i = 1; i <= 30; i++) {
        strList->append(QString::number(i));
    }
    ui->abnormalButton->addItems(*strList);
}

void MainWindow::comboxChangeHandle(int index) {
    int originNo = 60;

    ui->message->clear();

    if ((ui->abnDataButton->currentIndex() == 0) || (ui->abnormalButton->currentIndex() == 0)) {
        return;
    }

    originNo = ((ui->abnDataButton->currentIndex()-13)*10) + ui->abnormalButton->currentIndex() + originNo;

    ui->message->insertPlainText("@" + QString::number(originNo) + "+");
}

void MainWindow::clearWindows() {
    ui->comLog->clear();
}

void MainWindow::openSerialPort() {

    if (ui->portName->currentText() == "") {
        ui->openSerialButton->setText("打开串口");
        return;
    }

    if (ui->openSerialButton->text() == tr("打开串口")) {
        if (!initSerialPort()) {
            return;
        }
        ui->btnSend->setEnabled(true);
        ui->baudRate->setEnabled(false);
        ui->portName->setEnabled(false);
        ui->dataBits->setEnabled(false);
        ui->stopBits->setEnabled(false);
        ui->parity->setEnabled(false);
        ui->openSerialButton->setText("关闭串口");
    } else {
        this->serialPort->close();
        ui->btnSend->setEnabled(false);

        ui->baudRate->setEnabled(true);
        ui->portName->setEnabled(true);
        ui->dataBits->setEnabled(true);
        ui->stopBits->setEnabled(true);
        ui->parity->setEnabled(true);

        ui->openSerialButton->setText("打开串口");
    }
}


void MainWindow::onNewPortList(QStringList portName) {
    ui->portName->clear();
    ui->portName->addItems(portName);
}

//初始化串口
bool MainWindow::initSerialPort() {
    this->serialPort->setPortName(ui->portName->currentText());
    if (!this->serialPort->open(QIODevice::ReadWrite)){
        QMessageBox::warning(nullptr, "error", QStringLiteral("串口打开失败"));
        return false;
    }
    this->serialPort->setBaudRate(ui->baudRate->currentText().toInt());

    if (ui->dataBits->currentText().toInt() == 8){
        this->serialPort->setDataBits(QSerialPort::Data8);
    }else if (ui->dataBits->currentText().toInt() == 7){
        this->serialPort->setDataBits(QSerialPort::Data7);
    }else if (ui->dataBits->currentText().toInt() == 6){
        this->serialPort->setDataBits(QSerialPort::Data6);
    }else if (ui->dataBits->currentText().toInt() == 5){
        this->serialPort->setDataBits(QSerialPort::Data5);
    }

    if (ui->stopBits->currentText().toInt() == 1){
        this->serialPort->setStopBits(QSerialPort::OneStop);
    }else if (ui->stopBits->currentText().toInt() == 2){
        this->serialPort->setStopBits(QSerialPort::TwoStop);
    }


    if(ui->parity->currentText() == "NoParity"){
        this->serialPort->setParity(QSerialPort::NoParity);
    }else if (ui->parity->currentText() == "EvenParity"){
        this->serialPort->setParity(QSerialPort::EvenParity);
    }else if (ui->parity->currentText() == "OddParity"){
        this->serialPort->setParity(QSerialPort::OddParity);
    }
    return true;
}

void MainWindow::Sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

//向串口发送信息
void MainWindow::sendMsg(const QString &msg){
    this->serialPort->write(msg.toLatin1());
    ui->comLog->insertPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " [send] " + msg + "\n");
}

//接受来自串口的信息
void MainWindow::recvMsg(){

    // 无法读满一帧数据，加入延时，尽量读满一帧数据
    Sleep(10);

    QByteArray msg = this->serialPort->readAll();

    if (msg.size() == 0) {
        return;
    }

    //do something
    ui->comLog->insertPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " [recieve] " + msg.data() + "\n");

    QString backData;
    QString str = msg;
    QRegExp rx("[@](\\d{3,4})[+]");

    if (str.indexOf(rx) >= 0) {

        int data = rx.cap(1).toInt();
        if ((data >= 101) && (data <= 160)) {
            int num = (data / 100) + 1000;
            backData = "@" + QString::number(num) + "+";
            ui->comLog->insertPlainText(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + " [write data] " + backData + "\n");
            this->serialPort->write(backData.toLatin1());
        }
    }

    // 跟随数据进行滚动
    QTextCursor cursor=ui->comLog->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->comLog->setTextCursor(cursor);
}
