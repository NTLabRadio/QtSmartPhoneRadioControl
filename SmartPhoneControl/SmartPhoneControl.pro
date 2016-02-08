#-------------------------------------------------
#
# Project created by QtCreator 2016-02-08T13:52:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets serialport

TARGET = SmartPhoneControl
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    qSmartRadioModuleControl.cpp \
    slipinterface.cpp

HEADERS  += mainwindow.h \
    qSmartRadioModuleControl.h \
    slipinterface.h

FORMS    += mainwindow.ui
