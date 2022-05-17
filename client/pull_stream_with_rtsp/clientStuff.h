#ifndef CLIENTSTUFF_H
#define CLIENTSTUFF_H

#include <QString>
#include <QTcpSocket>
#include <QDataStream>
#include <QTimer>

// add
#include <QStandardPaths>
#include <QMetaType>
#include <QMessageBox>
#include <QHostAddress>
#include <QFileDialog>
#include <QAbstractSocket>
#include <QDebug>
#include <QFile>

class ClientStuff : public QObject
{
    Q_OBJECT

public:
    ClientStuff(const QString hostAddress, int portVal, QObject *parent = 0);

    QTcpSocket *tcpSocket;
    bool getStatus();

public slots:
    void closeConnection();
    void connect2host();

signals:
    void statusChanged(bool);
    void hasReadSome(QString msg);

private slots:
    void readyRead();
    void connected();
    void connectionTimeout();

private:
    QString host;
    int port;
    quint16 m_nNextBlockSize;
    QTimer *timeoutTimer;

public:
    bool status;

};

#endif // CLIENTSTUFF_H
