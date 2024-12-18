QT       += core gui

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

#msvc {
#    QMAKE_CFLAGS+=/utf-8
#    QMAKE_CXXFLAGS+=/utf-8
#}

INCLUDEPATH += src \
          include \
          SDL2-2.0.16/include \
          src/AVPlayer \
          src/Sonic \
          src/VolumeDialog \
          src/ConfigDialog

VPATH += src

SOURCES += \
    main.cpp \
    src/AVPlayer/av_decoder.cpp \
    src/AVPlayer/av_player.cpp \
    src/AVPlayer/threadpool.cpp \
    src/ConfigDialog/dialog_config.cpp \
    src/Sonic/sonic.c \
    src/VolumeDialog/dialog_volume.cpp \
    src/VolumeDialog/slider_volume.cpp \
    src/dialog_playspeed.cpp \
    src/dragablewidget.cpp \
    src/mainwindow.cpp \
    src/opengl_widget.cpp \
    src/playlistwidget.cpp \
    src/slider_pts.cpp \

HEADERS += \
    src/AVPlayer/av_clock.h \
    src/AVPlayer/av_decoder.h \
    src/AVPlayer/av_player.h \
    src/AVPlayer/threadpool.h \
    src/AVPlayer/vframe.h \
    src/ConfigDialog/dialog_config.h \
    src/Sonic/sonic.h \
    src/VolumeDialog/dialog_volume.h \
    src/VolumeDialog/slider_volume.h \
    src/dialog_playspeed.h \
    src/dragablewidget.h \
    src/mainwindow.h \
    src/opengl_widget.h \
    src/playlistwidget.h \
    src/slider_pts.h \

FORMS += \
    src/configdialog.ui \
    src/mainwindow.ui \
    src/playlistitemwidget.ui

LIBS += -L$$PWD/lib\
        -L$$PWD/SDL2-2.0.16/lib/x64\
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
    Resources/resources.qrc
