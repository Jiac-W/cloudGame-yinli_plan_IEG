/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout_2;
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *verticalSpacer;
    QPushButton *pushButton_stop;
    QLineEdit *lineEdit_log;
    QLabel *label_centos;
    QLineEdit *lineEdit_message;
    QPushButton *pushButton_start;
    QPushButton *pushButton_send;
    QPushButton *pushButton_connect;
    QPushButton *pushButton_disconnect;
    QLabel *label_status;
    QLabel *label_status_title;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1024, 900);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout_2 = new QGridLayout(centralWidget);
        gridLayout_2->setSpacing(6);
        gridLayout_2->setContentsMargins(11, 11, 11, 11);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        gridLayout = new QGridLayout();
        gridLayout->setSpacing(6);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(6);
        horizontalLayout_2->setObjectName(QStringLiteral("horizontalLayout_2"));
        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);

        horizontalLayout_2->addItem(verticalSpacer);


        gridLayout->addLayout(horizontalLayout_2, 0, 0, 1, 4);

        pushButton_stop = new QPushButton(centralWidget);
        pushButton_stop->setObjectName(QStringLiteral("pushButton_stop"));
        QFont font;
        font.setPointSize(13);
        font.setBold(true);
        font.setWeight(75);
        pushButton_stop->setFont(font);

        gridLayout->addWidget(pushButton_stop, 3, 0, 1, 1);

        lineEdit_log = new QLineEdit(centralWidget);
        lineEdit_log->setObjectName(QStringLiteral("lineEdit_log"));
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(lineEdit_log->sizePolicy().hasHeightForWidth());
        lineEdit_log->setSizePolicy(sizePolicy);
        QFont font1;
        font1.setPointSize(13);
        lineEdit_log->setFont(font1);

        gridLayout->addWidget(lineEdit_log, 2, 2, 1, 2);

        label_centos = new QLabel(centralWidget);
        label_centos->setObjectName(QStringLiteral("label_centos"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(label_centos->sizePolicy().hasHeightForWidth());
        label_centos->setSizePolicy(sizePolicy1);
        QFont font2;
        font2.setPointSize(13);
        font2.setBold(true);
        font2.setWeight(75);
        font2.setStrikeOut(false);
        label_centos->setFont(font2);
        label_centos->setMouseTracking(false);
        label_centos->setFocusPolicy(Qt::StrongFocus);
        label_centos->setLayoutDirection(Qt::LeftToRight);
        label_centos->setAutoFillBackground(false);
        label_centos->setFrameShape(QFrame::Box);
        label_centos->setFrameShadow(QFrame::Sunken);
        label_centos->setAlignment(Qt::AlignCenter);
        label_centos->setWordWrap(false);

        gridLayout->addWidget(label_centos, 2, 1, 1, 1);

        lineEdit_message = new QLineEdit(centralWidget);
        lineEdit_message->setObjectName(QStringLiteral("lineEdit_message"));
        sizePolicy.setHeightForWidth(lineEdit_message->sizePolicy().hasHeightForWidth());
        lineEdit_message->setSizePolicy(sizePolicy);
        lineEdit_message->setFont(font1);

        gridLayout->addWidget(lineEdit_message, 3, 2, 1, 2);

        pushButton_start = new QPushButton(centralWidget);
        pushButton_start->setObjectName(QStringLiteral("pushButton_start"));
        pushButton_start->setFont(font);

        gridLayout->addWidget(pushButton_start, 2, 0, 1, 1);

        pushButton_send = new QPushButton(centralWidget);
        pushButton_send->setObjectName(QStringLiteral("pushButton_send"));
        sizePolicy.setHeightForWidth(pushButton_send->sizePolicy().hasHeightForWidth());
        pushButton_send->setSizePolicy(sizePolicy);
        pushButton_send->setFont(font);

        gridLayout->addWidget(pushButton_send, 3, 1, 1, 1);

        pushButton_connect = new QPushButton(centralWidget);
        pushButton_connect->setObjectName(QStringLiteral("pushButton_connect"));
        pushButton_connect->setFont(font);
        pushButton_connect->setStyleSheet(QStringLiteral("color: rgb(0, 0, 255);"));

        gridLayout->addWidget(pushButton_connect, 4, 0, 1, 1);

        pushButton_disconnect = new QPushButton(centralWidget);
        pushButton_disconnect->setObjectName(QStringLiteral("pushButton_disconnect"));
        pushButton_disconnect->setEnabled(true);
        pushButton_disconnect->setFont(font);
        pushButton_disconnect->setStyleSheet(QStringLiteral("color: rgb(255, 0, 0);"));

        gridLayout->addWidget(pushButton_disconnect, 4, 1, 1, 1);

        label_status = new QLabel(centralWidget);
        label_status->setObjectName(QStringLiteral("label_status"));
        label_status->setMaximumSize(QSize(9999, 16777215));
        label_status->setFont(font);
        label_status->setFrameShape(QFrame::Box);
        label_status->setFrameShadow(QFrame::Sunken);
        label_status->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_status, 4, 3, 1, 1);

        label_status_title = new QLabel(centralWidget);
        label_status_title->setObjectName(QStringLiteral("label_status_title"));
        label_status_title->setFont(font);
        label_status_title->setAlignment(Qt::AlignCenter);

        gridLayout->addWidget(label_status_title, 4, 2, 1, 1);


        gridLayout_2->addLayout(gridLayout, 0, 0, 1, 3);

        MainWindow->setCentralWidget(centralWidget);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "\346\213\211\346\265\201\346\222\255\346\224\276\345\231\250_Ubuntu Local", Q_NULLPTR));
        pushButton_stop->setText(QApplication::translate("MainWindow", "STOP", Q_NULLPTR));
        label_centos->setText(QApplication::translate("MainWindow", "\344\272\221\346\234\215\345\212\241\345\231\250\345\223\215\345\272\224", Q_NULLPTR));
        pushButton_start->setText(QApplication::translate("MainWindow", "START", Q_NULLPTR));
        pushButton_send->setText(QApplication::translate("MainWindow", "Chat", Q_NULLPTR));
        pushButton_connect->setText(QApplication::translate("MainWindow", "\347\231\273\345\275\225", Q_NULLPTR));
        pushButton_disconnect->setText(QApplication::translate("MainWindow", "\351\200\200\345\207\272", Q_NULLPTR));
        label_status->setText(QApplication::translate("MainWindow", "status", Q_NULLPTR));
        label_status_title->setText(QApplication::translate("MainWindow", "\347\212\266\346\200\201", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
