#ifndef SHOPPINGMANAGER_H
#define SHOPPINGMANAGER_H

#include <QWidget>

class Shopping;
class Product;

namespace Ui {
class ShoppingManager;
}

class ShoppingManager : public QWidget
{
    Q_OBJECT

public:
    explicit ShoppingManager(QWidget *parent = nullptr);
    ~ShoppingManager();

    void dataLoad();
    void dataClear();

    void loadShoppingWidget(QString);

private slots:
    void on_pushButton_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_4_clicked();

    void receivedProductInfo(Product*);

    void successLoginCheck(QString);
    void failedLoginCheck();

private:
    Ui::ShoppingManager *ui;

    int shoppingCount();
    QMap<int, Shopping*> shoppingList;

signals:
    void newClient();
    void onlyStaff();
    void exitShopping();
    void viewClientList();
    void viewProductList();
    void login(QString);
    QString takeOrderSign(QString);
};

#endif // SHOPPINGMANAGER_H
