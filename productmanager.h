#ifndef PRODUCTMANAGER_H
#define PRODUCTMANAGER_H

#include <QWidget>

class Product;
class Client;
class QTreeWidgetItem;

namespace Ui {
class ProductManager;
}

class ProductManager : public QWidget
{
    Q_OBJECT

public:
    explicit ProductManager(QWidget *parent = nullptr);
    ~ProductManager();

private slots:
    void on_productRegisterPushButton_clicked();    //제품 등록/변경 버튼 클릭 시 동작
    void on_productRemovePushButton_clicked();      //제품 삭제 버튼 클릭 시 동작
    void on_clientUpdatePushButton_clicked();       //회원 정보 수정 버튼 클릭 시 동작
    void on_clientRemovePushButton_clicked();       //회원 삭제 버튼 클릭 시 동작
    void on_getOutPushButton_clicked();             //나가기 버튼 클릭 시 동작

    void receivedClientInfo(Client*);
    void containProductInfo();
    void on_productListTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_clientListTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void clearClientWidget_N_LineEdit();
    int updateAfterUpCount(QString, int);
    void updateAfterDownCount(QString, int);

private:
    Ui::ProductManager *ui;

    QMap<int, Product*> productList;

signals:
    void quitProduct();
    void sendProductInfo(Product*);
    void updateBtnClicked(QStringList);
    void deleteBtnClicked(QString);
};

#endif // PRODUCTMANAGER_H
