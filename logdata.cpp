#include "logdata.h"

#include <QTreeWidgetItem>
#include <QFile>
#include <QDateTime>

LogData::LogData(QObject *parent)
    : QThread{parent}
{
    QString format = "yyyyMMdd_hhmmss";
    filename = QString("log_%1.txt").arg(QDateTime::currentDateTime().toString(format));
}

void LogData::run()
{
    Q_FOREVER {
        saveData();
        sleep(60);      // 1분마다 저장
    }
}

void LogData::appendData(QTreeWidgetItem* item)
{
    itemList.append(item);
}

void LogData::saveData()
{
    if(itemList.count() > 0) {
        QFile file(filename);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        QTextStream out(&file);
        foreach(auto item, itemList) {
            out << item->text(0) << ", ";
            out << item->text(1) << ", ";
            out << item->text(2) << ", ";
            out << item->text(3) << ", ";
            out << item->text(4) << ", ";
            out << item->text(5) << "\n";
        }
        file.close();
    }
}
