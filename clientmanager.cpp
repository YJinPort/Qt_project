#include "clientmanager.h"
#include "ui_clientmanager.h"
#include "client.h"
#include <QMessageBox>
#include <QFile>

//생성자 - clientlist.txt에 저장된 정보를 불러와 사용자리스트에 저장한다.
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

    setWindowTitle(tr("Client Side"));
}

//소멸자 - 사용자리스트에 저장된 정보를 clientlist.txt에 저장한다.
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

//회원 수를 자동으로 생성하여 전달해주기 위한 함수
int ClientManager::userCount() {
    if(clientList.size() == 0) return 1;
    else {
        auto cnt = clientList.lastKey();
        return ++cnt;
    }
}

//회원 등록 버튼 클릭 시 동작
void ClientManager::on_clientRegisterPushButton_clicked()
{
    if(ui->userIdLineEdit->text().trimmed() == "") {
        QMessageBox::warning(this, tr("가입 실패"), tr("아이디를 입력하여 주세요."));
    }
    else if(ui->userNameLineEdit->text().trimmed() == "") {
        QMessageBox::warning(this, tr("가입 실패"), tr("성함을 입력하여 주세요."));
    }
    else if(ui->userAddressLineEdit->text().trimmed() == "") {
        QMessageBox::warning(this, tr("가입 실패"), tr("주소를 입력하여 주세요."));
    }
    else if(ui->agreeClientInfoCheckBox->isChecked() == false || ui->agreeAddressCheckBox->isChecked() == false) {
        QMessageBox::warning(this, tr("가입 실패"), tr("정보 수집을 동의하여 주세요"));
    }
    else {
        QString userId, name, call, address, gender;
        int ucnt = userCount();
        userId = ui->userIdLineEdit->text();
        name = ui->userNameLineEdit->text();
        call = ui->userCallLineEdit->text();
        address = ui->userAddressLineEdit->text();
        gender = ui->userGenderComboBox->currentText();

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

        ui->agreeClientInfoCheckBox->setChecked(false);
        ui->agreeAddressCheckBox->setChecked(false);
        ui->userIdLineEdit->clear();
        ui->userNameLineEdit->clear();
        ui->userCallLineEdit->clear();
        ui->userAddressLineEdit->clear();
        emit join();
    }
}

//등록 취소 버튼 클릭 시 동작
void ClientManager::on_cancelRegisterPushButton_clicked()
{
    ui->agreeClientInfoCheckBox->setChecked(false);
    ui->agreeAddressCheckBox->setChecked(false);
    ui->userIdLineEdit->clear();
    ui->userNameLineEdit->clear();
    ui->userCallLineEdit->clear();
    ui->userAddressLineEdit->clear();
    emit cancellation();
}

//관리자 페이지에서 회원 정보 수정 시 회원 정보 리스트에 등록된 회원 정보를 변경하기 위한 SLOT 함수
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

//관리자 페이지에서 회원 삭제 시 등록된 회원을 삭제하기 위한 SLOT 함수
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

//쇼핑 화면에서 관리자 페이지로 이동 버튼 클릭 시 회원 정보를 담아서 보내기 위한 SLOT 함수
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

//쇼핑 화면에서 로그인 시도 시 아이디가 등록되어 있는지 체크하는 SLOT 함수
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

//쇼핑 화면에서 주문하기 버튼 클릭 시 주문자의 주소 정보를 찾아주기 위한 SLOT 함수
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

//쇼핑 화면에서 회원 탈퇴 버튼 클릭 시 해당 아이디 검색 후 List에서 삭제하기 위한 SLOT 함수
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

//쇼핑 화면에서 서버오픈 시 사용자의 ID와 이름을 전달해주기 위한 SLOT 함수
void ClientManager::serverOpenFromShopping() {
    qDebug("serverOpenFromShopping");
    QString sendSeverName;
    //int sendServerId;
    QString sendServerId;
    Q_FOREACH(auto v, clientList) {
        //sendServerId = v->getUserID().toInt();
        sendServerId = v->getUserID();
        sendSeverName = v->getName();
        qDebug() << sendServerId;
        emit sendToServer(sendServerId, sendSeverName);
        qDebug("sendToServer");
    }
}
