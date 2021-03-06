# Session Recorder
# GPLv3 - Licensed
# Shawn P. Gilroy
# LSU Psychology

TARGET = SessionRecorder
TEMPLATE = app

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

VERSION_MAJOR = 0
VERSION_MINOR = 0
VERSION_BUILD = 3

DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
           "VERSION_MINOR=$$VERSION_MINOR"\
           "VERSION_BUILD=$$VERSION_BUILD"\
           "VERSION_TESTING=$$TEST_FEATURES"

DEFINES += QT_DEPRECATED_WARNINGS\
           VIDEOSTRING='\\"video.avi\\"'\
           VIDEOEXT='\\"avi\\"'

macx {
     message(Platform: Mac OS X)

     BOOSTPATH = /usr/local
     INCLUDEPATH += $$BOOSTPATH/include
     LIBS += -L$$BOOSTPATH/lib

     OPENCVDIR = /opt/local
     INCLUDEPATH += $$OPENCVDIR/include
     LIBS += -L$$OPENCVDIR/lib

    CONFIG(debug, debug|release) {
        DESTDIR = $$OUT_PWD/build/debug
        LIBS += -lopencv_cored -lopencv_highguid -lopencv_imgprocd -lopencv_videoiod
    } else {
        DESTDIR = $$OUT_PWD/build/release
        LIBS += -lopencv_core -lopencv_highgui -lopencv_imgproc -lopencv_videoio
    }
}

win32 {
    message(Platform: Win32)
    BOOSTDIR  = C:\Users\shawn\boost_1_58_0

    INCLUDEPATH += $$BOOSTDIR
    LIBS += -L$$BOOSTDIR\stage\lib

    OPENCVDIR = C:\local\opencv\build
    INCLUDEPATH += $$OPENCVDIR\include

    LIBS += -L$$OPENCVDIR\x64\vc14\bin
    LIBS += -L$$OPENCVDIR\x64\vc14\lib

    INCLUDEPATH += -L$$OPENCVDIR\x64\vc14\bin
    INCLUDEPATH += -L$$OPENCVDIR\x64\vc14\lib

    win32:RC_ICONS += SNS.ico

    CONFIG(debug, debug|release) {
        DESTDIR = $$OUT_PWD/build/debug
        LIBS += -lopencv_core2413d -lopencv_highgui2413d -lopencv_imgproc2413d
    } else {
        DESTDIR = $$OUT_PWD/build/release
        LIBS += -lopencv_core2413 -lopencv_highgui2413 -lopencv_imgproc2413
    }
}

SOURCES += \
    main.cpp \
    camerathread.cpp \
    avrecorder.cpp \
    qaudiolevel.cpp \
    initializationdialog.cpp

HEADERS += \
    camerathread.h \
    avrecorder.h \
    qaudiolevel.h \
    initializationdialog.h \
    enums.h \
    recordsettings.h

FORMS += \
    avrecorder.ui \
    initializationdialog.ui

DISTFILES += \
    README.md \
    COPYING \
    LICENSE_Meeting-Recorder \
    LICENSE_Qt \
    LICENSE_Qt-Examples \
    SNS.ico
