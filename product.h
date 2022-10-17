#ifndef PRODUCT_H
#define PRODUCT_H

#include <QTreeWidgetItem>

class Product : public QTreeWidgetItem
{
public:
    explicit Product(int = 0, QString = "", int = 0, int = 0, QString = "");

    int getProNumber() const;
    QString getProName() const;
    int getProPrice() const;
    int getProCount() const;
    QString getProType() const;

    void setProNumber(int&);
    void setProName(QString&);
    void setProPrice(int&);
    void setProCount(int&);
    void setProType(QString&);
};

#endif // PRODUCT_H
