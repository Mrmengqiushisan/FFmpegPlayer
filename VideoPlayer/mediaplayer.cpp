#include "mediaplayer.h"
#include "ui_mediaplayer.h"

MediaPlayer::MediaPlayer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaPlayer)
{
    ui->setupUi(this);
}

MediaPlayer::~MediaPlayer()
{
    delete ui;
}
