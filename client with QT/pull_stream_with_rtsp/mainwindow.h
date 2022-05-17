#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "clientStuff.h"

#include <QImage>
#include <QPaintEvent>
#include <QWidget>

#include "videoplayer.h"

namespace Ui { class MainWindow; }

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event);

public slots:
    void setStatus(bool newStatus);
    void receivedSomething(QString msg);
    void gotError(QAbstractSocket::SocketError err);

private slots:
    void on_pushButton_send_clicked();
    void on_pushButton_connect_clicked();
    void on_pushButton_disconnect_clicked();    
    void on_pushButton_start_clicked();
    void on_pushButton_stop_clicked();

    void slotGetOneFrame(QImage img);

private:
    Ui::MainWindow *ui;
    ClientStuff *client;

    VideoPlayer *mPlayer;    // 播放线程
    QImage mImage;           // 记录当前的图像
    QString url;

};

#endif // MAINWINDOW_H
