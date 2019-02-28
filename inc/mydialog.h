#ifndef MYDIALOG_H
#define MYDIALOG_H

#include <QDialog>
#include <QThread>
#include <QCloseEvent>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>

namespace Ui {
class MyDialog;
}

class UpDateThread : public QThread
{
    Q_OBJECT

public:
    explicit UpDateThread(QString &name, QString &comName);
    ~UpDateThread();
    void run();

signals:
    void valueChange(int);
    void RangeChange(int);
    void exitUpdate(void);
    void exitError(void);
private slots:
    void setValue( int );
    void setRange( int );
    void ReadData();
private:
    int val;
    QString fileName;
    QString comName;
    QSerialPort serial;
};

class MyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MyDialog(QWidget *parent, QString &fileNam, QString serial);
    ~MyDialog();
    void run();

private slots:
    void setValue( int );
    void setRange( int );

private:
    void closeEvent(QCloseEvent *event);

    Ui::MyDialog *ui;
    UpDateThread *UpThread;
    QString fileName;
};


#endif // MYDIALOG_H
