#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTcpSocket>
#include <QPainter>
#include <QInputDialog>
#include <QtMath>
#include<iostream>

using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pushButton_disconnect->setVisible(false);

    client = new ClientStuff("121.5.5.221", 12160);
//    client = new ClientStuff("192.168.1.101",8080);

    setStatus(client->getStatus());

    connect(client, &ClientStuff::hasReadSome, this, &MainWindow::receivedSomething);
    connect(client, &ClientStuff::statusChanged, this, &MainWindow::setStatus);
    // FIXME change this connection to the new syntax
    connect(client->tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(gotError(QAbstractSocket::SocketError)));

    // for codec
    mPlayer = new VideoPlayer;
    connect(mPlayer,SIGNAL(sig_GetOneFrame(QImage)),this,SLOT(slotGetOneFrame(QImage)));
    mPlayer->startPlay();

    // centos label style
    ui->label_centos->setText(tr("<font color=\"black\">Centos's Response:</font>"));
    ui->label_centos->setStyleSheet("QLabel{background:#FFFFF0; border:2px solid rgb(255, 228, 181);}");

    ui->label_status_title->setText(tr("<font color=\"black\">Centos's Status:</font>"));
    ui->label_status_title->setStyleSheet("QLabel{background:#FFFFF0; border:2px solid rgb(255, 228, 181);}");
}

MainWindow::~MainWindow()
{
    delete client;
    delete ui;
}

void MainWindow::setStatus(bool newStatus)
{
    if(newStatus)
    {
        ui->label_status->setText(
                    tr("<font color=\"green\">CONNECTED</font>"));
        ui->pushButton_connect->setVisible(false);
//        ui->pushButton_connect->setFlat(true);
        ui->pushButton_disconnect->setVisible(true);
    }
    else
    {
        ui->label_status->setText(
                tr("<font color=\"red\">DISCONNECTED</font>"));
        ui->pushButton_connect->setVisible(true);
        ui->pushButton_disconnect->setVisible(false);
    }
}

void MainWindow::receivedSomething(QString msg)
{
//    ui->textEdit_log->append(msg);
    ui->lineEdit_log->setText(msg);
}

void MainWindow::gotError(QAbstractSocket::SocketError err)
{
    QString strError = "unknown";
    switch (err)
    {
        case 0:
            strError = "Connection was refused";
            break;
        case 1:
            strError = "Remote host closed the connection";
            break;
        case 2:
            strError = "Host address was not found";
            break;
        case 5:
            strError = "Connection timed out";
            break;
        default:
            strError = "Unknown error";
    }

    ui->lineEdit_log->setText(strError);

}

void MainWindow::on_pushButton_connect_clicked()
{
    client->connect2host();
}

void MainWindow::on_pushButton_send_clicked()
{
    // 适配centos string
    QString textEdit = ui->lineEdit_message->text();
    QByteArray sendMessage = textEdit.toLocal8Bit();
    client->tcpSocket->write(sendMessage);
    ui->lineEdit_message->clear();
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    client->closeConnection();
}

// for codec
void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setBrush(Qt::white);
    painter.drawRect(0, 0, 1024, 768); //先画成白色
    if (mImage.size().width() <= 0) return;
    if(client->status)
        painter.drawImage(QPoint(0,0),mImage); //画出图像
}


void MainWindow::slotGetOneFrame(QImage img)
    mImage = img;
    update(); //调用update将执行 paintEvent函数
}

void MainWindow::on_pushButton_start_clicked()
{
    QString textEdit = "start";
    QByteArray sendMessage = textEdit.toLocal8Bit();
    client->tcpSocket->write(sendMessage);
}

void MainWindow::on_pushButton_stop_clicked()
{
    QString textEdit = "stop";
    QByteArray sendMessage = textEdit.toLocal8Bit();
    client->tcpSocket->write(sendMessage);
}
