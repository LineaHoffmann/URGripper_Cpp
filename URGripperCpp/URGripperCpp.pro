TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        main.cpp \
    l298.cpp \
    adc0832.cpp
unix:!macx: LIBS += -lcppgpio
unix:!macx: LIBS += -lpthread

HEADERS += \
    l298.h \
    adc0832.h
