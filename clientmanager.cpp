#include "clientmanager.h"
#include "ui_clientmanager.h"
#include "client.h"
#include <QMessageBox>
#include <QFile>

ClientManager::ClientManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientManager)
{
    ui->setupUi(this);

    QFile file("clientlist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if(row.size()) {
            int count = row[0].toInt();
            Client* c = new Client(count, row[1], row[2], row[3], row[4], row[5]);
            clientList.insert(count, c);
        }
    }

    file.close( );

    emit

    setWindowTitle(tr("Client Side"));
}

ClientManager::~ClientManager()
{
    delete ui;

    QFile file("clientlist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const auto& v : clientList) {
        Client* c = v;
        out << c->userCount() << ", " << c->getUserID() << ", ";
        out << c->getName() << ", ";
        out << c->getPhoneNumber() << ", ";
        out << c->getAddress() << ", ";
        out << c->get_Gender() << "\n";
    }
    file.close( );
}

int ClientManager::userCount() {
    if(clientList.size() == 0) return 1;
    else {
        auto cnt = clientList.lastKey();
        return ++cnt;
    }
}

//등록 취소
void ClientManager::on_pushButton_2_clicked()
{
    ui->checkBox->setChecked(false);
    ui->checkBox_2->setChecked(false);
    ui->lineEdit->clear();
    ui->lineEdit_2->clear();
    ui->lineEdit_3->clear();
    ui->lineEdit_4->clear();
    emit cancellation();
}

//회원 등록
void ClientManager::on_pushButton_clicked()
{
    if(ui->lineEdit->text().trimmed() == "") {
        QMessageBox::warning(this, tr("가입 실패"), tr("아이디를 입력하여 주세요."));
    }
    else if(ui->lineEdit_2->text().trimmed() == "") {
        QMessageBox::warning(this, tr("가입 실패"), tr("성함을 입력하여 주세요."));
    }
    else if(ui->lineEdit_4->text().trimmed() == "") {
        QMessageBox::warning(this, tr("가입 실패"), tr("주소를 입력하여 주세요."));
    }
    else if(ui->checkBox->isChecked() == false || ui->checkBox_2->isChecked() == false) {
        QMessageBox::warning(this, tr("가입 실패"), tr("정보 수집을 동의하여 주세요"));
    }
    else {
        QString userId, name, call, address, gender;
        int ucnt = userCount();
        userId = ui->lineEdit->text();
        name = ui->lineEdit_2->text();
        call = ui->lineEdit_3->text();
        address = ui->lineEdit_4->text();
        gender = ui->comboBox->currentText();

        if(userId.length()) {
            Client *c = new Client(ucnt, userId, name, call, address, gender);
            clientList.insert(ucnt, c);
        }
        qDebug() << "회원 수: " << ucnt;
        qDebug() << "아이디: " << userId;
        qDebug() << "이름: " << name;
        qDebug() << "전화번호: " << call;
        qDebug() << "주소: " << address;
        qDebug() << "성별: " << gender;

        ui->checkBox->setChecked(false);
        ui->checkBox_2->setChecked(false);
        ui->lineEdit->clear();
        ui->lineEdit_2->clear();
        ui->lineEdit_3->clear();
        ui->lineEdit_4->clear();
        emit join();
    }
    //errorMessage->exec();
}

//회원 탈퇴 시 해당 아이디 검색 후 List에서 삭제
int ClientManager::deleteId_List(QString id) {
    int cnt = 0;
    Q_FOREACH(auto v, clientList) {
        Client *c = static_cast<Client*>(v);
        if(id == c->getUserID()) {
            clientList.remove(c->userCount());
            cnt++;
            break;
        }
    }
    return cnt;
}

//회원 정보를 담아서 보내기
void ClientManager::containClientInfo() {
    QString userId, name, call, address, gender;
    int ucnt;

    Q_FOREACH(auto v, clientList) {
        Client *c = static_cast<Client*>(v);
        ucnt = c->userCount();
        userId = c->getUserID();
        name = c->getName();
        call = c->getPhoneNumber();
        address = c->getAddress();
        gender = c->get_Gender();
        Client *item = new Client(ucnt, userId, name, call, address, gender);
        emit sendClientInfo(item);
    }
}

//로그인 시도 시 아이디가 등록되어 있는지 체크
void ClientManager::checkLoginId(QString str) {
    QString userId = "";
    Q_FOREACH(auto v, clientList) {
        Client *c = static_cast<Client*>(v);
        if(str == c->getUserID()) {
            userId = c->getUserID();
            emit successLogin(c->getName());
            break;
        }
    }

    if(userId != str) emit failedLogin();
}

//회원 정보 리스트에 등록된 회원 정보를 변경
void ClientManager::updateClientInfo(QStringList updateList) {
    int userCount;
    bool checkUser = true;

    Q_FOREACH(auto v, clientList) {
        Client *c = static_cast<Client*>(v);
        if(updateList[0] == c->getUserID()) {
            userCount = c->userCount();
            c->setName(updateList[1]);
            c->setPhoneNumber(updateList[2]);
            c->setAddress(updateList[3]);
            c->setGender(updateList[4]);

            clientList.insert(userCount, c);
            QMessageBox::information(this, tr("수정 성공"), tr("회원 정보가 수정되었습니다."));

            checkUser = false;
            break;
        }
    }

    if(checkUser) QMessageBox::information(this, tr("수정 실패"), tr("회원 아이디를 확인해주세요."));
    else {
        emit clear_Widget_N_LineEdit();

        Q_FOREACH(auto v, clientList) {
            Client *c = static_cast<Client*>(v);
            userCount = c->userCount();
            updateList[0] = c->getUserID();
            updateList[1] = c->getName();
            updateList[2] = c->getPhoneNumber();
            updateList[3] = c->getAddress();
            updateList[4] = c->get_Gender();
            Client *item = new Client(userCount, updateList[0], updateList[1], updateList[2], updateList[3], updateList[4]);
            emit sendClientInfo(item);
        }
    }
}

//등록된 회원 삭제
void ClientManager::deleteClientInfo(QString userId) {
    bool checkUser = true;
    Q_FOREACH(auto v, clientList) {
        Client *c = static_cast<Client*>(v);
        if(userId == c->getUserID()) {
            clientList.remove(c->userCount());
            QMessageBox::information(this, tr("삭제 성공"), tr("회원 삭제가 완료되었습니다."));
            checkUser = false;
            break;
        }
    }

    if(checkUser) QMessageBox::information(this, tr("삭제 실패"), tr("회원 아이디를 확인해주세요."));
    else {
        emit clear_Widget_N_LineEdit();

        Q_FOREACH(auto v, clientList) {
            Client *c = static_cast<Client*>(v);
            Client *item = new Client(c->userCount(), c->getUserID(), c->getName(), c->getPhoneNumber(), c->getAddress(), c->get_Gender());
            emit sendClientInfo(item);
        }
    }
}

QString ClientManager::findAddressForOrder(QString orderName) {
    QString orderAddress;
    Q_FOREACH(auto v, clientList) {
        Client *c = static_cast<Client*>(v);
        if(orderName == c->getName()) {
            orderAddress = c->getAddress();
            break;
        }
    }

    return orderAddress;
}

void ClientManager::serverOpenFromShopping() {
    qDebug("serverOpenFromShopping");
    QString sendSeverName;
    int sendServerId;
    Q_FOREACH(auto v, clientList) {
        sendServerId = v->getUserID().toInt();
        sendSeverName = v->getName();
        emit sendToServer(sendServerId, sendSeverName);
        qDebug("sendToServer");
    }

}
