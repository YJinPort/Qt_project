#include "product.h"

Product::Product(int proNumber, QString proName, int proPrice, int proCount, QString proType)
{
    setText(0, QString::number(proNumber));
    setText(1, proName);
    setText(2, QString::number(proPrice));
    setText(3, QString::number(proCount));
    setText(4, proType);
}

int Product::getProNumber() const {
    return text(0).toInt();
}

QString Product::getProName() const {
    return text(1);
}

int Product::getProPrice() const {
    return text(2).toInt();
}

int Product::getProCount() const {
    return text(3).toInt();
}

QString Product::getProType() const {
    return text(4);
}

void Product::setProNumber(int& proNumber) {
    setText(0, QString::number(proNumber));
}

void Product::setProName(QString& proName) {
    setText(1, proName);
}

void Product::setProPrice(int& proPrice) {
    setText(2, QString::number(proPrice));
}

void Product::setProCount(int& proCount) {
    setText(3, QString::number(proCount));
}

void Product::setProType(QString& proType) {
    setText(4, proType);
}
