#include "serverside.h"
#include "ui_serverside.h"
#include "logdata.h"
#include <QPushButton>
#include <QBoxLayout>
#include <QTcpServer>
#include <QTcpSocket>
#include <QApplication>
#include <QMessageBox>
#include <QScrollBar>
#include <QDateTime>
#include <QDebug>
#include <QMenu>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>

ServerSide::ServerSide(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerSide), totalSize(0), byteReceived(0)
{
    ui->setupUi(this);  //생성한 Ui를 현재 클래스(this)위에 올리는 역할을 한다.

    QList<int> sizes;
    sizes << 220 << 500;
    ui->splitter->setSizes(sizes);

    chatServer = new QTcpServer(this);
    connect(chatServer, SIGNAL(newConnection()), SLOT(clientConnect()));
    if (!chatServer->listen(QHostAddress::Any, PORT_NUMBER)) {
        QMessageBox::critical(this, tr("Chatting Server"), tr("Unable to start the server: %1.").arg(chatServer->errorString( )));

        return;
    }

    /*채팅과 파일의 서버를 이원화한 이유: 파일 전송 중에도 채팅을 할 수 있도록 하기 위해 이원화한다.*/

    fileServer = new QTcpServer(this);
    connect(fileServer, SIGNAL(newConnection()), SLOT(acceptConnection()));
    if (!fileServer->listen(QHostAddress::Any, PORT_NUMBER+1)) {
        QMessageBox::critical(this, tr("Chatting Server"), tr("Unable to start the server: %1.").arg(fileServer->errorString( )));

        return;
    }

    qDebug("Start listening ...");

    QAction* inviteAction = new QAction(tr("&Invite"));
    inviteAction->setObjectName("Invite");
    connect(inviteAction, SIGNAL(triggered()), SLOT(inviteClient()));

    QAction* removeAction = new QAction(tr("&Kick out"));
    connect(removeAction, SIGNAL(triggered()), SLOT(kickOut()));

    menu = new QMenu;
    menu->addAction(inviteAction);
    menu->addAction(removeAction);
    ui->clientTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    progressDialog = new QProgressDialog(0);
    progressDialog->setAutoClose(true);
    progressDialog->reset();

    logData = new LogData(this);
    logData->start();

    connect(ui->savePushButton, SIGNAL(clicked()), logData, SLOT(saveData()));

    qDebug() << tr("The server is running on port %1.").arg(chatServer->serverPort());

    setWindowTitle(tr("Server_Admin"));
}

ServerSide::~ServerSide()
{
    delete ui;

    logData->terminate();
    chatServer->close();
    fileServer->close();
}

void ServerSide::clientConnect( )
{
    QTcpSocket *clientConnection = chatServer->nextPendingConnection( );

    connect(clientConnection, SIGNAL(readyRead()), SLOT(receiveData()));

    //연결이 끊어진 경우 실행
    connect(clientConnection, SIGNAL(disconnected()), SLOT(removeClient()));
    qDebug("new connection is established...");
}

void ServerSide::receiveData( )
{
    /*어떤 클라이언트가 데이터를 보낸 것인지 구분하는 역할*/
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender());   //sender: 연결 되어있는 소켓, 시그널을 보낼 객체(clientconnect에서)
    QByteArray bytearray = clientConnection->read(BLOCK_SIZE);

    Chat_Status type;       // 채팅의 목적
    char data[1020];        // 전송되는 메시지/데이터
    memset(data, 0, 1020);  //memst이 쓰레기값이 들어가기 때문에 0으로 초기화

    QDataStream in(&bytearray, QIODevice::ReadOnly);
    in.device()->seek(0);
    in >> type;
    in.readRawData(data, 1020);

    QString ip = clientConnection->peerAddress().toString();
    quint16 port = clientConnection->peerPort();
    QString name = QString::fromStdString(data);

    qDebug() << ip << " : " << type;

    switch(type) {
    case Chat_Login:
        int idx;
        foreach(auto item, ui->clientQuitTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            idx = ui->clientQuitTreeWidget->indexOfTopLevelItem(item);
            ui->clientQuitTreeWidget->takeTopLevelItem(idx);
            item->setIcon(0, QIcon("stay.png"));
            item->setText(1, name);
            item->setText(2, "대기중");
            ui->clientTreeWidget->addTopLevelItem(item);
            ui->clientTreeWidget->resizeColumnToContents(0);

            clientSocketHash[name] = clientConnection;  //이름에다가 클라이언트 소켓을 연결 - 누구의 소켓인지 구별하기 위함
        }
        break;

    case Chat_In:
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            if(item->text(2) != "채팅중") {
                item->setIcon(0, QIcon("talk.png"));
                item->setText(2, "채팅중");
            }
            clientNameHash[port] = name;
            if(clientSocketHash.contains(name))
                clientSocketHash[name] = clientConnection;
        }
        break;

    /*메시지를 보낼 경우*/
    case Chat_Talk: {
        foreach(QTcpSocket *sock, clientSocketHash.values()) {
            qDebug() << sock->peerPort();
            /*보낸 사용자를 제외한 모든 사용자에게 채팅한 내용을 보낸다.*/
            if(clientNameHash.contains(sock->peerPort()) && port != sock->peerPort()) {
                QByteArray sendArray;
                sendArray.clear();
                QDataStream out(&sendArray, QIODevice::WriteOnly);
                out << Chat_Talk;
                sendArray.append("<font color=lightsteelblue>");
                sendArray.append(clientNameHash[port].toStdString().data());
                sendArray.append("</font> : ");
                sendArray.append(name.toStdString().data());
                sock->write(sendArray);
                qDebug() << sock->peerPort();
            }
        }

        /*채팅한 내역에 대한 로그를 남긴다.*/
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget);
        item->setText(0, ip);
        item->setText(1, QString::number(port));
        //item->setText(2, QString::number(clientIDHash[clientNameHash[port]]));
        item->setText(2, clientIDHash[clientNameHash[port]]);
        item->setText(3, clientNameHash[port]);
        qDebug() << port << "   //   " << clientIDHash[clientNameHash[port]] << "   //   " << clientNameHash[port];
        item->setText(4, QString(data));
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, QString(data));
        ui->messageTreeWidget->addTopLevelItem(item);

        /*로그에 표현될 컬럼의 사이즈를 메시지의 길이에 따라 결정한다.*/
        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        logData->appendData(item);  //로그파일로의 저장을 위해 채팅 내역을 보낸다.
    }
        break;

    case Chat_Out:
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {
            if(item->text(2) != "대기중") {
                item->setIcon(0, QIcon("stay.png"));
                item->setText(2, "대기중");
            }
            clientNameHash.remove(port);
        }
        break;

    case Chat_LogOut:
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {
            idx = ui->clientTreeWidget->indexOfTopLevelItem(item);
            ui->clientTreeWidget->takeTopLevelItem(idx);
            item->setIcon(0, QIcon("disconnect.png"));
            item->setText(1, name);
            item->setText(2, "종료");
            ui->clientQuitTreeWidget->addTopLevelItem(item);

            clientSocketHash.remove(name);
        }
        break;

    }
}

//클라이언트가 종료되었을 경우
void ServerSide::removeClient()
{
    QTcpSocket *clientConnection = dynamic_cast<QTcpSocket *>(sender( ));
    if(clientConnection != nullptr) {
        QString name = clientNameHash[clientConnection->peerPort()];
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchContains, 1)) {
            item->setIcon(0, QIcon("disconnect.png"));
            item->setText(2, "종료");
        }
        clientSocketHash.remove(name);
        clientConnection->deleteLater();    //서버에서 올 데이터가 남아있을 수 있기 때문에 deleteLater를 사용하였다.
    }
}

//클라이언트가 추가되었을 경우
void ServerSide::addClient(QString id, QString name)
{
    //QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientTreeWidget);
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->clientQuitTreeWidget);
    item->setIcon(0, QIcon("disconnect.png"));
    item->setText(1, name);
    item->setText(2, "종료");
    //ui->clientTreeWidget->addTopLevelItem(item);
    ui->clientQuitTreeWidget->addTopLevelItem(item);
    clientIDHash[name] = id;
    //ui->clientTreeWidget->resizeColumnToContents(0);
    ui->clientQuitTreeWidget->resizeColumnToContents(0);

    //ui->clientTreeWidget->setCurrentItem(nullptr);
    ui->clientQuitTreeWidget->setCurrentItem(nullptr);
}

void ServerSide::on_clientTreeWidget_customContextMenuRequested(const QPoint &pos)
{
    if(ui->clientTreeWidget->currentItem()) {
        foreach(QAction *action, menu->actions()) {
            if(action->objectName() == "Invite")        // 초대
                action->setEnabled(ui->clientTreeWidget->currentItem()->text(2) == "대기중");
            else                                        // 강퇴
                action->setEnabled(ui->clientTreeWidget->currentItem()->text(2) == "채팅중");
        }
        QPoint globalPos = ui->clientTreeWidget->mapToGlobal(pos);
        menu->exec(globalPos);
    }
    else return;
}

/* 클라이언트 강퇴하기 */
void ServerSide::kickOut()
{
    QByteArray sendArray;
    QDataStream out(&sendArray, QIODevice::WriteOnly);
    out << Chat_KickOut;
    out.writeRawData("", 1020);

    if(ui->clientTreeWidget->currentItem() != nullptr) {
        QString name = ui->clientTreeWidget->currentItem()->text(1);
        QTcpSocket* sock = clientSocketHash[name];  //사용자 이름으로 해당 사용자에게 연결된 소켓을 찾는다.
        sock->write(sendArray);

        ui->clientTreeWidget->currentItem()->setIcon(0, QIcon("stay.png"));
        ui->clientTreeWidget->currentItem()->setText(2, "대기중");
    }
}

/* 클라이언트 초대하기 */
void ServerSide::inviteClient()
{
    if(ui->clientTreeWidget->topLevelItemCount()) {
        QString name = ui->clientTreeWidget->currentItem()->text(1);    //대기 회원 리스트에서 우클릭한 회원의 이름을 담는다.

        /*클라이언트로 초대한 상태를 ByteArray타입으로 상태와 이름을 전송*/
        QByteArray sendArray;
        QDataStream out(&sendArray, QIODevice::WriteOnly);
        out << Chat_Invite;
        qDebug() << "out" << name;
        out.writeRawData("", 1020);

        QTcpSocket* sock = clientSocketHash[name];  //회원 이름으로 현재 연결된 소켓을 찾는다.

        if (sock == nullptr) return;    //회원이 선택되어 있지 않으면 Invite(초대), Kick out(강퇴) 메뉴가 표시되지 않는다.

        sock->write(sendArray);

        /*이름을 트리위젯에서 검색해서 고객의 현재 상태 아이콘을 Chat in으로 변경*/
        foreach(auto item, ui->clientTreeWidget->findItems(name, Qt::MatchFixedString, 1)) {
            /*고객이 채팅방에 들어왔다고 알리는 아이콘 변경*/
            if(item->text(2) != "채팅중") {
                item->setText(2, "채팅중");
                item->setIcon(0, QIcon("talk.png"));
            }
        }

        /*소켓의 포트 번호를 키값으로 지정하여 회원 이름을 clientNameHash에 저장한다.*/
        quint64 port = sock->peerPort();
        clientNameHash[port] = name;
    }
}

/* 파일 전송을 위한 소켓 생성 */
void ServerSide::acceptConnection()
{
    qDebug("Connected, preparing to receive files!");

    QTcpSocket* receivedSocket = fileServer->nextPendingConnection();
    connect(receivedSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
}

/* 파일 전송 */
void ServerSide::readClient()
{
    qDebug("Receiving file ...");
    QTcpSocket* receivedSocket = dynamic_cast<QTcpSocket *>(sender( ));
    QString filename, name;

    if (byteReceived == 0) {        // 파일 전송 시작 : 파일에 대한 정보를 이용해서 QFile 객체 생성
        progressDialog->reset();
        progressDialog->show();

        QString ip = receivedSocket->peerAddress().toString();
        quint16 port = receivedSocket->peerPort();
        qDebug() << ip << " : " << port;

        QDataStream in(receivedSocket);
        in >> totalSize >> byteReceived >> filename >> name;
        progressDialog->setMaximum(totalSize);

        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget);
        item->setText(0, ip);
        item->setText(1, QString::number(port));
        //item->setText(2, QString::number(clientIDHash[name]));
        item->setText(2, clientIDHash[name]);
        item->setText(3, name);
        item->setText(4, filename);
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, filename);

        /* 컨텐츠의 길이로 QTreeWidget의 헤더의 크기를 고정 */
        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        ui->messageTreeWidget->addTopLevelItem(item);
        logData->appendData(item);

        QFileInfo info(filename);
        QString currentFileName = info.fileName();  //경로에서 이름만 가져온다. 이유: 보안상의 이유
        file = new QFile(currentFileName);
        //file->open(QFile::WriteOnly);
        file->open(QFile::WriteOnly | QFile::Truncate);   //덮어쓰기까지 추가
    } else {                    // 파일 데이터를 읽어서 저장
        inBlock = receivedSocket->readAll();

        byteReceived += inBlock.size();
        file->write(inBlock);
        file->flush();
    }

    progressDialog->setValue(byteReceived);

    if (byteReceived == totalSize) {        /* 파일의 다 읽으면 QFile 객체를 닫고 삭제 */
        qDebug() << QString("%1 receive completed").arg(filename);

        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;
        progressDialog->reset();
        progressDialog->hide();

        file->close();
        delete file;
    }
}

//exit 버튼을 클릭했을 경우 실행
void ServerSide::on_exitPushButton_clicked()
{
    logData->terminate();
    chatServer->close();
    fileServer->close();

    close();
}

//windowTitle단의 X를 클릭하여 종료할 경우 실행
void ServerSide::closeEvent(QCloseEvent*) {
    logData->terminate();
    chatServer->close();
    fileServer->close();

    close();
}

//메시지 콤보 박스를 채우기 위해 회원 이름을 받아오는 SLOT 함수
void ServerSide::inputNameComboBox(QStringList nameList) {
    /*메시지 콤보 박스의 0번째 인덱스의 글자는 전체, 이후는 회원 이름으로 채운다.*/
    ui->clientListComboBox->addItem("전체");
    ui->clientListComboBox->addItems(nameList);
}

//서버가 클라이언트에게 채탕하기
void ServerSide::on_sendPushButton_clicked()
{
    if(ui->clientTreeWidget->topLevelItemCount()) {
        /*전체 공지로 메시지를 보낼 경우*/
        if(ui->clientListComboBox->currentIndex() == 0) {
            foreach(QTcpSocket *sock, clientSocketHash.values()) {
                QByteArray sendArray;
                sendArray.clear();
                QDataStream out(&sendArray, QIODevice::WriteOnly);
                out << Chat_Talk;
                sendArray.append("<font color=blue>");
                sendArray.append("Administrator");
                sendArray.append("</font> : ");
                sendArray.append(ui->sendLineEdit->text().toStdString().data());
                sock->write(sendArray);
            }
        }
        /*개인으로 메시지를 보낼 경우*/
        else {
            QString name = ui->clientListComboBox->currentText();
            if(clientSocketHash[name] == nullptr) return;
            QTcpSocket *sock = clientSocketHash[name];
            QByteArray sendArray;
            sendArray.clear();
            QDataStream out(&sendArray, QIODevice::WriteOnly);
            out << Chat_Talk;
            sendArray.append("<font color=blue>");
            sendArray.append("Administrator");
            sendArray.append("</font> : ");
            sendArray.append(ui->sendLineEdit->text().toStdString().data());
            sock->write(sendArray);
        }

        /*채팅한 내역에 대한 로그를 남긴다.*/
        QTreeWidgetItem* item = new QTreeWidgetItem(ui->messageTreeWidget);
        item->setText(0, "Host");
        item->setText(1, QString::number(PORT_NUMBER));
        //item->setText(2, QString::number(clientIDHash[clientNameHash[port]]));
        item->setText(2, "0");
        item->setText(3, "Administrator");
        item->setText(4, ui->sendLineEdit->text());
        item->setText(5, QDateTime::currentDateTime().toString());
        item->setToolTip(4, ui->sendLineEdit->text());
        ui->messageTreeWidget->addTopLevelItem(item);

        /*로그에 표현될 컬럼의 사이즈를 메시지의 길이에 따라 결정한다.*/
        for(int i = 0; i < ui->messageTreeWidget->columnCount(); i++)
            ui->messageTreeWidget->resizeColumnToContents(i);

        logData->appendData(item);  //로그파일로의 저장을 위해 채팅 내역을 보낸다.
    }

    ui->sendLineEdit->clear();
}

