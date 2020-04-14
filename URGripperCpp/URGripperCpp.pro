# Restructured slightly to support Qt libraries
CONFIG += c++14 console
CONFIG -= app_bundle

# Borrowed from Qt-terminal project default
#DEFINES += QT_DEPRECATED_WARNINGS

# For std::make_unique
QMAKE_CXXFLAGS += -std=c++14

# Raspberry Pi and threading libs
unix:!macx: LIBS += -lcppgpio
unix:!macx: LIBS += -lpthread
unix:!macx: LIBS += -lncurses

# Source files
SOURCES += \
        main.cpp \
    l298.cpp \
    adc0832.cpp \
    motorcontroller.cpp

# Header files
HEADERS += \
    l298.h \
    adc0832.h \
    motorcontroller.h

# Borrowed from Qt-terminal project default
# Default rules for deployment.
#qnx: target.path = /tmp/$${TARGET}/bin
#else: unix:!android: target.path = /opt/$${TARGET}/bin
#!isEmpty(target.path): INSTALLS += target
