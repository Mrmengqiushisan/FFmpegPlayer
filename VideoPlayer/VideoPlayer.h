#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <memory>
#include <mutex>
#include "avplayer.h"
#include "vframe.h"
extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
}

QT_BEGIN_NAMESPACE
namespace Ui { class VideoPlayer; }
QT_END_NAMESPACE

class AVPlayer;

class VideoPlayer : public QWidget
{
    Q_OBJECT

public:
    VideoPlayer(QWidget *parent = nullptr);
    ~VideoPlayer();
protected:
    void paintEvent(QPaintEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event)override;
protected slots:
    void openFile();
    void setVolume(int volume);
    void ptsChangedSlot(unsigned int pts);
    void durationChangedSlot(unsigned int duration);
    void terminateSlot();
    void ptsSliderPressSlot();
    void ptsSliderMovedSlot();
    void ptsSliderReleaseSlot();
    void seekForwardSlot();
    void seekBackSlot();
    void pauseOnBtnClickSlot();
private:
    Ui::VideoPlayer *ui;
    AVPlayer* m_player;
    const QString m_vFmt;
    unsigned int m_duration;
    bool m_ptsSliderPressed;
    int  m_seekTarget;
};
#endif // VIDEOPLAYER_H
