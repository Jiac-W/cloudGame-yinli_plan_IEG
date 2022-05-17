QT += core gui network widgets

CONFIG += c++11

TEMPLATE = app
TARGET = pull_stream_with_rtsp

MOC_DIR     += generated/mocs
UI_DIR      += generated/uis
RCC_DIR     += generated/rccs
OBJECTS_DIR += generated/objs

SOURCES += main.cpp\
        mainwindow.cpp \
        clientStuff.cpp \
        videoplayer.cpp

HEADERS  += mainwindow.h \
        clientStuff.h \
        videoplayer.h

FORMS    += mainwindow.ui

INCLUDEPATH += /usr/include/x86_64-linux-gnu

LIBS += /usr/lib/x86_64-linux-gnu/libavcodec.so \
/usr/lib/x86_64-linux-gnu/libavfilter.so \
/usr/lib/x86_64-linux-gnu/libswresample.so \
/usr/lib/x86_64-linux-gnu/libswscale.so \
/usr/lib/x86_64-linux-gnu/libavformat.so \
/usr/lib/x86_64-linux-gnu/libavutil.so \
