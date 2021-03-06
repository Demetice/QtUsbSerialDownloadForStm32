#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QString>
#include <QDebug>
#include <QFile>
#include <QDialog>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include "mydialog.h"
#include "log.h"
#include "crc32.h"
#include "iapcmd.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    downloading(E_DOWNLOAD_STATUS_NONE),
    ulNum(0),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    foreach (const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
    {
        QSerialPort serial;
        serial.setPort(info);
        if(serial.open(QIODevice::ReadWrite))
        {
            ui->comboBox->addItem(serial.portName());
            serial.close();
        }
    }

    serial = new QSerialPort(this);

    connect(this, SIGNAL(exitUpdate()), this, SLOT(EndDownload()));
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    connect(serial, static_cast<void (QSerialPort::*)(QSerialPort::SerialPortError)>(&QSerialPort::error),
                this, &MainWindow::handleError);

    timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timerTransDate()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool isFileExist(QString &fullFileName)
{
    QFileInfo fileInfo(fullFileName);
    return fileInfo.isFile();
}

void MainWindow::on_btnChooseFile_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select file"), "", "*.bin , *.hex");
    ui->lineEdit->setText(fileName);

    bool ret = isFileExist(fileName);
    if (ret == false) return;
    ui->btnDownload->setEnabled(true);

    QFile fwFile(fileName);

    ret = fwFile.open(QIODevice::ReadOnly);
    if (false == ret)
    {
        LOG_D<<"open file error";
        return;
    }

    LOG_D<<"file size"<<fwFile.size();

    btData = fwFile.readAll();
    ui->progressBar->setRange(0, btData.size());
    fwFile.close();
}

void MainWindow::setProgressValue(int value)
{
    ui->progressBar->setValue(value);
}

void MainWindow::setProgressRange(int value)
{
    ui->progressBar->setRange(0, value);
}

void MainWindow::on_btnDownload_clicked()
{
    ResetMCU();

    ui->btnChooseFile->setEnabled(false);
    ui->btnDownload->setEnabled(false);
    ui->comboBox->setEnabled(false);
    ui->btnConnect->setEnabled(false);
}

void MainWindow::openSerialPort()
{
    serial->setPortName(ui->comboBox->currentText());
    serial->setBaudRate(QSerialPort::Baud115200);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    if (serial->open(QIODevice::ReadWrite)) {
        ui->comboBox->setEnabled(false);
        ui->btnChooseFile->setEnabled(true);
        ui->btnDownload->setEnabled(false);
        ui->btnConnect->setText(tr("断开"));
        LOG_D<<tr("Connected to");
    } else {
        QMessageBox::critical(this, tr("Error"), serial->errorString());

        LOG_D<<(tr("Open error"));
    }
}

void MainWindow::closeSerialPort()
{
    if (serial->isOpen())
        serial->close();

    ui->btnConnect->setText(tr("连接"));
    ui->comboBox->setEnabled(true);
    ui->btnChooseFile->setEnabled(false);
    ui->btnDownload->setEnabled(false);
    LOG_D<<(tr("Disconnected"));
}

void MainWindow::writeData(const QByteArray &data)
{
    serial->write(data);
}

void MainWindow::readData()
{
    QByteArray temp = serial->readAll();
    QString buf;
    char *cmd = temp.data();

    if (downloading == E_DOWNLOAD_STATUS_DOWNLOADING
            && cmd[0] == 'c')
    {
        timer->start(5);
        return;
    }

    if (downloading == E_DOWNLOAD_STATUS_PREPARE && temp.size() > 12)
    {
        //have msg Bootload start.
        if(cmd[0] == 'B' && cmd[1] == 'o'
           && cmd[9] == 's')
        {
            StartDownload();
        }
    }

    if(!temp.isEmpty()){
        if(ui->checkBox->isChecked())
        {
            QString s(temp);
            buf = s;
        }
        else
        {
            for(int i = 0; i < temp.count(); i++){
                QString s;
                s.sprintf("%02X ", (unsigned char)temp.at(i));
                buf += s;
            }
        }

        ui->textBrowser->append(buf);
        ui->textBrowser->moveCursor(QTextCursor::End);
    }
}

void MainWindow::handleError(QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        QMessageBox::critical(this, tr("Critical Error"), serial->errorString());
        closeSerialPort();
    }
}


void MainWindow::on_btnConnect_clicked()
{
    if (serial->isOpen())
    {
        serial->close();
        ui->btnConnect->setText(tr("连接"));
        ui->comboBox->setEnabled(true);
        ui->btnChooseFile->setEnabled(false);
        ui->btnDownload->setEnabled(false);
    }
    else
    {
        openSerialPort();
        ui->btnConnect->setText(tr("断开"));
    }
}

void MainWindow::ResetMCU(void)
{
    char buf[256] = {0};
    int len = 0;

    iapCmd iap;

    iap.ConstructResetPkt(buf, &len);

    qDebug("Sent commond to reset MCU");

    serial->write(buf, len);

    downloading = E_DOWNLOAD_STATUS_PREPARE;
}

void MainWindow::StartDownload(void)
{
    char buf[256] = {0};
    int len = 0;
    unsigned long crc32 = 0;

    iapCmd iap;

    crc32 = CalcCrc32(0xffffffff, btData.size(), (uint8_t *)btData.data());

    iap.ConstructStartPkt(buf, &len, btData.size(), crc32);

    qDebug("crc32 : %#x, file size :%u start pkg len : %d", crc32, btData.size(), len);

    serial->write(buf, len);

    downloading = E_DOWNLOAD_STATUS_DOWNLOADING;

    timer->start(100);
    ulNum = 0;
}

void MainWindow::EndDownload(void)
{
    char buf[256];
    int len = 0;

    iapCmd iap;

    iap.ConstructEndPkt(buf, &len);

    serial->write(buf, len);
    downloading = E_DOWNLOAD_STATUS_COMPLETE;
}

void MainWindow::timerTransDate()
{
    int writeNum = 64;
    char buf[256];
    int len = 0;
    iapCmd iap;



    if (ulNum >= btData.size())
    {
        emit exitUpdate();
        timer->stop();
        ui->btnChooseFile->setEnabled(true);
        ui->btnDownload->setEnabled(true);
        ui->comboBox->setEnabled(true);
        ui->btnConnect->setEnabled(true);
        ui->progressBar->setValue(btData.size());
        return;
    }

    char * binLitByte = btData.data() + ulNum;//bin缓存

    if (ulNum + 64 > btData.size())
    {
        writeNum = btData.size()- ulNum;
    }

    iap.packet(binLitByte, writeNum, &len, buf);

    ulNum += writeNum;
    ui->progressBar->setValue(ulNum);
    serial->write(buf, len);
    timer->stop();

    timer->start(1000);

}

void MainWindow::on_btnClear_clicked()
{
    ui->textBrowser->clear();
}
