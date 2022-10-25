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
    if(clientList.size() == 0) return 1;    //회원 정보 리스트에 저장된 정보가 없으면 1을 반환한다.
    else {
        auto cnt = clientList.lastKey();    //회원 정보 리스트에 저장된 마지막 키값을 얻는다.
        return ++cnt;                       //얻은 키값에 1을 더한 값을 반환한다.
    }
}

//회원 등록 버튼 클릭 시 동작
void ClientManager::on_clientRegisterPushButton_clicked()
{
    /*아이디, 이름, 주소를 입력 혹은 정보 수집을 동의하지 않았을 경우 경고메시지 발생*/
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
    /*회원가입 시작*/
    else {
        QString userId, name, call, address, gender;
        int ucnt = userCount();     //회원 수의 경우 자동 생성 함수의 반환값을 받아온다.

        /*나머지 정보의 경우 입력된 LineEdit에 적혀있는 값을 가져온다.*/
        userId = ui->userIdLineEdit->text();
        name = ui->userNameLineEdit->text();
        call = ui->userCallLineEdit->text();
        address = ui->userAddressLineEdit->text();
        gender = ui->userGenderComboBox->currentText();

        Client *c = new Client(ucnt, userId, name, call, address, gender);
        clientList.insert(ucnt, c);     //사용자 리스트에 회원 정보를 입력(저장)한다.

        /*현재 입력 되어있는 LineEdit를 비우고 체크 되어있는 CheckBox를 체크 해제시킨다*/
        ui->agreeClientInfoCheckBox->setChecked(false);
        ui->agreeAddressCheckBox->setChecked(false);
        ui->userIdLineEdit->clear();
        ui->userNameLineEdit->clear();
        ui->userCallLineEdit->clear();
        ui->userAddressLineEdit->clear();

        emit join();    //쇼핑 화면으로 돌아가기 위해 SIGNAL 신호를 보낸다.
    }
}

//등록 취소 버튼 클릭 시 동작
void ClientManager::on_cancelRegisterPushButton_clicked()
{
    /*현재 입력 되어있는 LineEdit를 비우고 체크 되어있는 CheckBox를 체크 해제시킨다*/
    ui->agreeClientInfoCheckBox->setChecked(false);
    ui->agreeAddressCheckBox->setChecked(false);
    ui->userIdLineEdit->clear();
    ui->userNameLineEdit->clear();
    ui->userCallLineEdit->clear();
    ui->userAddressLineEdit->clear();

    emit cancellation();    //쇼핑 화면으로 돌아가기 위해 SIGNAL 신호를 보낸다.
}

//관리자 페이지에서 회원 정보 수정 시 회원 정보 리스트에 등록된 회원 정보를 변경하기 위한 SLOT 함수
void ClientManager::updateClientInfo(QStringList updateList) {
    int userCount;
    bool checkUser = true;

    /*변경하려는 회원 정보가 리스트에 등록되어 있는 확인하기 위한 반복문*/
    Q_FOREACH(auto v, clientList) {
        Client *c = static_cast<Client*>(v);
        if(updateList[0] == c->getUserID()) {   //회원 아이디가 이미 등록 되어있을 경우
            userCount = c->userCount();     //리스트의 키값을 위한 변수에 값을 입력
            /*현재 키값의 회원 정보를 변경될 값으로 지정한다*/
            c->setName(updateList[1]);
            c->setPhoneNumber(updateList[2]);
            c->setAddress(updateList[3]);
            c->setGender(updateList[4]);

            clientList.insert(userCount, c);    //사용자 리스트에 회원 정보를 입력(저장)한다.
            checkUser = false;  //등록된 회원이라는 것이 확인되어 변경에 성공했기에 아래의 조건문을 위해 값을 변경한다.

            QMessageBox::information(this, tr("수정 성공"), tr("회원 정보가 수정되었습니다."));     //수정에 성공했다는 메시지를 띄운다.

            break;
        }
    }

    if(checkUser) QMessageBox::warning(this, tr("수정 실패"), tr("회원 아이디를 확인해주세요."));   //등록된 회원 아이디가 아니기 때문에 경고 메시지를 띄운다.
    /*관리자 페이지의 위젯 리스트에 변경된 회원 정보를 등록 시작*/
    else {
        emit clear_Widget_N_LineEdit();     //관리자 페이지의 회원 위젯 리스트를 비우기 위해 호출되는 신호

        /*비운 위젯 리스트에 변경된 회원 정보를 등록하기 위한 반복문*/
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
        Client *c = static_cast<Client*>(v);    //auto변수 v의 자료형을 Client*형으로 변환 후 고정
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
