#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dragablewidget.h"
#include "dialog_playspeed.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
}

#include <memory>
#include <mutex>

class AVPlayer;

class MainWindow : public DragableWidget
{
    Q_OBJECT

    enum class PlayMode:int
    {
        LIST_LOOP,//列表循环
        SINGLE_LOOP,//单曲循环
        SINGLE_STOP,//播完暂停
        SEQUENCE //顺序播放,播完最后一个暂停
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow();

protected:
    virtual void paintEvent(QPaintEvent *event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual bool eventFilter(QObject* obj,QEvent* event) override;

    virtual void closeBtnClickSlot() override;

private:
    AVPlayer* m_player;

    const QString m_vFmt;

    unsigned int m_duration;

    int m_seekTarget;

    PlayMode m_playMode;

    bool m_ptsSliderPressed;

    bool m_bottomwidget_enter;

    //无操作一定时间隐藏展示播放列表按钮
    QTimer m_showBtnTimer;

    QTimer m_showVolumeTimer;

private:
    void setVolume(int volume);

    void setWidgetDefaultState();

    void enableRelatedWidgets();

    void showPlayList();
    void hidePlayList();

private Q_SLOTS:
    void pauseOnBtnClickSlot();

    void showFullScreenBtnSlot();

    void addFileBtnSlot();

    void ptsChangedSlot(unsigned int duration);

    void durationChangedSlot(unsigned int pts);

    void terminateSlot();

    void playNextMedia();

    void ptsSliderPressedSlot();

    void ptsSliderMovedSlot();

    void ptsSliderReleaseSlot();

    void seekForwardSlot();

    void nextBtnSlot();

    void seekBackSlot();

    void playSpeedSlot(PlaySpeedDialog::Speed speed);

    void playModeSlot(int modeId);

    void scaleRateSlot(int scaleRate);

    void showSideBtnSlot();

    void playlistSelectionChangedSlot(QListWidgetItem *current, QListWidgetItem *previous);

    void clearlistBtnSlot();

    void removeSingleItemSlot();

    void showBottomWidgetSlot();
    void hideBottomWidgetSlot();
};
#endif // MAINWINDOW_H
