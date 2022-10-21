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

ShoppingManager::ShoppingManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShoppingManager)
{
    ui->setupUi(this);

    QFile file("shoppinglist.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QList<QString> row = line.split(", ");
        if(row.size()) {
            int shoppingCount = row[0].toInt();
            int proPrice = row[2].toInt();
            int proCount = row[3].toInt();
            Shopping* s = new Shopping(shoppingCount, row[1], proPrice, proCount, row[4], row[5], row[6]);
            //ui->treeWidget_3->addTopLevelItem(s);
            shoppingList.insert(shoppingCount, s);
        }
    }
    file.close();

    setWindowTitle(tr("Shopping Side"));
}

void ShoppingManager::dataLoad() {
    emit viewProductList();
}

void ShoppingManager::dataClear() {
    ui->treeWidget->clear();
}

int ShoppingManager::shoppingCount() {
    if(shoppingList.size() == 0) return 1;
    else {
        auto cnt = shoppingList.lastKey();
        return ++cnt;
    }
}

ShoppingManager::~ShoppingManager()
{
    delete ui;

    QFile file("shoppinglist.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const auto& v : shoppingList) {
        Shopping* s = v;
        out << s->shoppingCount() << ", " << s->getProductName() << ", ";
        out << s->getProductPrice() << ", " << s->getProductCount() << ", ";
        out << s->getProductType() << ", " << s->getClientAddress() << ", ";
        out << s->getClientName() << "\n";
    }
    file.close( );
}

//회원 가입
void ShoppingManager::on_pushButton_clicked()
{
    emit newClient();
}

//회원탈퇴
void ShoppingManager::on_pushButton_9_clicked()
{
    bool questionCheck, inputUserId;
    int checkUserId;
    QString question, userId;

    do {
        question = QInputDialog::getText(this, "회원 탈퇴", "탈퇴하시겠습니까? (입력: 회원탈퇴)", QLineEdit::Normal, NULL, &questionCheck);
        if(questionCheck == false) break;
    } while(question.trimmed() != "회원탈퇴");

    if(question.trimmed() == "회원탈퇴") {
        do {
            userId = QInputDialog::getText(this, "Manager", "회원 아이디를 입력해주세요.", QLineEdit::Normal, NULL, &inputUserId);
            if(inputUserId == false) break;
            checkUserId = emit deleteClient(userId);
            if(checkUserId <= 0) QMessageBox::warning(this, tr("탈퇴 실패"), tr("존재하지 않는 아이디 입니다."));
            else QMessageBox::warning(this, tr("탈퇴 성공"), tr("회원 탈퇴되었습니다."));
        } while(checkUserId <= 0);
    }
}

//회원정보, 제품정보 관리
void ShoppingManager::on_pushButton_6_clicked()
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



//쇼핑 종료
void ShoppingManager::on_pushButton_7_clicked()
{
    emit exitShopping();
}

//로그인 버튼
void ShoppingManager::on_pushButton_2_clicked()
{
    emit login(ui->lineEdit->text());   //입력한 로그인 아이디가 clientList에 등록되어 있는지 확인
    ui->lineEdit->clear();
}

//로그인 성공 시
void ShoppingManager::successLoginCheck(QString clientName) {
    ui->label_2->setText(clientName + "님의 주문내역");   //Label에 사용자 이름 표시
    //ui->treeWidget_3->clear();

    loadShoppingWidget(clientName);
}

void ShoppingManager::loadShoppingWidget(QString name) {
    Q_FOREACH(auto v, shoppingList)
    {
        Shopping *s = static_cast<Shopping*>(v);
        ui->treeWidget_3->takeTopLevelItem(ui->treeWidget_3->indexOfTopLevelItem(s));
        if(name == s->getClientName())
        {
            ui->treeWidget_3->addTopLevelItem(s);
        }
    }
}

//로그인 실패 시
void ShoppingManager::failedLoginCheck() {
    //로그인 실패 메시지
    QMessageBox::critical(this, tr("로그인 실패"), tr("아이디가 일치하지 않습니다."));
}

//주문하기
void ShoppingManager::on_pushButton_3_clicked()
{
    int orderCount, proPrice, proCount, checkCount;
    QString proName, proType, address;
    QString clientName;
    QList<QString> labelText;
    bool ok;
    QLineEdit *onlyNum = new QLineEdit(this);
    QIntValidator *intValidator = new QIntValidator(this);
    onlyNum->setValidator(intValidator);

    labelText = ui->label_2->text().split("님");
    clientName = labelText[0];

    if(ui->label_2->text().length() > 5 && ui->treeWidget->currentItem() != nullptr) {
        orderCount = shoppingCount();
        proName = ui->treeWidget->currentItem()->text(1);
        proPrice = ui->treeWidget->currentItem()->text(2).toInt();
        proCount = QInputDialog::getText(this, "Order", "주문 수량을 입력하세요.", onlyNum->Normal, NULL, &ok).toInt();
        if(ok == false) return;
        else if(proCount <= 0) {
            QMessageBox::warning(this, tr("주문 실패"), tr("주문 수량을 확인해주세요."));
            return;
        }
        proType = ui->treeWidget->currentItem()->text(4);
        address = emit takeOrderSign(clientName);

        checkCount = emit updateAfter_upCount(ui->treeWidget->currentItem()->text(1), proCount);
        if(checkCount < 0) {
            QMessageBox::information(this, tr("주문 실패"), tr("재고가 부족합니다."));
            return;
        }

        Shopping *s = new Shopping(orderCount, proName, proPrice, proCount, proType, address, clientName);
        shoppingList.insert(orderCount, s);
        QMessageBox::information(this, tr("주문 성공"), tr("주문이 완료되었습니다."));
        ui->treeWidget_3->addTopLevelItem(s);
        dataClear();
        dataLoad();
    }
    else return;
}

//주문 변경
void ShoppingManager::on_pushButton_5_clicked()
{
    bool choice, change;
    int updateCount, checkCount;
    QString choiceUpdate, updateAddress;
    QLineEdit *onlyNum = new QLineEdit(this);
    QIntValidator *intValidator = new QIntValidator(this);
    onlyNum->setValidator(intValidator);

    if(ui->treeWidget_3->currentItem() != nullptr) {
        choiceUpdate = QInputDialog::getText(this, "변경 정보 선택", "주문수량 / 배송주소", QLineEdit::Normal, NULL, &choice);
        if(choiceUpdate.trimmed() == "주문수량") {
            do {
                updateCount = QInputDialog::getText(this, "Update", "변경하실 수량을 입력해주세요.", onlyNum->Normal, NULL, &change).toInt();
                if(change == false) {
                    updateCount = ui->treeWidget_3->currentItem()->text(3).toInt();
                    break;
                }
            } while(change != true || updateCount <= 0);

            Q_FOREACH(auto v, shoppingList) {
                Shopping *s = static_cast<Shopping*>(v);
                if(ui->treeWidget_3->currentItem()->text(0).toInt() == s->shoppingCount()) {
                    if(s->getProductCount() < updateCount) {
                        int upCount = updateCount - s->getProductCount();
                        checkCount = emit updateAfter_upCount(ui->treeWidget_3->currentItem()->text(1), upCount);
                        if(checkCount < 0) {
                            QMessageBox::information(this, tr("변경 실패"), tr("재고가 부족합니다."));
                            break;
                        }
                    }
                    else {
                        int downCount = s->getProductCount() - updateCount;
                        emit updateAfter_downCount(ui->treeWidget_3->currentItem()->text(1), downCount);
                    }
                    s->setProductCount(updateCount);
                    ui->treeWidget_3->currentItem()->setText(3, QString::number(updateCount));
                    shoppingList.insert(ui->treeWidget_3->currentItem()->text(0).toInt(), s);

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
                    updateAddress = ui->treeWidget_3->currentItem()->text(5);
                    break;
                }
            } while(change != true || updateAddress.trimmed() == "");

            Q_FOREACH(auto v, shoppingList) {
                Shopping *s = static_cast<Shopping*>(v);
                if(ui->treeWidget_3->currentItem()->text(0).toInt() == s->shoppingCount()) {
                    s->setClientAddress(updateAddress);
                    ui->treeWidget_3->currentItem()->setText(5, updateAddress);
                    shoppingList.insert(ui->treeWidget_3->currentItem()->text(0).toInt(), s);
                    QMessageBox::information(this, tr("변경 성공"), tr("배송 주소가 수정되었습니다."));
                    break;
                }
            }
        }


    }
}

//주문 취소
void ShoppingManager::on_pushButton_4_clicked()
{
    int eraseNum;
    if(ui->treeWidget_3->currentItem() != nullptr) {
        eraseNum = ui->treeWidget_3->currentItem()->text(0).toInt();
        Q_FOREACH(auto v, shoppingList) {
            Shopping *s = static_cast<Shopping*>(v);
            if(eraseNum == s->shoppingCount()) {
                emit updateAfter_downCount(ui->treeWidget_3->currentItem()->text(1), ui->treeWidget_3->currentItem()->text(3).toInt());
                shoppingList.remove(eraseNum);
                ui->treeWidget_3->takeTopLevelItem(ui->treeWidget_3->indexOfTopLevelItem(s));
                QMessageBox::information(this, tr("취소 성공"), tr("주문이 취소되었습니다."));
                dataClear();
                dataLoad();
                break;
            }
        }
    }
    else QMessageBox::warning(this, tr("취소 실패"), tr("취소하실 주문을 선택해주세요."));
}

//제품 정보 리스트 출력
void ShoppingManager::receivedProductInfo(Product *p) {
    ui->treeWidget->addTopLevelItem(p);
}

//채팅하기
void ShoppingManager::on_pushButton_8_clicked()
{
    ChattingForm_Client *clientForm = new ChattingForm_Client();
    //clientForm = new ChattingForm_Client();
    clientForm->show();
}

//서버오픈
void ShoppingManager::on_pushButton_10_clicked()
{
    bool ok;
    QString passwd;

    do {
        passwd = QInputDialog::getText(this, "Manager", "관리자 번호를 입력하세요.", QLineEdit::Normal, NULL, &ok);
        if(ok == false) break;
    } while(/*ok != true || */passwd.trimmed() != "ossmall");

    qDebug("serverBtnClicked");
//    emit serverBtnClicked();

    if(ok == true) {
        ServerSide *serverForm = new ServerSide();
        emit serverBtnClicked();
        serverForm->show();
        //ui->pushButton_10->setDisabled(true);
    }
}
