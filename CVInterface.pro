#-------------------------------------------------
#
# Project created by QtCreator 2015-11-02T18:54:33
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = CVInterface
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    imagewidget.cpp \
    itemmaker.cpp \
    targetlistwindow.cpp \
    targetlist.cpp \
    targetlistitem.cpp \
    targetmaker.cpp \
    targetwindow.cpp \
    lifesupport.cpp \
    imagesetprocessor.cpp \
    loadingbardialog.cpp \
    geolocation.cpp \
    latlon.cpp

HEADERS  += mainwindow.h \
    imagewidget.h \
    itemmaker.h \
    targetlistwindow.h \
    targetlist.h \
    targetlistitem.h \
    targetmaker.h \
    targetwindow.h \
    lifesupport.h \
    imagesetprocessor.h \
    loadingbardialog.h \
    geolocation.h \
    latlon.h

FORMS    += mainwindow.ui \
    imagewidget.ui \
    itemmaker.ui \
    targetlistwindow.ui \
    targetmaker.ui \
    targetwindow.ui \
    imagesetprocessor.ui \
    loadingbardialog.ui

DISTFILES += \
    clasic.png \
    delete85.png \
    down.png \
    round75.png \
    up.png

RESOURCES += \
    resouces.qrc

win32 { # For Windows
  QMAKE_CXXFLAGS += -openmp
  QMAKE_CXXFLAGS += -arch:AVX
  QMAKE_CXXFLAGS += -D "_CRT_SECURE_NO_WARNINGS"
  QMAKE_CXXFLAGS_RELEASE *= -O2
}
	
win32:RC_ICONS += Icon.ico
