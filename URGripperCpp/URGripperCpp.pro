CONFIG += c++17 console
CONFIG -= app_bundle

# For actual C++17
QMAKE_CXXFLAGS += -std=c++17

# Some Ncurses troubles
DEFINES += NCURSES_NOMACROS

# Libs
unix:!macx: LIBS += -lcppgpio       # Raspberry hardware
unix:!macx: LIBS += -lpthread       # Threading for CppGPIO
unix:!macx: LIBS += -lncurses       # Console window manager
unix:!macx: LIBS += -ltinfo         # OS checking
unix:!macx: LIBS += -lboost_system  # Boost for TCP and some stuff

# Source files
SOURCES += \
        main.cpp \
    l298.cpp \
    adc0832.cpp \
    motorcontroller.cpp \
    consolegui.cpp \
    tcpserver.cpp \
    statecontroller.cpp

# Header files
HEADERS += \
    l298.h \
    adc0832.h \
    motorcontroller.h \
    consolegui.h \
    tcpserver.h \
    statecontroller.h
