#ifndef LOGDATA_H
#define LOGDATA_H

#include <QThread>
#include <QList>

class QTreeWidgetItem;

class LogData : public QThread
{
    Q_OBJECT
public:
    explicit LogData(QObject *parent = nullptr);

private:
    void run();

    QList<QTreeWidgetItem*> itemList;
    QString filename;

public slots:
    void appendData(QTreeWidgetItem*);
    void saveData();

signals:
    void send(int data);
};

#endif // LOGDATA_H
