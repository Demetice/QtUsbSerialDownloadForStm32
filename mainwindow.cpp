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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
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
    ulNum = 0;
    downloading = false;
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
    StartDownload();
    timer->start(100);
    ulNum = 0;
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

    if (downloading
            && cmd[0] == 'c'
            && cmd[1] == 0xd
            && cmd[2] == 0xa)
    {
        timer->start(5);
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

void MainWindow::StartDownload(void)
{
    char buf[3] = {0x31, 0xd, 0xa};
    serial->write(buf, 3);
    downloading = true;
}

void MainWindow::EndDownload(void)
{
    char buf[3] = {0x32, 0xd, 0xa};
    serial->write(buf, 3);
    downloading = false;
}

void MainWindow::timerTransDate()
{
    int writeNum = 64;

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

    ulNum += writeNum;
    ui->progressBar->setValue(ulNum);
    serial->write(binLitByte, writeNum);
    timer->stop();
}
