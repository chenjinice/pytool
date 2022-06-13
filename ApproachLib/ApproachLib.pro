TEMPLATE = lib
TARGET = Approach
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11

DEFINES += EWAY_SDK
DEFINES += IOS_64
DEFINES += IOS_LINUX


#DESTDIR = $$PWD/../../Bin/$$CurConfig/Function/$$TARGET
DESTDIR = /usr/share/ewaybot/ewayos/Function/$$TARGET

LIBS += -lFuncSdkLib
LIBS += -lEwaySys
LIBS += -ljsoncpp
LIBS += -lEwayModel
LIBS += -lEwayMath
LIBS += -lopencv_core
LIBS += -lopencv_imgcodecs
LIBS += -lopencv_highgui
LIBS += -lopencv_imgproc
LIBS += -lpthread

QMAKE_LFLAGS += -Wl,-unresolved-symbols=ignore-in-shared-libs
QMAKE_LFLAGS += -L/usr/share/ewaybot/ewaylib/
QMAKE_LFLAGS += -Wl,-rpath=/usr/share/ewaybot/ewaylib/
QMAKE_LFLAGS += -L/usr/share/ewaybot/thirdpartylib/lib/
QMAKE_LFLAGS += -Wl,-rpath=/usr/share/ewaybot/thirdpartylib/lib/

INCLUDEPATH += /usr/share/ewaybot/ewaysdk/include/
INCLUDEPATH += /usr/share/ewaybot/thirdpartylib/include/
INCLUDEPATH += /usr/share/ewaybot/thirdpartylib/include/ros

SOURCES += Approach.cpp  \
    PosTransform.cpp \
    TestUi.cpp
HEADERS += Approach.h \
    PosTransform.h \
    TestUi.h
