#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QWidget>

namespace Ui {
class MediaPlayer;
}

class MediaPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit MediaPlayer(QWidget *parent = nullptr);
    ~MediaPlayer();

private:
    Ui::MediaPlayer *ui;
};

#endif // MEDIAPLAYER_H
