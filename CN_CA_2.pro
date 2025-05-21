QT += core network
CONFIG += c++11
SOURCES += \
    src/client.cpp \
    src/manager.cpp \
    src/chunkserver.cpp \
    src/reedsolomon.cpp \
    src/firewall_punching.cpp \
    src/main.cpp
HEADERS += \
    src/client.h \
    src/manager.h \
    src/chunkserver.h \
    src/reedsolomon.h \
    src/firewall_punching.h
