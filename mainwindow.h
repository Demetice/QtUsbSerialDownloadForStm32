#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QTimer>
#include "mydialog.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void writeData(const QByteArray &data);
    void closeSerialPort(void);
    void handleError(QSerialPort::SerialPortError error);
    void openSerialPort();
    void StartDownload(void);

signals:
    void exitUpdate(void);

private slots:

    void on_btnChooseFile_clicked();

    void on_btnDownload_clicked();

    void setProgressValue(int value);

    void setProgressRange(int value);
    void readData();
    void timerTransDate();
    void on_btnConnect_clicked();
    void EndDownload(void);

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    UpDateThread *UpThread;
    QTimer *timer;
    qint32 ulNum;
    bool downloading;
    QByteArray btData;
};

#endif // MAINWINDOW_H
