QT       += core gui opengl

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += \
    $$PWD/SDL2-2.0.16/include \
    $$PWD/ffmpegcode/include

SOURCES += \
    avplayer.cpp \
    avptsslider.cpp \
    c11threadpool.cpp \
    configdialog.cpp \
    decoder.cpp \
    main.cpp \
    VideoPlayer.cpp \
    mediaplayer.cpp \
    openglwidget.cpp \
    playlistwidget.cpp \
    playspeeddialog.cpp

HEADERS += \
    VideoPlayer.h \
    avplayer.h \
    avptsslider.h \
    c11threadpool.h \
    clock.h \
    configdialog.h \
    decoder.h \
    mediaplayer.h \
    openglwidget.h \
    playlistwidget.h \
    playspeeddialog.h \
    vframe.h

FORMS += \
    VideoPlayerget.ui \
    configdialog.ui \
    mediaplayer.ui \
    playlistwidget.ui
LIBS += \
    -L$$PWD/SDL2-2.0.16/lib/x64 \
    -L$$PWD/ffmpegcode/lib \
    -lavcodec\
    -lavdevice\
    -lavfilter\
    -lavformat\
    -lavutil\
    -lpostproc\
    -lswresample\
    -lswscale\
    -lSDL2\
    -lSDL2main\
    -lSDL2test

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc
