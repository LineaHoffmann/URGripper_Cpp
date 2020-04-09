TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QT       += core
QT       += network
QT       -= gui

SOURCES += \
        main.cpp \
    l298.cpp \
    adc0832.cpp \
    motorcontroller.cpp \
    mythread.cpp \
    myserver.cpp
unix:!macx: LIBS += -lcppgpio
unix:!macx: LIBS += -lpthread

HEADERS += \
    l298.h \
    adc0832.h \
    motorcontroller.h \
    myserver.h \
    mythread.h
