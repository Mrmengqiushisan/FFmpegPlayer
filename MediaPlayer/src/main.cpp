#include "mainwindow.h"

#include <QApplication>
#include <QFile>
#include <QDebug>
#include <QString>
#include "dialog_playspeed.h"
#undef main
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/Image/player_logo.png"));
    QFile qssFile(":/QSS/widget.qss");
    if(qssFile.open(QFile::ReadOnly)){
        QString str=qssFile.readAll();
         a.setStyleSheet(str);
    }
    MainWindow w;
    w.show();
    w.activateWindow();
//    PlaySpeedDialog::getInstance()->show();
    return a.exec();
}
