#ifndef SETTINGFILES_H
#define SETTINGFILES_H
#include <QFile>
#include <QDebug>
#include <QMessageLogger>

#define LOG_D qDebug()<<"[LINE"<<__LINE__<<",FUNC"<<__FUNCTION__<<"]"
#define LOG_W qWarning()<<"[LINE"<<__LINE__<<",FUNC"<<__FUNCTION__<<"]"
#define LOG_E qCritical()<<"[LINE"<<__LINE__<<",FUNC"<<__FUNCTION__<<"]"

void MsgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
void SaveLog();

#endif // SETTINGFILES_H
