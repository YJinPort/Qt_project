#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "clientmanager.h"
#include "productmanager.h"
#include "shoppingmanager.h"
#include "serverside.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->resize(1400, 800);

    shoppingManager = new ShoppingManager(this);
    clientManager = new ClientManager(this);
    productManager = new ProductManager(this);
    //server = new ServerSide();

    ui->mdiArea->addSubWindow(shoppingManager);
    ui->mdiArea->addSubWindow(clientManager);
    ui->mdiArea->addSubWindow(productManager);
    shoppingManager->showMaximized();

    connect(shoppingManager, SIGNAL(newClient()), this, SLOT(openClientWindow()));
    connect(shoppingManager, SIGNAL(onlyStaff()), this, SLOT(openProductWindow()));
    connect(shoppingManager, SIGNAL(exitShopping()), this, SLOT(close()));
    connect(shoppingManager, SIGNAL(viewClientList()), clientManager, SLOT(containClientInfo()));
    connect(shoppingManager, SIGNAL(viewProductList()), productManager, SLOT(containProductInfo()));
    connect(shoppingManager, SIGNAL(login(QString)), clientManager, SLOT(checkLoginId(QString)));
    connect(shoppingManager, SIGNAL(takeOrderSign(QString)), clientManager, SLOT(findAddressForOrder(QString)));
    connect(shoppingManager, SIGNAL(updateAfter_upCount(QString, int)), productManager, SLOT(updateAfterUpCount(QString, int)));
    connect(shoppingManager, SIGNAL(updateAfter_downCount(QString, int)), productManager, SLOT(updateAfterDownCount(QString, int)));
    connect(shoppingManager, SIGNAL(deleteClient(QString)), clientManager, SLOT(deleteId_List(QString)));
    connect(shoppingManager, SIGNAL(serverBtnClicked()), clientManager, SLOT(serverOpenFromShopping()));
    connect(shoppingManager, SIGNAL(serverInputComboBox()), clientManager, SLOT(sendNameListToServer()));

    connect(clientManager, SIGNAL(cancellation()), this, SLOT(cancellationClient()));
    connect(clientManager, SIGNAL(join()), this, SLOT(joinClient()));
    connect(clientManager, SIGNAL(sendClientInfo(Client*)), productManager, SLOT(receivedClientInfo(Client*)));
    connect(clientManager, SIGNAL(successLogin(QString)), shoppingManager, SLOT(successLoginCheck(QString)));
    connect(clientManager, SIGNAL(failedLogin()), shoppingManager, SLOT(failedLoginCheck()));
    connect(clientManager, SIGNAL(clear_Widget_N_LineEdit()), productManager, SLOT(clearClientWidget_N_LineEdit()));
    //connect(clientManager, SIGNAL(sendToServer(int, QString)), server, SLOT(addClient(int, QString)));
    //client -> shopping
    connect(clientManager, SIGNAL(sendToServer(QString, QString)), shoppingManager, SLOT(clientSignalReceived(QString, QString)));
    connect(clientManager, SIGNAL(sendNameToServer(QStringList)), shoppingManager, SLOT(inputNameServerCombobox(QStringList)));

    //connect(this, signal2, server, slot(addclient));

    connect(productManager, SIGNAL(quitProduct()), this, SLOT(quitProductWindow()));
    connect(productManager, SIGNAL(sendProductInfo(Product*)), shoppingManager, SLOT(receivedProductInfo(Product*)));
    connect(productManager, SIGNAL(updateBtnClicked(QStringList)), clientManager, SLOT(updateClientInfo(QStringList)));
    connect(productManager, SIGNAL(deleteBtnClicked(QString)), clientManager, SLOT(deleteClientInfo(QString)));

    shoppingManager->dataLoad();
}

void MainWindow::openClientWindow() {
    clientManager->setFocus();
}

void MainWindow::joinClient() {
    shoppingManager->setFocus();
}

void MainWindow::cancellationClient() {
    shoppingManager->setFocus();
}

void MainWindow::openProductWindow() {
    productManager->setFocus();
}

void MainWindow::quitProductWindow() {
    shoppingManager->dataClear();
    shoppingManager->dataLoad();
    shoppingManager->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}
