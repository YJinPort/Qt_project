#include "shoppingmanager.h"
#include "ui_shoppingmanager.h"
#include "shopping.h"
#include "product.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QFile>

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

//회원정보, 제품정보 관리
void ShoppingManager::on_pushButton_6_clicked()
{
    bool ok;
    QString passwd;

    do {
        passwd = QInputDialog::getText(this, "Manager", "관리자 번호를 입력하세요.", QLineEdit::Normal, NULL, &ok);
        if(ok == false) break;
    } while(ok != true || passwd != "ossmall");

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
    emit login(ui->lineEdit->text());
    ui->lineEdit->clear();
}

//Label에 사용자 이름 표시
void ShoppingManager::successLoginCheck(QString clientName) {
    ui->label_2->setText(clientName + "님의 주문내역");

    loadShoppingWidget(clientName);
}

void ShoppingManager::loadShoppingWidget(QString name) {
    ui->treeWidget_3->clear();
    qDebug() << name;

    foreach(const auto v, shoppingList) {
        qDebug("3");
        Shopping *s = static_cast<Shopping*>(v);

        qDebug("4");
        //Shopping을 참조하려하면 crashed남....
        if(s->getClientName() == "") {
            qDebug("없음");
            break;
        }
        qDebug("5");
        if(name == s->getClientName()) {
            ui->treeWidget_3->addTopLevelItem(s);
        }
    }
}

void ShoppingManager::failedLoginCheck() {
    ui->lineEdit->clear();

    //로그인 실패 메시지
    QMessageBox::critical(this, tr("로그인 실패"), tr("아이디가 일치하지 않습니다."));
}

//주문하기
void ShoppingManager::on_pushButton_3_clicked()
{
    int orderCount, proPrice, proCount;
    QString proName, proType, address;
    QString clientName;
    QList<QString> labelText;
    bool ok;

    labelText = ui->label_2->text().split("님");
    clientName = labelText[0];

    if(ui->label_2->text().length() > 5 && ui->treeWidget->currentItem() != nullptr) {
        orderCount = shoppingCount();
        proName = ui->treeWidget->currentItem()->text(1);
        proPrice = ui->treeWidget->currentItem()->text(2).toInt();
        proCount = QInputDialog::getText(this, "Order", "주문 수량을 입력하세요.", QLineEdit::Normal, NULL, &ok).toInt();
        proType = ui->treeWidget->currentItem()->text(4);
        address = emit takeOrderSign(clientName);

        Shopping *s = new Shopping(orderCount, proName, proPrice, proCount, proType, address, clientName);
        shoppingList.insert(orderCount, s);
        QMessageBox::information(this, tr("주문 성공"), tr("주문이 완료되었습니다."));
        ui->treeWidget_3->addTopLevelItem(s);
    }
    else return;
}

//주문 변경
void ShoppingManager::on_pushButton_5_clicked()
{
    qDebug("주문 변경");
}

//주문 취소
void ShoppingManager::on_pushButton_4_clicked()
{
    qDebug("주문 취소");
}

//제품 정보 리스트 출력
void ShoppingManager::receivedProductInfo(Product *p) {
    ui->treeWidget->addTopLevelItem(p);
}
