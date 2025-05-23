QT = core network
QT -= gui
CONFIG += c++17
TARGET = manager
TEMPLATE = app

SOURCES = \
    src/manager_main.cpp \
    src/client.cpp \
    src/manager.cpp \
    src/chunkserver.cpp \
    src/reedsolomon.cpp \
    src/firewall_punching.cpp

HEADERS = \
    src/client.h \
    src/manager.h \
    src/chunkserver.h \
    src/reedsolomon.h \
    src/firewall_punching.h