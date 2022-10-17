#ifndef CLIENT_H
#define CLIENT_H

#include <QTreeWidgetItem>

class Client : public QTreeWidgetItem
{
public:
    explicit Client(int = 0, QString = "", QString = "", QString = "", QString = "", QString = "");

    QString getUserID() const;
    QString getName() const;
    QString getPhoneNumber() const;
    QString getAddress() const;
    QString get_Gender() const;

    void setUserID(QString&);
    void setName(QString&);
    void setPhoneNumber(QString&);
    void setAddress(QString&);
    void setGender(QString&);

    int userCount() const;
    bool operator==(const Client &other) const;
};

#endif // CLIENT_H
