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

     OPENCVDIR = /opt/local
     INCLUDEPATH += $$OPENCVDIR/include
     LIBS += -L$$OPENCVDIR/lib
     LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio

     BOOSTPATH = /usr/local
     INCLUDEPATH += $$BOOSTPATH/include
     LIBS += -L$$BOOSTPATH/lib
}

win32 {
    message(Platform: Win32)

    OPENCVDIR = C:\local\opencv\build
    INCLUDEPATH += $$OPENCVDIR\include
    LIBS += -L$$OPENCVDIR\x64\vc14\lib
    LIBS += -lopencv_core2413d -lopencv_highgui2413d -lopencv_imgproc2413d

    #LIBS += -L$$OPENCVDIR\x64\vc14\bin
    #INCLUDEPATH += -L$$OPENCVDIR\x64\vc14\bin
    #INCLUDEPATH += -L$$OPENCVDIR\x64\vc14\lib

    BOOSTDIR  = C:\local\boost_1_67_0_b1_rc2
    INCLUDEPATH += $$BOOSTDIR
    LIBS += -L$$BOOSTDIR\lib64-msvc-14.0
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
