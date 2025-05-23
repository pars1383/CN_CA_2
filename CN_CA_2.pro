QT = core network
QT -= gui
CONFIG += c++17

# Subdirs template
TEMPLATE = subdirs

# Subprojects
SUBDIRS += manager client chunkservers

manager.file = manager.pro
client.file = client.pro
chunkservers.file = chunkservers.pro