######################################################################
# Automatically generated by qmake (3.0) ?? 10? 8 21:43:27 2018
######################################################################

TEMPLATE = lib
TARGET = ipcobject
INCLUDEPATH += . ./include
CONFIG += staticlib

include($$(SOUIPATH)/demo_com.pri)

PRECOMPILED_HEADER = stdafx.h

# Input
HEADERS += \
           include/IpcObject.h \
           include/paramstream.hpp \
           include/paramhelper.hpp \
           include/ShareMemBuffer.h \

SOURCES += src/IpcObject.cpp src/ShareMemBuffer.cpp