#include "VideoPlayer.h"

#include <QApplication>
#include <QString>
#include <QFile>
#include "playspeeddialog.h"
#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    QFile qss(":/resource/QSS/widget.qss");
//    if(qss.open(QFile::ReadOnly|QFile::Text)){
//        a.setStyleSheet(qss.readAll());
//    }
//    VideoPlayer w;
//    w.show();
    PlaySpeedDialog::getInstance()->show();
    return a.exec();
}
