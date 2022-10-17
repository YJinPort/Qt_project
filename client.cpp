#include "client.h"

Client::Client(int userCount, QString userID, QString name, QString phoneNumber, QString address, QString gender)
{
    setText(0, QString::number(userCount));
    setText(1, userID);
    setText(2, name);
    setText(3, phoneNumber);
    setText(4, address);
    setText(5, gender);
}

int Client::userCount() const {
    return text(0).toInt();
}

QString Client::getUserID() const {
    return text(1);
}

QString Client::getName() const {
    return text(2);
}

QString Client::getPhoneNumber() const {
    return text(3);
}

QString Client::getAddress() const {
    return text(4);
}

QString Client::get_Gender() const {
    return text(5);
}

void Client::setUserID(QString &userID) {
    setText(1, userID);
}

void Client::setName(QString &name) {
    setText(2, name);
}

void Client::setPhoneNumber(QString &phoneNumber) {
    setText(3, phoneNumber);
}

void Client::setAddress(QString &address) {
    setText(4, address);
}

void Client::setGender(QString &gender) {
    setText(5, gender);
}

// Define copy assignment operator.
bool Client::operator==(const Client &other) const {
    return (this->text(2) == other.text(2));
}
