QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    audiothread.cpp \
    ffmpegs.cpp

HEADERS += \
    mainwindow.h \
    audiothread.h \
    ffmpegs.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32 {
    FFMPEG_HOME = ...
}

macx {
    FFMPEG_HOME = /usr/local/Cellar/ffmpeg/6.0
    QMAKE_INFO_PLIST = mac/info.plist
}

INCLUDEPATH += $${FFMPEG_HOME}/include

# 设置库文件路径
LIBS += -L$${FFMPEG_HOME}/lib \
        -lavcodec \
        -lavdevice \
        -lavfilter \
        -lavformat \
        -lavutil \
        -lpostproc \
        -lswscale \
        -lswresample

#LIBS += -L$${FFMPEG_HOME}/lib \
#        -lavdevice \
#        -lavformat \
#        -lavutil \
#        -lavcodec

DISTFILES += \
    mac/info.plist
