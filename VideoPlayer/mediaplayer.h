#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QWidget>
#include "dragablewidget.h"
#include "playspeeddialog.h"
#include "avplayer.h"

class AVPlayer;

class MediaPlayer : public DragableWidget
{
    Q_OBJECT
    enum class PlayMode:int{
        LIST_LOOP,//列表循环
        SINGLE_LOOP,//单曲循环
        SINGLE_STOP,//播完暂停
        SEQUENCE   //顺序播放，播完最后一个暂停
    };

public:
    explicit MediaPlayer(QWidget *parent = nullptr);
    ~MediaPlayer();
protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual bool eventFilter(QObject* obj,QEvent* event) override;

    virtual void closeBtnClickSlot() override;
private:
    AVPlayer*       m_player;
    const QString   m_vFmt;
    unsigned int    m_duration;
    int             m_seekTarget;
    PlayMode        m_playMode;
    bool            m_ptsSliderPressed;
    bool            m_bottomwidget_enter;
    //误操作一定时间隐藏展示播放列表按钮
    QTimer          m_showBtnTimer;
    QTimer          m_showVolumeTimer;
private:
    void setVolume(int volume);
    void setWidgetDefaultState();
    void enableReatedWidgets();
    void showPlayList();
    void hidePlayList();
private slots:
    void pauseOnBtnClickSlot();
    void showFullScreenBtnSlot();
    void addFileBtnSlot();
    void ptsChangedSlot(unsigned int pts);
    void durationChangedSlot(unsigned int duration);
    void terminateSlot();
    void playNextMedia();
    void ptsSliderPressSlot();
    void ptsSliderMovedSlot();
    void ptsSliderReleaseSlot();
    void seekForwardSlot();
    void nextBtnSlot();
    void seekBackSlot();
    void playSpeedSlot(PlaySpeedDialog::Speed speed);
    void playModeSlot(int modeId);
    void scaleRateSlot(int scaleRate);
    void showSideBtnSlot();
    void playlistSelectionChangedSlot(QListWidgetItem* current,QListWidgetItem* previous);
    void clearlistBtnSlot();
    void removeSingleItemSlot();
    void showBottomWidgetSlot();
    void hideBottomWidgetSlot();
};

#endif // MEDIAPLAYER_H
