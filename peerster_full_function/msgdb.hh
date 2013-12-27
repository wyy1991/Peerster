#ifndef MSGDB_HH
#define MSGDB_HH



#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QTextEdit>
#include <QHash>
#include <QPair>
#include <QHostAddress>

#define SEQ_NUM_START (1)

typedef enum
{
    RTN_OK = 0,
    RTN_ERR = 1,
    RTN_INVALID_MSG_ERR = 2,
    RTN_NO_LAST_PATH = 3,
    RTN_NO_ROUTE_UPDATE = 4,
    RTN_NOT_IN_FILE_CONTENT = 5,
    RTN_RPL_CONTENT_MISSMATCH = 6,
    RTN_RQST_BLOCK_END = 7,
    RTN_NO_KEYWORD = 8,
    RTN_NO_MATCHING_FILE = 9,
    RTN_RPLY_NOT_FOR_CURR_RQST = 10

}RTN_STATUS;

typedef enum
{
    MSG_ST_EXACTLY_NEEDED = 0,
    MSG_ST_ALREADY_RECEIVED = 1,
    MSG_ST_RECEIVE_LATER = 2,
    MSG_ST_DB_EMPTY = 3,
    MSG_ST_DB_NO_SUCH_ORIGIN = 4

}MSG_IN_DB_STATUS;

typedef enum
{
    ROUTE_INSERT = 0,
    ROUTE_UPDATE = 1

}ROUTE_ST;

class MsgDB
{
public:
    MsgDB();

    int insertMsg(QVariantMap singleMsg);
    void updateDbStatus();
    quint32 getSeqNo(QString originName);
    int insertDbStatus(QString origin, quint32 seq);
    int checkMsginDBStatus(QVariantMap msgMap);
    QMap<QString, QVariantMap> compareStatusMap(QVariantMap stMap);
    QVariantMap getDBStatus();
    QMap < QString, QList< QVariantMap > > * getDataBase();
    void printStatusMessage(QTextEdit * textView);
    int insertRouteToTable(QString origin, QHostAddress addr, quint16 port);
    QStringList outputRouteTable();
    QPair <QHostAddress, quint16> getRouteContent(QString origin);
    int updateDirectRoute(QVariantMap rummorMsg, QHostAddress sender, quint16 senderPort);
    QString getOriginName(QHostAddress ip, quint16 port);

private:
    // current database info

    // current database status
    QVariantMap *dbStatus;
    // structure to store all QVariantMap
    QMap < QString, QList< QVariantMap > > *database;
    // next-hop routing table
    QHash < QString, QPair< QHostAddress, quint16> > *routingTable;

};

#endif // MSGDB_HH
