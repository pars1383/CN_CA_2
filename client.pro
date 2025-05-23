QT -= gui
QT += core network

CONFIG += c++17 console
CONFIG -= app_bundle

SOURCES += \
    src/client_main.cpp \
    src/client.cpp \
    src/manager.cpp \
    src/chunkserver.cpp \
    src/reedsolomon.cpp \
    src/firewall_punching.cpp

HEADERS += \
    src/client.h \
    src/manager.h \
    src/chunkserver.h \
    src/reedsolomon.h \
    src/firewall_punching.h

INCLUDEPATH += src

qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target