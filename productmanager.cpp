#include "productmanager.h"
#include "ui_productmanager.h"
#include "product.h"
#include "client.h"
#include <QMessageBox>
#include <QFile>

//생성자 - productlist.txt에 저장된 정보를 불러와 제품리스트에 저장한다.
ProductManager::ProductManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ProductManager)
{
    ui->setupUi(this);

    QFile file("productlist.txt");  //productlist.txt라는 파일을 불러온다.

    /*해당 파일을 텍스트 파일의 읽기 전용으로 열기*/
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;  //파일을 열기에 실패하면 return;

    QTextStream in(&file);  //파일의 정보를 textStream에 저장할 준비를 한다.

    /*저장된 정보가 끝날 때 까지 반복*/
    while (!in.atEnd()) {
        QString line = in.readLine();           //저장 되어있는 정보를 QString타입의 변수에 담는다.
        QList<QString> row = line.split(", ");  //리스트 변수 row에 ", " 구분자를 제외한 데이터를 담는다.
        if(row.size()) {    //리스트가 비어있지 않은 경우
            int number = row[0].toInt();    //0번째 인덱스에 있는 정보를 int타입으로 변환하여 변수에 담는다.
            int price = row[2].toInt();     //2번째 인덱스에 있는 정보를 int타입으로 변환하여 변수에 담는다.
            int count = row[3].toInt();     //3번째 인덱스에 있는 정보를 int타입으로 변환하여 변수에 담는다.

            //해당 정보를 담은 객체를 생성한다.
            Product* p = new Product(number, row[1], price, count, row[4]);
            ui->productListTreeWidget->addTopLevelItem(p);  //관리자 페이지의 제품 리스트 위젯에 출력한다.
            productList.insert(number, p);  //정보를 담은 객체를 제품 리스트에 저장한다.
        }
    }
    file.close();   //productlist.txt파일에 저장된 정보를 모두 제품 리스트에 저장했으므로 파일을 종료한다.

    setWindowTitle(tr("Product Side"));     //열리는 윈도우의 제목을 Product Side로 설정한다.
}

//소멸자 - 제품리스트에 저장된 정보를 productlist.txt에 저장한다.
ProductManager::~ProductManager()
{
    delete ui;

    QFile file("productlist.txt");  //productlist.txt라는 파일을 불러온다(없을 경우 생성한다).

    /*해당 파일을 텍스트 파일의 쓰기 전용으로 열기*/
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return; //파일을 열기에 실패하면 return;

    QTextStream out(&file); //파일의 정보를 textStream으로 출력할 준비를 한다.

    /*제품 리스트에 저장된 정보를 파일에 모두 저장하기 위한 반복문*/
    for (const auto& v : productList) {
        Product* p = v;

        /*제품 리스트에 대한 각 정보들을 , 를 구분자로 하여 파일(productlist.txt)에 저장한다.*/
        out << p->getProNumber() << ", " << p->getProName() << ", ";
        out << p->getProPrice() << ", ";
        out << p->getProCount() << ", ";
        out << p->getProType() << "\n";
    }

    file.close( );  //productlist.txt파일로 제품 리스트에 저장된 정보를 출력해 저장했으므로 파일을 종료한다.
}

//제품 등록/변경 버튼 클릭 시 동작
void ProductManager::on_productRegisterPushButton_clicked()
{
    if(ui->productNumberLineEdit->text().trimmed() == "") QMessageBox::warning(this, tr("등록 실패"), tr("제품 번호를 입력하여 주세요."));
    else if(ui->productNameLineEdit->text().trimmed() == "") QMessageBox::warning(this, tr("등록 실패"), tr("제품명을 입력하여 주세요."));
    else if(ui->productPriceLineEdit->text().trimmed() == "") QMessageBox::warning(this, tr("등록 실패"), tr("제품 가격을 입력하여 주세요."));
    else if(ui->productCountLineEdit->text().trimmed() == "") QMessageBox::warning(this, tr("등록 실패"), tr("제품 수량을 입력하여 주세요."));
    else {
        int proNumber, proPrice, proCount;
        QString proName, proType;
        bool newNum = true;

        proNumber = ui->productNumberLineEdit->text().toInt();
        proName = ui->productNameLineEdit->text();
        proPrice = ui->productPriceLineEdit->text().toInt();
        proCount = ui->productCountLineEdit->text().toInt();
        proType = ui->productTypeLineEdit->text();

        Q_FOREACH(auto v, productList) {
            Product *p = static_cast<Product*>(v);
            if(proNumber == p->getProNumber()) {    //제품 번호가 이미 등록되어 있을 경우
                //현재 lineEdit에 입력한 값으로 변경한다.
                p->setProName(proName);
                p->setProPrice(proPrice);
                p->setProCount(proCount);
                p->setProType(proType);

                //ui의 treeWidget에 입력되어있는 기존의 값을 삭제한다.
                ui->productListTreeWidget->takeTopLevelItem(ui->productListTreeWidget->indexOfTopLevelItem(p));
                ui->productListTreeWidget->update();

                //변경한 값을 제품리스트에 입력한다.(QMap의 경우 Id의 중복을 방지한다)
                productList.insert(proNumber, p);
                ui->productListTreeWidget->addTopLevelItem(p);     //변경한 입력값을 ui의 treeWidget에 등록한다.

                newNum = false;                 //이미 등록된 제품 번호가 존재하므로 newNum을 false로 설정한다.

                QMessageBox::information(this, tr("등록 성공"), tr("제품 목록이 변경되었습니다."));
                break;
            }
        }

        //제품 번호의 중복 유무로 비교한다. (bool 형식)
        if(newNum) {
            Product *p = new Product(proNumber, proName, proPrice, proCount, proType);
            productList.insert(proNumber, p);       //새로운 제품의 값을 제품리스트에 입력한다.
            ui->productListTreeWidget->addTopLevelItem(p);     //입력된 값을 ui의 treeWidget에 등록한다.
            QMessageBox::information(this, tr("등록 성공"), tr("새로운 제품이 등록되었습니다."));
        }

        ui->productNumberLineEdit->clear();
        ui->productNameLineEdit->clear();
        ui->productPriceLineEdit->clear();
        ui->productCountLineEdit->clear();
        ui->productTypeLineEdit->clear();
    }
}

//제품 삭제 버튼 클릭 시 동작
void ProductManager::on_productRemovePushButton_clicked()
{
    int proNumber = ui->productNumberLineEdit->text().toInt();
    int cnt = 0;

    if(ui->productNumberLineEdit->text().trimmed() != "") {
        Q_FOREACH(auto v, productList) {
            Product *p = static_cast<Product*>(v);
            if(proNumber == p->getProNumber()) {
                productList.remove(proNumber);
                ui->productListTreeWidget->takeTopLevelItem(ui->productListTreeWidget->indexOfTopLevelItem(p));
                ui->productListTreeWidget->update();
                QMessageBox::warning(this, tr("삭제 성공"), tr("제품 삭제를 완료하였습니다."));
                ui->productNumberLineEdit->clear();
                ui->productNameLineEdit->clear();
                ui->productPriceLineEdit->clear();
                ui->productCountLineEdit->clear();
                ui->productTypeLineEdit->clear();
                cnt++;
                break;
            }
        }
        if(cnt == 0) QMessageBox::warning(this, tr("삭제 실패"), tr("등록된 제품이 없습니다."));
    }
    else QMessageBox::warning(this, tr("삭제 실패"), tr("제품 번호를 입력하여 주세요."));
}

//회원 정보 수정 버튼 클릭 시 동작
void ProductManager::on_clientUpdatePushButton_clicked()
{
    QString userId, userName, userCall, userAddress, userGender;
    userId = ui->userIdLineEdit->text();
    userName = ui->userNameLineEdit->text();
    userCall = ui->userCallLineEdit->text();
    userAddress = ui->userAddressLineEdit->text();
    userGender = ui->userGenderLineEdit->text();

    QStringList updateCliList;
    updateCliList << userId << userName << userCall << userAddress << userGender;
    qDebug("수정 버튼 클릭");
    emit updateBtnClicked(updateCliList);
}

//회원 삭제 버튼 클릭 시 동작
void ProductManager::on_clientRemovePushButton_clicked()
{
    QString userId;
    userId = ui->userIdLineEdit->text();

    emit deleteBtnClicked(userId);
}

//나가기 버튼 클릭 시 동작
void ProductManager::on_getOutPushButton_clicked()
{
    ui->productNumberLineEdit->clear();
    ui->productNameLineEdit->clear();
    ui->productPriceLineEdit->clear();
    ui->productCountLineEdit->clear();
    ui->productTypeLineEdit->clear();

    ui->clientListTreeWidget->clear();      //회원 정보 리스트의 중복 출력 방지를 위해 트리위젯을 초기화

    emit quitProduct();
}

//제품 위젯에서 해당 제품을 클릭했을 경우 우측 라인Edit에 표시
void ProductManager::on_productListTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    ui->productNumberLineEdit->setText(item->text(0));
    ui->productNameLineEdit->setText(item->text(1));
    ui->productPriceLineEdit->setText(item->text(2));
    ui->productCountLineEdit->setText(item->text(3));
    ui->productTypeLineEdit->setText(item->text(4));
}

//회원 위젯에서 해당 회원을 클릭했을 경우 우측 라인Edit에 표시
void ProductManager::on_clientListTreeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    ui->userIdLineEdit->setText(item->text(1));
    ui->userNameLineEdit->setText(item->text(2));
    ui->userCallLineEdit->setText(item->text(3));
    ui->userAddressLineEdit->setText(item->text(4));
    ui->userGenderLineEdit->setText(item->text(5));
}

//회원 정보 리스트 출력
void ProductManager::receivedClientInfo(Client *c) {
    ui->clientListTreeWidget->addTopLevelItem(c);
}

//회원 정보 수정 시 우측 라인Edit을 비우고 리스트를 새롭게 load하기 위해 비운다.
void ProductManager::clearClientWidget_N_LineEdit() {
    ui->userIdLineEdit->clear();
    ui->userNameLineEdit->clear();
    ui->userCallLineEdit->clear();
    ui->userAddressLineEdit->clear();
    ui->userGenderLineEdit->clear();

    ui->clientListTreeWidget->clear();
}

//제품 정보를 담아서 보내기(Shopping으로 보내기)
void ProductManager::containProductInfo() {
    int proNumber, proPrice, proCount;
    QString proName, proType;

    Q_FOREACH(auto v, productList) {
        Product *p = static_cast<Product*>(v);
        proNumber = p->getProNumber();
        proName = p->getProName();
        proPrice = p->getProPrice();
        proCount = p->getProCount();
        proType = p->getProType();

        Product *item = new Product(proNumber, proName, proPrice, proCount, proType);
        emit sendProductInfo(item);
    }
}

//쇼핑에서 주문하기 or 주문변경 시 재고 확인 및 관리
int ProductManager::updateAfterUpCount(QString name, int cnt) {
    int afterCount;
    Q_FOREACH(auto v, productList) {
        Product *p = static_cast<Product*>(v);
        if(name == p->getProName()) {
            afterCount = p->getProCount() - cnt;
            if(afterCount < 0) return -1;
            p->setProCount(afterCount);
            productList.insert(p->getProNumber(), p);
            break;
        }
    }
    return 0;
}

//쇼핑에서 주문변경, 주문취소 시 재고 관리
void ProductManager::updateAfterDownCount(QString name, int cnt) {
    int afterCount;
    Q_FOREACH(auto v, productList) {
        Product *p = static_cast<Product*>(v);
        if(name == p->getProName()) {
            afterCount = p->getProCount() + cnt;
            p->setProCount(afterCount);
            productList.insert(p->getProNumber(), p);
            break;
        }
    }
}


