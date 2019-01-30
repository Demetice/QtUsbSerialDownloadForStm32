#include "mydialog.h"
#include "ui_mydialog.h"
#include "log.h"
#include <QMessageBox>

MyDialog::MyDialog(QWidget *parent, QString &fileName, QString serial) :
    QDialog(parent),
    ui(new Ui::MyDialog)
{
    ui->setupUi(this);
    //setModal(true);
    this->fileName = fileName;
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);

    UpThread = new UpDateThread(fileName, serial);

    QObject::connect(UpThread, SIGNAL(valueChange(int)), this, SLOT(setValue(int)));
    QObject::connect(UpThread, SIGNAL(RangeChange(int)), this, SLOT(setRange(int)));
    QObject::connect(UpThread, SIGNAL(exitUpdate(void)), this, SLOT(accept()));
    QObject::connect(UpThread, SIGNAL(exitError(void)), this, SLOT(reject()));

    UpThread->start();
}

MyDialog::~MyDialog()
{
    LOG_D<<"destory my update dialog";
    UpThread->exit(0);
    UpThread->wait();
    delete ui;
    delete UpThread;
}

void MyDialog::closeEvent(QCloseEvent *event)
{
    LOG_D<<"Close update dialog event";
    event->ignore();
}

void UpDateThread::ReadData()
{
    QByteArray buf;
    buf = serial.readAll();
    if(!buf.isEmpty())
    {
        LOG_D<<tr(buf);
    }
    buf.clear();
}

void UpDateThread::run()
{
    LOG_D<<"run update dialog";

    QSerialPort serial(comName);
    if (false == serial.open(QIODevice::ReadWrite))
    {
        LOG_D<<"Can't open device";
        emit exitError();
        return;
    }

    serial.setBaudRate(115200);//设置波特率为115200
    serial.setDataBits(QSerialPort::Data8);//设置数据位8
    serial.setParity(QSerialPort::NoParity);
    serial.setStopBits(QSerialPort::OneStop);//停止位设置为1
    serial.setFlowControl(QSerialPort::NoFlowControl);//设置为无流控制

    //QObject::connect(&serial,&QSerialPort::readyRead,this,&UpDateThread::ReadData);

    bool ret = false;
    QByteArray btData;
    QFile fwFile(fileName);

    ret = fwFile.open(QIODevice::ReadOnly);
    if (false == ret)
    {
        LOG_D<<"open file error";
        emit exitError();
        return;
    }

    LOG_D<<"file size"<<fwFile.size();

    btData = fwFile.readAll();
    fwFile.close();

    char cmd[3] = {0x31, 0x0d, 0x0a};
    qint64 wrt = serial.write(cmd, sizeof(cmd));
    if (wrt < 0)
    {
        LOG_D<<"Error in write serial";
        emit exitError();
    }
    serial.flush();
    serial.clear();
    serial.clearError();

    msleep(2000);
    serial.write("12345678");
    char *buf = (char *)btData.data();

    int range = btData.size();
    RangeChange(range);

//    for (int i = 0; i < range; i += 64)
//    {
//        int size = 64;
//        if (range - i < 64)
//        {
//            size = range - i;
//        }

//        emit valueChange(i);
//        wrt = serial.write(buf+i, size);
//        if (wrt < 0)
//        {
//            LOG_D<<"Error in write serial";
//            emit exitError();
//        }
//        if (serial.flush())
//        {
//            LOG_D<<"Serial flush success";
//        }
//        msleep(500);
//    }


    cmd[0] = 0x32;
    serial.write(cmd, sizeof(cmd));
    serial.flush();
    msleep(500);

    emit valueChange(range);

    emit exitUpdate();
}

void UpDateThread::setValue( int i)
{
    emit valueChange(i);
}
void UpDateThread::setRange( int i)
{
    emit RangeChange(i);
}

UpDateThread::UpDateThread(QString &name, QString &comName)
{
    val = 0;
    fileName = name;
    this->comName = comName;
}

UpDateThread::~UpDateThread()
{

}

void MyDialog::setValue(int i)
{
    ui->progressBar->setValue(i);
}

void MyDialog::setRange(int i)
{
    ui->progressBar->setRange(0, i);
}
