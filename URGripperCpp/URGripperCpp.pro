# Restructured slightly to support Qt libraries
CONFIG += c++17 console
CONFIG -= app_bundle

# Borrowed from Qt-terminal project default
#DEFINES += QT_DEPRECATED_WARNINGS

# For std::make_unique
QMAKE_CXXFLAGS += -std=c++17

DEFINES += NCURSES_NOMACROS

# Raspberry Pi and threading libs
unix:!macx: LIBS += -lcppgpio
unix:!macx: LIBS += -lpthread
unix:!macx: LIBS += -lncurses
unix:!macx: LIBS += -ltinfo
unix:!macx: LIBS += -lboost_system

# Source files
SOURCES += \
        main.cpp \
    l298.cpp \
    adc0832.cpp \
    motorcontroller.cpp \
    consolegui.cpp \
    tcpserver.cpp

# Header files
HEADERS += \
    l298.h \
    adc0832.h \
    motorcontroller.h \
    consolegui.h \
    tcpserver.h

# Borrowed from Qt-terminal project default
# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target
