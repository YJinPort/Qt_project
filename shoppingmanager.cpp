#include "shoppingmanager.h"
#include "ui_shoppingmanager.h"
#include "shopping.h"
#include "product.h"
#include "chattingform_client.h"
#include "serverside.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFile>
#include <QSpinBox>

//생성자 - shoppinglist.txt에 저장된 정보를 불러와 쇼핑리스트에 저장한다.
ShoppingManager::ShoppingManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShoppingManager)
{
    ui->setupUi(this);

    QFile file("shoppinglist.txt");  //shoppinglist.txt라는 파일을 불러온다.

    /*해당 파일을 텍스트 파일의 읽기 전용으로 열기*/
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;  //파일을 열기에 실패하면 return;

    QTextStream in(&file);  //파일의 정보를 textStream에 저장할 준비를 한다.

    /*저장된 정보가 끝날 때 까지 반복*/
    while (!in.atEnd()) {
        QString line = in.readLine();           //저장 되어있는 정보를 QString타입의 변수에 담는다.
        QList<QString> row = line.split(", ");  //리스트 변수 row에 ", " 구분자를 제외한 데이터를 담는다.
        if(row.size()) {    //리스트가 비어있지 않은 경우
            int shoppingCount = row[0].toInt(); //0번째 인덱스에 있는 정보를 int타입으로 변환하여 변수에 담는다.
            int proPrice = row[2].toInt();      //2번째 인덱스에 있는 정보를 int타입으로 변환하여 변수에 담는다.
            int proCount = row[3].toInt();      //3번째 인덱스에 있는 정보를 int타입으로 변환하여 변수에 담는다.

            //해당 정보를 담은 객체를 생성한다.
            Shopping* s = new Shopping(shoppingCount, row[1], proPrice, proCount, row[4], row[5], row[6]);
            shoppingList.insert(shoppingCount, s);  //정보를 담은 객체를 주문 리스트에 저장한다.
        }
    }
    file.close();   //shoppinglist.txt파일에 저장된 정보를 모두 회원 리스트에 저장했으므로 파일을 종료한다.

    setWindowTitle(tr("Shopping Side"));    //열리는 윈도우의 제목을 Shopping Side로 설정한다.
}

//소멸자 - 쇼핑리스트에 저장된 정보를 shoppinglist.txt에 저장한다.
ShoppingManager::~ShoppingManager()
{
    delete ui;

    QFile file("shoppinglist.txt"); //shoppinglist.txt라는 파일을 불러온다(없을 경우 생성한다).

    /*해당 파일을 텍스트 파일의 쓰기 전용으로 열기*/
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return; //파일을 열기에 실패하면 return;

    QTextStream out(&file); //파일의 정보를 textStream으로 출력할 준비를 한다.

    /*주문 리스트에 저장된 정보를 파일에 모두 저장하기 위한 반복문*/
    for (const auto& v : shoppingList) {
        Shopping* s = v;

        /*주문 리스트에 대한 각 정보들을 , 를 구분자로 하여 파일(shoppinglist.txt)에 저장한다.*/
        out << s->shoppingCount() << ", " << s->getProductName() << ", ";
        out << s->getProductPrice() << ", " << s->getProductCount() << ", ";
        out << s->getProductType() << ", " << s->getClientAddress() << ", ";
        out << s->getClientName() << "\n";
    }

    file.close( );  //shoppinglist.txt파일로 회원 리스트에 저장된 정보를 출력해 저장했으므로 파일을 종료한다.
}

//제품 리스트의 정보를 불러오기 위한 신호를 보내는 함수
void ShoppingManager::dataLoad() {
    emit viewProductList(); //제품 리스트를 불러오기 위해 호출하는 SIGANL
}

//등록되어 있는 제품 정보를 쇼핑화면의 제품 정보 리스트에 출력하는 SLOT 함수
void ShoppingManager::receivedProductInfo(Product *p) {
    //관리자 페이지에서 받아온 제품 정보를 쇼핑 화면의 제품 리스트 위젯에 등록
    ui->productInfoTreeWidget->addTopLevelItem(p);
}

//제품 정보의 리스트를 초기화하는 함수
void ShoppingManager::dataClear() {
    //쇼핑 화면의 제품 리스트 위젯을 비운다.
    ui->productInfoTreeWidget->clear();
}

//주문 번호를 자동으로 생성하여 전달해주기 위한 함수
int ShoppingManager::shoppingCount() {
    if(shoppingList.size() == 0) return 1;  //주문 정보 리스트에 저장된 정보가 없으면 1을 반환한다.
    else {
        auto cnt = shoppingList.lastKey();  //주문 정보 리스트에 저장된 마지막 키값을 얻는다.
        return ++cnt;                       //얻은 키값에 1을 더한 값을 반환한다.
    }
}

//회원가입 버튼 클릭 시 동작
void ShoppingManager::on_addNewClientPushButton_clicked()
{
    emit newClient();   //회원 가입 화면을 열기 위해 호출하는 SIGNAL
}

//회원탈퇴 버튼 클릭 시 동작
void ShoppingManager::on_removeClientPushButton_clicked()
{
    bool questionCheck, inputUserId;
    int checkUserId;
    QString question, userId;

   /*입력값의 예외처리를 위한 do-while문*/
    do {
        question = QInputDialog::getText(this, "회원 탈퇴", "탈퇴하시겠습니까? (입력: 회원탈퇴)", QLineEdit::Normal, NULL, &questionCheck);
        if(questionCheck == false) break;
    } while(question.trimmed() != "회원탈퇴");

    if(question.trimmed() == "회원탈퇴") {  //"회원탈퇴"를 온전히 입력했을 경우
        /*탈퇴하기 위해 입력한 아이디의 예외처리, 검사를 위한 do-while문*/
        do {
            //탈퇴할 아이디의 입력을 받는다.
            userId = QInputDialog::getText(this, "Manager", "회원 아이디를 입력해주세요.", QLineEdit::Normal, NULL, &inputUserId);
            if(inputUserId == false) break; //inputDialog의 취소를 누를 경우 종료한다.
            checkUserId = emit deleteClient(userId);    //입력한 회원 아이디 값이 회원 정보 리스트에 등록되어 있는지 확인하기 위해 호출하는 SIGNAL

            //SIGNAL이 결과 여부에 따라 출력하는 메시지를 구분한다.
            if(checkUserId <= 0) QMessageBox::warning(this, tr("탈퇴 실패"), tr("존재하지 않는 아이디 입니다."));
            else QMessageBox::warning(this, tr("탈퇴 성공"), tr("회원 탈퇴되었습니다."));
        } while(checkUserId <= 0);
    }
}

//로그인 버튼 클릭 시 동작
void ShoppingManager::on_shoppingLoginPushButton_clicked()
{
    emit login(ui->userIdLoginLineEdit->text());    //입력한 로그인 아이디가 clientList에 등록되어 있는지 확인
    ui->userIdLoginLineEdit->clear();               //로그인할 아이디를 입력한 LineEdit를 비운다.
}

//로그인 성공 시 동작하는 SLOT 함수
void ShoppingManager::successLoginCheck(QString clientName) {
    ui->orderListLabel->setText(clientName + "님의 주문내역");   //Label에 사용자 이름 표시

    loadShoppingWidget(clientName);     //로그인한 사용자가 주문한 주문내역을 불러오기 위한 함수
}

//로그인 성공 시 주문 내역 리스트에 해당 사용자가 주문한 리스트 출력
void ShoppingManager::loadShoppingWidget(QString name) {
    /*사용자가 주문한 주문내역을 확인하기 위한 반복문*/
    Q_FOREACH(auto v, shoppingList)
    {
        Shopping *s = static_cast<Shopping*>(v);

        //현재 표시되어있는 주문 리스트를 삭제한다.
        ui->orderListTreeWidget->takeTopLevelItem(ui->orderListTreeWidget->indexOfTopLevelItem(s));

        if(name == s->getClientName())  //인자로 받은 사용자의 이름이 주문 내역 리스트의 사용자 이름과 같은 경우
        {
            ui->orderListTreeWidget->addTopLevelItem(s);    //해당 주문 정보를 주문 내역 리스트에 표시한다.
        }
    }
}

//로그인 실패 시 동작하는 SLOT 함수
void ShoppingManager::failedLoginCheck() {
    //로그인 실패 메시지
    QMessageBox::critical(this, tr("로그인 실패"), tr("아이디가 일치하지 않습니다."));
}

//주문하기 버튼 클릭 시 동작
void ShoppingManager::on_takeOrderPushButton_clicked()
{
    int orderCount, proPrice, proCount, checkCount;
    QString proName, proType, address;
    QString clientName;
    QList<QString> labelText;
    bool ok;
    QLineEdit *onlyNum = new QLineEdit(this);
    QIntValidator *intValidator = new QIntValidator(this);
    onlyNum->setValidator(intValidator);

    labelText = ui->orderListLabel->text().split("님");
    clientName = labelText[0];

    if(ui->orderListLabel->text().length() > 5 && ui->productInfoTreeWidget->currentItem() != nullptr) {
        orderCount = shoppingCount();
        proName = ui->productInfoTreeWidget->currentItem()->text(1);
        proPrice = ui->productInfoTreeWidget->currentItem()->text(2).toInt();
        proCount = QInputDialog::getText(this, "Order", "주문 수량을 입력하세요.", onlyNum->Normal, NULL, &ok).toInt();
        if(ok == false) return;
        else if(proCount <= 0) {
            QMessageBox::warning(this, tr("주문 실패"), tr("주문 수량을 확인해주세요."));
            return;
        }
        proType = ui->productInfoTreeWidget->currentItem()->text(4);
        address = emit takeOrderSign(clientName);

        checkCount = emit updateAfter_upCount(ui->productInfoTreeWidget->currentItem()->text(1), proCount);
        if(checkCount < 0) {
            QMessageBox::information(this, tr("주문 실패"), tr("재고가 부족합니다."));
            return;
        }

        Shopping *s = new Shopping(orderCount, proName, proPrice, proCount, proType, address, clientName);
        shoppingList.insert(orderCount, s);
        QMessageBox::information(this, tr("주문 성공"), tr("주문이 완료되었습니다."));
        ui->orderListTreeWidget->addTopLevelItem(s);
        dataClear();
        dataLoad();
    }
    else return;
}

//주문변경 버튼 클릭 시 동작
void ShoppingManager::on_updateOrderPushButton_clicked()
{
    bool choice, change;
    int updateCount, checkCount;
    QString choiceUpdate, updateAddress;
    QLineEdit *onlyNum = new QLineEdit(this);
    QIntValidator *intValidator = new QIntValidator(this);
    onlyNum->setValidator(intValidator);

    if(ui->orderListTreeWidget->currentItem() != nullptr) {
        choiceUpdate = QInputDialog::getText(this, "변경 정보 선택", "주문수량 / 배송주소", QLineEdit::Normal, NULL, &choice);
        if(choiceUpdate.trimmed() == "주문수량") {
            do {
                updateCount = QInputDialog::getText(this, "Update", "변경하실 수량을 입력해주세요.", onlyNum->Normal, NULL, &change).toInt();
                if(change == false) {
                    updateCount = ui->orderListTreeWidget->currentItem()->text(3).toInt();
                    break;
                }
            } while(change != true || updateCount <= 0);

            Q_FOREACH(auto v, shoppingList) {
                Shopping *s = static_cast<Shopping*>(v);
                if(ui->orderListTreeWidget->currentItem()->text(0).toInt() == s->shoppingCount()) {
                    if(s->getProductCount() < updateCount) {
                        int upCount = updateCount - s->getProductCount();
                        checkCount = emit updateAfter_upCount(ui->orderListTreeWidget->currentItem()->text(1), upCount);
                        if(checkCount < 0) {
                            QMessageBox::information(this, tr("변경 실패"), tr("재고가 부족합니다."));
                            break;
                        }
                    }
                    else {
                        int downCount = s->getProductCount() - updateCount;
                        emit updateAfter_downCount(ui->orderListTreeWidget->currentItem()->text(1), downCount);
                    }
                    s->setProductCount(updateCount);
                    ui->orderListTreeWidget->currentItem()->setText(3, QString::number(updateCount));
                    shoppingList.insert(ui->orderListTreeWidget->currentItem()->text(0).toInt(), s);

                    QMessageBox::information(this, tr("변경 성공"), tr("주문 수량이 수정되었습니다."));

                    dataClear();
                    dataLoad();

                    break;
                }
            }

        }
        else if(choiceUpdate.trimmed() == "배송주소") {
            do {
                updateAddress = QInputDialog::getText(this, "Update", "변경하실 주소를 입력해주세요.", QLineEdit::Normal, NULL, &change);
                if(change == false) {
                    updateAddress = ui->orderListTreeWidget->currentItem()->text(5);
                    break;
                }
            } while(change != true || updateAddress.trimmed() == "");

            Q_FOREACH(auto v, shoppingList) {
                Shopping *s = static_cast<Shopping*>(v);
                if(ui->orderListTreeWidget->currentItem()->text(0).toInt() == s->shoppingCount()) {
                    s->setClientAddress(updateAddress);
                    ui->orderListTreeWidget->currentItem()->setText(5, updateAddress);
                    shoppingList.insert(ui->orderListTreeWidget->currentItem()->text(0).toInt(), s);
                    QMessageBox::information(this, tr("변경 성공"), tr("배송 주소가 수정되었습니다."));
                    break;
                }
            }
        }
    }
}

//주문취소 버튼 클릭 시 동작
void ShoppingManager::on_cancelOrderPushButton_clicked()
{
    int eraseNum;
    if(ui->orderListTreeWidget->currentItem() != nullptr) {
        eraseNum = ui->orderListTreeWidget->currentItem()->text(0).toInt();
        Q_FOREACH(auto v, shoppingList) {
            Shopping *s = static_cast<Shopping*>(v);
            if(eraseNum == s->shoppingCount()) {
                emit updateAfter_downCount(ui->orderListTreeWidget->currentItem()->text(1), ui->orderListTreeWidget->currentItem()->text(3).toInt());
                shoppingList.remove(eraseNum);
                ui->orderListTreeWidget->takeTopLevelItem(ui->orderListTreeWidget->indexOfTopLevelItem(s));
                QMessageBox::information(this, tr("취소 성공"), tr("주문이 취소되었습니다."));
                dataClear();
                dataLoad();
                break;
            }
        }
    }
    else QMessageBox::warning(this, tr("취소 실패"), tr("취소하실 주문을 선택해주세요."));
}

//제품/회원 정보 관리 버튼 클릭 시 동작
void ShoppingManager::on_managementPushButton_clicked()
{
    bool ok;
    QString passwd;

    do {
        passwd = QInputDialog::getText(this, "Manager", "관리자 번호를 입력하세요.", QLineEdit::Normal, NULL, &ok);
        if(ok == false) break;
    } while(/*ok != true || */passwd.trimmed() != "ossmall");

    if(ok == true) {
        emit viewClientList();
        emit onlyStaff();
    }
}

//채팅하기 버튼 클릭 시 동작
void ShoppingManager::on_chatClientPushButton_clicked()
{
    ChattingForm_Client *clientForm = new ChattingForm_Client();
    //clientForm = new ChattingForm_Client();
    clientForm->show();
}

//서버오픈 버튼 클릭 시 동작
void ShoppingManager::on_chatServerPushButton_clicked()
{
    bool ok;
    QString passwd;

    do {
        passwd = QInputDialog::getText(this, "Manager", "관리자 번호를 입력하세요.", QLineEdit::Normal, NULL, &ok);
        if(ok == false) break;
    } while(/*ok != true || */passwd.trimmed() != "ossmall");

    if(ok == true) {
        ServerSide *serverForm = new ServerSide();
        connect(this, SIGNAL(sendClientToServer(QString, QString)), serverForm, SLOT(addClient(QString, QString)));
        emit serverBtnClicked();

        serverForm->show();
        //ui->pushButton_10->setDisabled(true);
    }
}

//사용자의 아이디와 리스트를 받아서 채팅서버로 전달하기 위한 SLOT 함수
void ShoppingManager::clientSignalReceived(QString id, QString name) {
    emit sendClientToServer(id, name);
}

//쇼핑 끝내기 버튼 클릭 시 동작
void ShoppingManager::on_exitShoppingPushButton_clicked()
{
    emit exitShopping();
}
