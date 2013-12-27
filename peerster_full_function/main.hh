#ifndef PEERSTER_MAIN_HH
#define PEERSTER_MAIN_HH




#include <unistd.h>
#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QVariant>
#include <QString>
#include <QMap>
#include <QDataStream>
#include <QFile>
#include <QByteArray>
#include <QKeyEvent>
#include <QHostInfo>
#include <QTimer>
#include <QLabel>
#include <QTime>
#include <QFileDialog>
#include <QtCrypto>
#include <QtCore/qmath.h>

#include "msgdb.hh" //@@@ where should I include this??
#include "peer.hh"
#include "file.hh"
#include "filedb.hh"
#include "searchfile.hh"
#include "recommend.hh"


#define TIMER_LENGTH (1000)  //ms
#define HOP_LIMIT (10)
#define INIT_BUDGET (2) // initial budget for search //@@@ testcode
#define TIME_INC_BUDGET (1000) // ms
#define STOP_RQST_SEARCH_R_NUM (10)
#define MAX_BUDGET (50)

Q_DECLARE_METATYPE(QList<QString>);

typedef enum
{
    MTYPE_CHAT_RUMMOR = 0,
    MTYPE_ROUTE_RUMMOR = 1,
    MTYPE_STATUS = 2,
    MTYPE_PRIVATE = 3,
    MTYPE_BLOCK_REQUEST = 4,
    MTYPE_BLOCK_REPLY = 5,
    MTYPE_SEARCH_REQUEST = 6,
    MTYPE_SEARCH_REPLY = 7,
    MTYPE_WHOLIKE_RQST = 8, // ask server for who liked the file
    MTYPE_WHOLIKE_REPLY = 9,
    MTYPE_LIKE_FILE = 10,  // send to server to like/unlike one file
    MTYPE_ERR = 11

} MAP_TYPE;


//******************************************************
class PrivateD : public QDialog
{
    Q_OBJECT

public:
    PrivateD();
    void p_setPrivateOrigin(QString origin);
    void p_showMessage(QString chatTxt);

signals:
    void gotPrivateInput(QString origin, QString text);

private:
    QLabel *title ;
    QTextEdit *textChat;
    QTextEdit *textInput;
    QString targetOrigin;

    //QVariantMap createPrivateMsg(QString chatText);
    void p_GotReturnPressed();


protected:
    bool eventFilter(QObject *obj, QEvent *event);

};




//******************************************************
class NetSocket : public QUdpSocket
{
    Q_OBJECT

public:
    NetSocket();

    // Bind this socket to a Peerster-specific default port.
    bool bind();
    int getMyPortMin();
    int getMyPortMax();
    QString getLocalHostName();
    quint16 getLocalPortNumber();
    QString getSocketOriginName();
    void setSocketOriginName();
    QHostAddress getLocalIP();


private:
    int myPortMin, myPortMax;
    QString hostName;
    quint16 portNumber;
    QString socketOriginName;
};



//******************************************************
class ChatDialog : public QDialog
{
	Q_OBJECT

public:
    // put into QDataStream
	ChatDialog();
    QVariantMap createNewMsg(QString inputMsg);
    QByteArray createByteArrayFromMsg(QVariantMap msgMap);


    void afterSavedNewMsg_sendToOneNB(QVariantMap newMsg);
    int checkDiagramType(QByteArray byteArray);
    void sendMsg(QVariantMap msg, QHostAddress tarAddr, quint16 tarPort);
    QVariantMap getMsgInfo(QString origin, quint32 seq);
    bool msgIsValid(QVariantMap msg);
    void pickNBSendStatus();
    void initPeerList();
    Peer pickRandomNeighbour();
    void addPeer(QString inputPeer, bool isServer);
    void showPeers();

    QVariantMap createNewRouteRummor();
    void insertRoute(QString origin, QHostAddress address, quint16 port);
    void insertRummorMsgLastIPPort(QVariantMap *msgptr, QHostAddress lasthost, quint16 lastport);

public slots:
    void gotReturnPressed();
    void readPendingDatagrams();
    void timeOutAntiEntropy();
    void timeOutHandler();
    void timeOutRepeatSearch();
    void flipCoin();

    void addNewNeighbour();
    void lookedUp(const QHostInfo &host);
    void timerRouteRumorHandler();
    void startPrivateCon();
    void selectedChanged(QListWidgetItem *);
    void doubleClickedWidgetItem(QListWidgetItem* item);
    void doubleClickedSearchResult(QListWidgetItem* item);
    void gotTextFromPrivate(QString origin, QString txt);
    void shareFileClicked();
    void serverShareClicked();
    void downloadFileClicked();
    void searchButtonClicked();

    // recommend
    void addNewNbAndSetServer();
    void clickedServerSharedFile(QListWidgetItem* item);
    void likeClicked();
    void dislikeClicked();
    void wholikeClicked();
private:
    //msg layout
    QTextEdit *textview; // mainoutput
    QTextEdit *textline; // input
    QTextEdit *textPeers;
    QLineEdit *inputPeer;
    QPushButton *buttonAddPeer;
    QListWidget *listWidget;
    QPushButton *buttonPrivateCon;
    //share file layout
    QPushButton *buttonShareFile;
    QPushButton *buttonRequestFile;
    QTextEdit *textTargetID;
    QTextEdit *textFileID;
    //search file layout
    QLineEdit *lineKeyW;
    QPushButton *buttonSearch;
    QListWidget *searchResult;
    //server layout
    QPushButton *buttonShare;
    QListWidget *listFileShared;
    QListWidget *textVotedClients;

    //client layout
    QListWidget *listNodeTrust;



    // basic
    NetSocket *netSocket;
    MsgDB *msgDatabase;
    QTimer *timer_anti_entropy;
    QTimer *timer_route_rumor;
    QTimer *timer_search;
    QList<Peer> *peerList;
    PrivateD *pDialog;
    bool noForwardMode;
    bool recServerMode;
    bool recClientMode;

    FileDB *fileDBlocal;
    QHash <QByteArray, FileRcving> *receivingFiles; //receivingfiles
    SearchFile *searchStatus;

    void checkAndInsertPeer(QHostAddress address, quint16 port);
    QVariantMap createPrivateMessage(QString targetName, QString inputTxt);
    void checkNoForwardMode();
    int rummorCheckLast_addToPeers(QVariantMap msgMap);
    void sendRouteToAllPeers(QVariantMap routeRummor);


    QVariantMap createBlockRequest(QString target,  QByteArray bkHash);
    QVariantMap createBlockReply(QString target, QString originName, QByteArray rqstRpl, QByteArray data);
    QVariantMap createSearchRequest(QString keyword);
    QVariantMap createSearchReply(QString dest, QString searchkey, QVariantList matchNames, QVariantList matchIDs);

    int gotStatusDatagram(QByteArray statusByteArray,
                          QHostAddress sender, quint16 senderPort);
    int processChatRummorDatagram(QByteArray msgByteArray,
                           QHostAddress sender, quint16 senderPort);
    int processRouteRummorDatagram(QByteArray msgByteArray,
                           QHostAddress sender, quint16 senderPort);
    int processPrivateMsg(QByteArray msgByteArray,
                          QHostAddress sender, quint16 senderPort);
    int processBlockRequest(QByteArray msgByteArray,
                          QHostAddress sender, quint16 senderPort);
    int processBlockReply(QByteArray msgByteArray,
                          QHostAddress sender, quint16 senderPort);
    int processSearchRequest(QByteArray msgByteArray,
                             QHostAddress sender, quint16 senderPort);
    int processSearchReply(QByteArray msgByteArray,
                           QHostAddress sender, quint16 senderPort);

    int requestNextFileBock(QString dest, QByteArray fileID);
    void sendSearchRequest(QVariantMap searchRequest);
    void forwardSearchReqest_Bgt(QVariantMap searchRequest);
    void requestToDownload(QByteArray fileID, QString targetName);



    //recommendation


    Recommend * trustRec;
    QPair <QHostAddress, quint16> * serverAddress;
    QPushButton *buttonSetServer;
    QPushButton *buttonLike;
    QPushButton *buttonWhoLike;
    QPushButton *buttonDislike;

    void requestWhoLikeTheFile(QByteArray fileid);
    QVariantMap createWhoLikeRequest(QByteArray fileID);
    QVariantMap createWhoLikeReply(QString dest, QByteArray fid);

    int processWhoLikeRequest(QByteArray msgByteArray,
                              QHostAddress sender, quint16 senderPort);
    int processWhoLikeReply(QByteArray msgByteArray,
                              QHostAddress sender, quint16 senderPort);

    int processLikeFile(QByteArray msgByteArray,
                        QHostAddress sender, quint16 senderPort);
    void likeFile(QByteArray fileid);
    QVariantMap createLikeMsg(QByteArray fileid, quint32 like);
    void updateListFileShared();
    void checkServerAddress();
    void updateLocalTrustList();

    void updateSearchResultWidget();
    void updateSearchResultWidget_Rec(QByteArray recFileID);

    void checkRecServerMode();
    void checkRecClientMode();


protected:
    bool eventFilter(QObject *obj, QEvent *event);
};




#endif // PEERSTER_MAIN_HH
