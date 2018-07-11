# Session Recorder
# GPLv3 - Licensed
# Shawn P. Gilroy
# LSU Psychology

TARGET = SessionRecorder
TEMPLATE = app

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

DEFINES += QT_DEPRECATED_WARNINGS

macx {
     message(Platform: Mac OS X)

     BOOSTPATH = /usr/local
     INCLUDEPATH += $$BOOSTPATH/include
     LIBS += -L$$BOOSTPATH/lib

     OPENCVDIR = /opt/local
     INCLUDEPATH += $$OPENCVDIR/include
     LIBS += -L$$OPENCVDIR/lib
     LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio #-lssh2
}

win32 {
    # TODO un-tested yet

    #message(Platform: Win32)
    #OPENCVDIR = C:\Users\localadmin_jmakoske\opencv\build
    #BOOSTDIR  = C:\Users\localadmin_jmakoske\boost_1_58_0

    #INCLUDEPATH += $$OPENCVDIR\include
    #INCLUDEPATH += $$BOOSTDIR

    #LIBS += -L$$OPENCVDIR\x86\mingw\bin
    #LIBS += -lopencv_core2411 -lopencv_highgui2411 -lopencv_imgproc2411
}

SOURCES += \
        main.cpp \
    camerathread.cpp \
    avrecorder.cpp \
    qaudiolevel.cpp

HEADERS += \
    camerathread.h \
    avrecorder.h \
    qaudiolevel.h

FORMS += \
    avrecorder.ui

DISTFILES += \
    README.md \
    COPYING \
    LICENSE_Meeting-Recorder \
    LICENSE_Qt \
    LICENSE_Qt-Examples
