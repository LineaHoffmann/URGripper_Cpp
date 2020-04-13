# Restructured slightly to support Qt libraries
QT -= gui
QT += network
QT += core
CONFIG += c++14 console
CONFIG -= app_bundle

# Borrowed from Qt-terminal project default
DEFINES += QT_DEPRECATED_WARNINGS

# For std::make_unique
QMAKE_CXXFLAGS += -std=c++14

# Raspberry Pi and threading libs
unix:!macx: LIBS += -lcppgpio
unix:!macx: LIBS += -lpthread

# Source files
SOURCES += \
        main.cpp \
    l298.cpp \
    adc0832.cpp \
    motorcontroller.cpp \
    myserver.cpp \
    mythread.cpp

# Header files
HEADERS += \
    l298.h \
    adc0832.h \
    motorcontroller.h \
    myserver.h \
    mythread.h

# Borrowed from Qt-terminal project default
# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
