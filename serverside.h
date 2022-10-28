#ifndef SERVERSIDE_H
#define SERVERSIDE_H

#include <QWidget>
#include <QList>
#include <QHash>

class QLabel;
class QTcpServer;
class QTcpSocket;
class QFile;
class QProgressDialog;
class LogData;

namespace Ui {
class ServerSide;
}

typedef enum {
    Chat_Login,             // 로그인(서버 접속)   --> 초대를 위한 정보 저장
    Chat_In,                // 채팅방 입장
    Chat_Talk,              // 채팅
    Chat_Out,               // 채팅방 퇴장         --> 초대 가능
    Chat_LogOut,            // 로그 아웃(서버 단절) --> 초대 불가능
    Chat_Invite,            // 초대
    Chat_KickOut,           // 강퇴
} Chat_Status;

class ServerSide : public QWidget
{
    Q_OBJECT

public:
    explicit ServerSide(QWidget *parent = nullptr);
    ~ServerSide();

private:
    const int BLOCK_SIZE = 1024;
    const int PORT_NUMBER = 8000;

    Ui::ServerSide *ui;

    QTcpServer *chatServer;
    QTcpServer *fileServer;
    QHash<quint16, QString> clientNameHash;
    QHash<QString, QTcpSocket*> clientSocketHash;
    QHash<QString, QString> clientIDHash;
    QMenu* menu;
    QFile* file;
    QProgressDialog* progressDialog;
    qint64 totalSize;
    qint64 byteReceived;
    QByteArray inBlock;
    LogData* logData;

    void closeEvent(QCloseEvent*) override; //윈도우Title 창의 X를 눌러 종료하였을 경우 처리하는 이벤트 핸들러

private slots:
    void acceptConnection();                /* 파일 서버 */
    void readClient();

    void clientConnect( );                  /* 채팅 서버 */
    void receiveData( );
    void removeClient( );
    void addClient(QString, QString);
    void inviteClient();
    void kickOut();
    void on_clientTreeWidget_customContextMenuRequested(const QPoint &pos);

    //void receivedClientInfo(int, QString);
    void on_exitPushButton_clicked();
    void on_sendPushButton_clicked();
    void inputNameComboBox(QStringList);
};

#endif // SERVERSIDE_H
