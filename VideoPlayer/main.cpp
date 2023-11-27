#include "VideoPlayer.h"

#include <QApplication>
#include <QString>
#include <QFile>
#include "mediaplayer.h"
#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/resource/image/player_logo.png"));
    QFile qss(":/resource/QSS/mediaplayer.qss");
    if(qss.open(QFile::ReadOnly|QFile::Text)){
        a.setStyleSheet(qss.readAll());
    }
//    VideoPlayer w;
//    w.show();
    MediaPlayer m;
    m.show();
    return a.exec();
}
