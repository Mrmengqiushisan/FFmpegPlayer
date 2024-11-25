#include "mainwindow.h"
#include <QDebug>
#include <QPainter>
#include "threadpool.h"
#include "av_player.h"
#include <QResizeEvent>
#include "vframe.h"
#include <QFileDialog>
#include <QDateTime>
#include "dialog_volume.h"
#include "dialog_config.h"
#include <QtPlatformHeaders/QWindowsWindowFunctions>
#include <QDesktopWidget>
#include <QEvent>

Q_DECLARE_METATYPE(QSharedPointer<YUV420Frame>)

MainWindow::MainWindow(QWidget *parent)
    : DragableWidget(parent)
    , m_duration(1)
    , m_seekTarget(0)
    , m_ptsSliderPressed(false)
    , m_bottomwidget_enter(false)
    , m_vFmt("视频文件(*.mp4 *.mov *.avi *.mkv *.wmv *.flv *.webm *.mpeg *.mpg *.3gp *.m4v *.rmvb *.vob *.ts *.mts *.m2ts *.f4v *.divx *.xvid)")
{
    ui->slider_AVPts->setEnabled(false);
    ui->btn_next->setEnabled(false);
    ui->btn_pauseon->setEnabled(false);

    ui->label_central_volume->setAlignment(Qt::AlignCenter);
    ui->label_central_volume->hide();

    //为opengl窗口注册事件过滤器
    ui->widget_center->installEventFilter(this);

    ui->widget_side->installEventFilter(this);

    ui->widget_bottom->installEventFilter(this);

    //自定义数据类型在槽中作为参数传递需先注册
    qRegisterMetaType<QSharedPointer<YUV420Frame>>("QSharedPointer<YUV420Frame>");
    m_player=new AVPlayer;

    connect(m_player,&AVPlayer::frameChanged,ui->opengl_widget,&OpenGLWidget::onShowYUV,Qt::QueuedConnection);

    connect(ui->btn_pauseon,&QPushButton::clicked,this,&MainWindow::pauseOnBtnClickSlot);

    connect(ui->listwidget_playlist,&PlayListWidget::currentItemChanged,this,&MainWindow::playlistSelectionChangedSlot);

    connect(ui->btn_next,&QPushButton::clicked,this,&MainWindow::nextBtnSlot);

    connect(ui->btn_showSideWidget,&QPushButton::clicked,this,&MainWindow::showSideBtnSlot);

    connect(ui->btn_addfile,&QPushButton::clicked,this,&MainWindow::addFileBtnSlot);

    connect(ui->btn_opendir,&QPushButton::clicked,this,&MainWindow::addFileBtnSlot);

    connect(m_player,&AVPlayer::AVDurationChanged,this,&MainWindow::durationChangedSlot);

    connect(m_player,&AVPlayer::AVPtsChanged,this,&MainWindow::ptsChangedSlot);

    connect(m_player,&AVPlayer::AVTerminate,this,&MainWindow::terminateSlot,Qt::QueuedConnection);

    connect(ui->slider_AVPts,&AVPtsSlider::sliderPressed,this,&MainWindow::ptsSliderPressedSlot);
    connect(ui->slider_AVPts,&AVPtsSlider::sliderMoved,this,&MainWindow::ptsSliderMovedSlot);
    connect(ui->slider_AVPts,&AVPtsSlider::sliderReleased,this,&MainWindow::ptsSliderReleaseSlot);

    connect(ui->opengl_widget,&OpenGLWidget::mouseDoubleClicked,[&](){
        if(isFullScreen()&&!ui->widget_side->isHidden())
            showSideBtnSlot();
        showFullScreenBtnSlot();
    });
    connect(ui->opengl_widget,&OpenGLWidget::mouseClicked,[this](){
        if(isFullScreen()&&!ui->widget_side->isHidden()) {
            showSideBtnSlot();
            return;
        }
        if(ui->listwidget_playlist->currentRow()!=-1)
            pauseOnBtnClickSlot();
    });

    connect(ui->btn_playspeed,&QPushButton::clicked,[this](){
        QPoint pos=ui->widget_bottom->mapToGlobal(QPoint(ui->btn_playspeed->x(),0));
        pos+=QPoint(-30,-PlaySpeedDialog::getInstance()->height()-10);
        PlaySpeedDialog::getInstance()->move(pos);
        qDebug()<<"isFullScreen():"<<isFullScreen();
        PlaySpeedDialog::getInstance()->show();
        if(isFullScreen())
            setGeometry(m_actualFullScreenRect);
    });
    connect(PlaySpeedDialog::getInstance(),&PlaySpeedDialog::playSpeedChanged,this,&MainWindow::playSpeedSlot);

    connect(ui->btn_volume,&QPushButton::clicked,[this](){
        QPoint point=ui->widget_bottom->mapToGlobal(QPoint(ui->btn_volume->x(),0));
        point+=QPoint(-15,-VolumeDialog::getInstance()->height()-15);
        VolumeDialog::getInstance()->move(point);
        VolumeDialog::getInstance()->show();
        if(isFullScreen())
            setGeometry(m_actualFullScreenRect);
    });
    connect(VolumeDialog::getInstance(),&VolumeDialog::volumeValueChanged,this,&MainWindow::setVolume);

    connect(ui->btn_config,&QPushButton::clicked,[this](){
        QPoint pos=ui->widget_bottom->mapToGlobal(QPoint(ui->btn_config->x(),0));
        pos+=QPoint(-ConfigDialog::getInstance()->width()*3/4,-ConfigDialog::getInstance()->height()-15);
        ConfigDialog::getInstance()->move(pos);
        ConfigDialog::getInstance()->show();
        if(isFullScreen())
            setGeometry(m_actualFullScreenRect);
    });
    connect(ConfigDialog::getInstance(),&ConfigDialog::playModeChanged,this,&MainWindow::playModeSlot);
    connect(ConfigDialog::getInstance(),&ConfigDialog::scaleRateChanged,this,&MainWindow::scaleRateSlot);

    m_showBtnTimer.setInterval(2*1000);
    connect(&m_showBtnTimer,&QTimer::timeout,[this](){
        ui->btn_showSideWidget->hide();
        m_showBtnTimer.stop();
    });
    connect(&m_showBtnTimer,&QTimer::timeout,this,&MainWindow::hideBottomWidgetSlot);

    m_showVolumeTimer.setInterval(2*1000);
    connect(&m_showVolumeTimer,&QTimer::timeout,[this](){
        ui->label_central_volume->hide();
        m_showVolumeTimer.stop();
    });

    connect(VolumeDialog::getInstance(),&VolumeDialog::mouseEnter,this,&MainWindow::showBottomWidgetSlot);
    connect(PlaySpeedDialog::getInstance(),&PlaySpeedDialog::mouseEnter,this,&MainWindow::showBottomWidgetSlot);
    connect(ConfigDialog::getInstance(),&ConfigDialog::mouseEnter,this,&MainWindow::showBottomWidgetSlot);

    connect(ui->btn_clearlist,&QPushButton::clicked,this,&MainWindow::clearlistBtnSlot);

    connect(ui->listwidget_playlist,&PlayListWidget::selectedItemRemoved,this,&MainWindow::removeSingleItemSlot);

    connect(ui->btn_fullscreen,&QPushButton::clicked,this,&MainWindow::showFullScreenBtnSlot);

    ui->btn_showSideWidget->hide();
    ui->widget_side->hide();

    ui->widget_bottom_assit->hide();
    ui->widget_title_fullscreen->hide();
    ui->widget_space_side->hide();

    //默认列表循环
    ConfigDialog::getInstance()->setPlayMode(0);
    //默认原始比例渲染
    ConfigDialog::getInstance()->setScaleRate(0);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    qDebug()<<"change brfore:";
    qDebug()<<"ui->openglwidget_container: "<<ui->openglwidget_container->geometry();
    qDebug()<<"ui->widget_center: "<<ui->widget_center->geometry();
    ui->openglwidget_container->setGeometry(0,0,ui->widget_center->width(),ui->widget_center->height());
    qDebug()<<"change after: ";
    qDebug()<<"ui->openglwidget_container: "<<ui->openglwidget_container->geometry();
    qDebug()<<"ui->widget_center: "<<ui->widget_center->geometry();
    if(ui->widget_side->isHidden()) {
        ui->btn_showSideWidget->move(ui->widget_center->width()-ui->btn_showSideWidget->width(),\
                                     (ui->widget_center->height()-ui->btn_showSideWidget->height())/2);
    } else {
        ui->btn_showSideWidget->move(ui->widget_center->width()-ui->widget_side->width()-ui->btn_showSideWidget->width(),\
                                     (ui->widget_center->height()-ui->btn_showSideWidget->height())/2);

    }
    ui->widget_side->setGeometry(ui->widget_center->width()-ui->widget_side->width(),0,\
                                 ui->widget_side->width(),ui->widget_center->height());
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_Right:
            seekForwardSlot();
            break;
        case Qt::Key_Left:
            seekBackSlot();
            break;
        case Qt::Key_Space:
            pauseOnBtnClickSlot();
            break;
        default:
            break;
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    static QPoint mousePos;
    if(obj==ui->widget_center) {
        if(event->type()==QEvent::MouseMove||event->type()==QEvent::Enter) {
            showBottomWidgetSlot();
            m_showBtnTimer.start();  
        } else if(event->type()==QEvent::Wheel) {
            QMouseEvent* mouse=(QMouseEvent*)(event);
            QWidget* child=ui->widget_center->childAt(mouse->pos());
            if(!(child==ui->opengl_widget||child==ui->widget_bottom))
                return true;
            QWheelEvent* e=dynamic_cast<QWheelEvent*>(event);
            m_showVolumeTimer.start();
            int volume;
            if(e->angleDelta().y()>0) {
                volume=qMax(0,qMin(VolumeDialog::getInstance()->getVolume()+2,100));
            } else {
                volume=qMax(0,qMin(VolumeDialog::getInstance()->getVolume()-2,100));
            }
            VolumeDialog::getInstance()->setVolume(volume);
            ui->label_central_volume->setText(QString("音量: %1%").arg(volume));
            ui->label_central_volume->show();
        } else if(event->type()==QEvent::Leave) {
            hideBottomWidgetSlot();
        }
    } else if(obj==ui->widget_bottom) {
        if(event->type()==QEvent::Enter) {
            m_bottomwidget_enter=true;
            showBottomWidgetSlot();
        } else if(event->type()==QEvent::Leave) {
            m_bottomwidget_enter=false;
        }
        return true;
    }
    return DragableWidget::eventFilter(obj,event);
}

void MainWindow::closeBtnClickSlot()
{
    m_player->clearPlayer();
    return DragableWidget::closeBtnClickSlot();
}

void MainWindow::durationChangedSlot(unsigned int duration)
{
    ui->label_duration->setText(QString("%1:%2:%3").\
                                arg(duration/60/60,2,10,QLatin1Char('0')).\
                                arg(duration/60%60,2,10,QLatin1Char('0')).\
                                arg(duration%60,2,10,QLatin1Char('0')));
    m_duration=duration;
}

void MainWindow::ptsChangedSlot(unsigned int pts)
{
    if(m_ptsSliderPressed)
        return;
    ui->slider_AVPts->setPtsPercent((double)pts/m_duration);
    ui->label_pts->setText(QString("%1:%2:%3").\
                           arg(pts/60/60,2,10,QLatin1Char('0')).\
                           arg(pts/60%60,2,10,QLatin1Char('0')).\
                           arg(pts%60,2,10,QLatin1Char('0')));
}

void MainWindow::setWidgetDefaultState()
{
    ui->label_pts->setText(QString("00:00:00"));
    ui->label_duration->setText(QString("00:00:00"));
    ui->label_playing->setText(QString(""));
    ui->label_playing_fullscreen->setText(QString(""));
    ui->btn_pauseon->setStyleSheet("QPushButton#btn_pauseon{"
                                   "border-image: url(:/Image/pause_normal.png);}"
                                   "QPushButton:hover#btn_pauseon{"
                                   "border-image:url(:/Image/pause_hover.png);}");
    ui->slider_AVPts->setPtsPercent(0.00);
    ui->slider_AVPts->setEnabled(false);

    ui->opengl_widget->resetFrame();

    m_player->clearPlayer();
}

void MainWindow::enableRelatedWidgets()
{
    if(ui->listwidget_playlist->count()) {
        ui->btn_pauseon->setEnabled(true);
        ui->btn_next->setEnabled(true);
        ui->btn_opendir->hide();
        ui->label_addfileTip->hide();
        if(ui->widget_side->isHidden())
            showSideBtnSlot();
    } else {
        ui->btn_pauseon->setEnabled(false);
        ui->btn_next->setEnabled(false);
        ui->btn_opendir->show();
        ui->label_addfileTip->show();
    }
}

void MainWindow::showPlayList()
{
    if(!isFullScreen()) {
        ui->widget_space_side->show();
    } else {
        ui->widget_title_fullscreen->hide();
        ui->widget_bottom_assit->hide();
        ui->slider_AVPts->hide();
    }
    ui->btn_showSideWidget->move(ui->widget_center->width()-ui->widget_side->width()-ui->btn_showSideWidget->width(),\
                                 (ui->widget_center->height()-ui->btn_showSideWidget->height())/2);
    ui->widget_side->show();
}

void MainWindow::hidePlayList()
{
    ui->widget_space_side->hide();
    ui->btn_showSideWidget->move(ui->widget_center->width()-ui->btn_showSideWidget->width(),\
                                 (ui->widget_center->height()-ui->btn_showSideWidget->height())/2);
    ui->widget_side->hide();
    ui->widget_bottom_assit->show();
    ui->slider_AVPts->show();
}

void MainWindow::terminateSlot()
{
    setWidgetDefaultState();
    playNextMedia();
}

void MainWindow::playNextMedia()
{
    int _row;
    switch (m_playMode) {
        case PlayMode::SINGLE_STOP:
            ui->listwidget_playlist->setCurrentRow(-1);
            break;
        case PlayMode::LIST_LOOP:
            nextBtnSlot();
            break;
        case PlayMode::SEQUENCE:
            _row=ui->listwidget_playlist->currentRow()+1;
            if(_row>=ui->listwidget_playlist->count())
                _row=-1;
            ui->listwidget_playlist->setCurrentRow(_row);
            break;
        case PlayMode::SINGLE_LOOP:
            _row=ui->listwidget_playlist->currentRow();
            ui->listwidget_playlist->setCurrentRow(-1);
            ui->listwidget_playlist->setCurrentRow(_row);
            break;
    }
}

void MainWindow::ptsSliderPressedSlot()
{
    m_ptsSliderPressed=true;
    m_seekTarget=(int)(ui->slider_AVPts->ptsPercent()*m_duration);
}

void MainWindow::ptsSliderMovedSlot()
{
    //qDebug()<<"ptsSlider value:"<<endl;
    m_seekTarget=(int)(ui->slider_AVPts->cursorXPercent()*m_duration);
    const QString& ptsStr=QString("%1:%2:%3").\
            arg(m_seekTarget/60/60,2,10,QLatin1Char('0')).\
            arg(m_seekTarget/60%60,2,10,QLatin1Char('0')).\
            arg(m_seekTarget%60,2,10,QLatin1Char('0'));
    if(m_ptsSliderPressed)
        ui->label_pts->setText(ptsStr);
    else
        ui->slider_AVPts->setToolTip(ptsStr);
}

void MainWindow::ptsSliderReleaseSlot()
{
    m_player->seekTo(m_seekTarget);
    m_ptsSliderPressed=false;
}

void MainWindow::seekForwardSlot()
{
    m_player->seekBy(6);
    if(m_player->playState()==AVPlayer::AV_PAUSED)
        m_player->pause(false);
}

void MainWindow::nextBtnSlot()
{
    if(ui->listwidget_playlist->count()) {
        int _row=(ui->listwidget_playlist->currentRow()+1)%ui->listwidget_playlist->count();
        if(_row==0)
            ui->listwidget_playlist->setCurrentRow(-1);
        ui->listwidget_playlist->setCurrentRow(_row);
    }
}

void MainWindow::seekBackSlot()
{
    m_player->seekBy(-6);
    if(m_player->playState()==AVPlayer::AV_PAUSED)
        m_player->pause(false);
}

void MainWindow::playSpeedSlot(PlaySpeedDialog::Speed speed)
{
    switch (speed) {
        case PlaySpeedDialog::Speed::_2_0:
            m_player->setSpeed(2.0);
            ui->btn_playspeed->setText("2.0x");
            break;
        case PlaySpeedDialog::Speed::_1_5:
            m_player->setSpeed(1.5);
            ui->btn_playspeed->setText("1.5x");
            break;
        case PlaySpeedDialog::Speed::_1_25:
            m_player->setSpeed(1.25);
            ui->btn_playspeed->setText("1.25x");
            break;
        case PlaySpeedDialog::Speed::_1_0:
            m_player->setSpeed(1.0);
            ui->btn_playspeed->setText(QString("\345\200\215\351\200\237"));
            break;
        case PlaySpeedDialog::Speed::_0_75:
            m_player->setSpeed(0.75);
            ui->btn_playspeed->setText("0.75x");
            break;
        case PlaySpeedDialog::Speed::_0_5:
            m_player->setSpeed(0.5);
            ui->btn_playspeed->setText("0.5x");
            break;
        default:
            break;
    }
}

void MainWindow::scaleRateSlot(int scaleRate)
{
    //qDebug()<<"scaleRate:"<<scaleRate;
    switch (scaleRate) {
        case 0:
            ui->opengl_widget->setScaleRate(OpenGLWidget::ScaleRate::RATE_ORIGIN);
            break;
        case 1:
            ui->opengl_widget->setScaleRate(OpenGLWidget::ScaleRate::RATE_4_3);
            break;
        case 2:
            ui->opengl_widget->setScaleRate(OpenGLWidget::ScaleRate::RATE_16_9);
            break;
        case 3:
            ui->opengl_widget->setScaleRate(OpenGLWidget::ScaleRate::RATE_FULLSCREEN);
            break;
        default:
            break;
    }
}

void MainWindow::playModeSlot(int modeId)
{
    //qDebug()<<"modeId:"<<modeId;
    switch (modeId) {
        case 0:
            m_playMode=PlayMode::LIST_LOOP;
            break;
        case 1:
            m_playMode=PlayMode::SINGLE_LOOP;
            break;
        case 2:
            m_playMode=PlayMode::SINGLE_STOP;
            break;
        case 3:
            m_playMode=PlayMode::SEQUENCE;
            break;
        default:
            break;
    }
}

void MainWindow::showSideBtnSlot()
{
    if(ui->widget_side->isHidden()) {
        ui->btn_showSideWidget->setStyleSheet("QPushButton#btn_showSideWidget{"
                                              "background-color: rgb(0,0,0,100);"
                                              "border-image: url(:/Image/hideside_normal.png);}"
                                              "QPushButton:hover#btn_showSideWidget{"
                                              "border-image: url(:/Image/hideside_hover.png);}");
        showPlayList();
    } else {
        ui->btn_showSideWidget->setStyleSheet("QPushButton#btn_showSideWidget{"
                                              "background-color: rgb(0,0,0,100);"
                                              "border-image: url(:/Image/showside_normal.png);}"
                                              "QPushButton:hover#btn_showSideWidget{"
                                              "border-image: url(:/Image/showside_hover.png);}");
        hidePlayList();
    }
}

void MainWindow::playlistSelectionChangedSlot(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(previous) {
        setWidgetDefaultState();
    }
    if(!current)
        return;
    PlayListWidgetItem* _item=dynamic_cast<PlayListWidgetItem*>(current);
    if(!m_player->play(_item->url)) {
        ui->listwidget_playlist->setCurrentRow(-1);
        return;
    }
    ui->btn_pauseon->setStyleSheet("QPushButton#btn_pauseon{"
                                   "border-image: url(:/Image/play_normal.png);}"
                                   "QPushButton:hover#btn_pauseon{"
                                   "border-image:url(:/Image/play_hover.png);}");
    ui->slider_AVPts->setEnabled(true);
    ui->label_playing->setText(_item->filename);
    ui->label_playing_fullscreen->setText(_item->filename);
}

void MainWindow::clearlistBtnSlot()
{
    if(ui->listwidget_playlist->currentRow()>=0)
        setWidgetDefaultState();

    for(int i=0;i<ui->listwidget_playlist->count();i++) {
        PlayListWidgetItem* item=dynamic_cast<PlayListWidgetItem*>(ui->listwidget_playlist->item(i));
        ui->listwidget_playlist->removeItemWidget(item);
        delete item->widget;
    }
    ui->listwidget_playlist->clear();

    enableRelatedWidgets();
}

void MainWindow::removeSingleItemSlot()
{
    enableRelatedWidgets();
}

void MainWindow::showBottomWidgetSlot()
{
    ui->slider_AVPts->setStyleSheet("QSlider::groove:horizontal#slider_AVPts{border:0px;}"
                                    "QSlider::sub-page:horizontal#slider_AVPts{"
                                    "background:rgb(255,92,56);"
                                    "margin-top:7px;"
                                    "margin-bottom:7px;}"
                                    "QSlider::add-page:horizontal#slider_AVPts{"
                                    "background:rgb(83,83,83);"
                                    "margin-top:7px;"
                                    "margin-bottom:7px;}");
    QTime curTime=QTime::currentTime();
    ui->label_current_time->setText(QString("%1:%2").arg(curTime.hour(),2,10,QLatin1Char('0'))\
                                                .arg(curTime.minute(),2,10,QLatin1Char('0')));
    ui->btn_showSideWidget->show();
    if(!(isFullScreen()&&!ui->widget_side->isHidden()))
        ui->widget_bottom_assit->show();
    if(isFullScreen()&&ui->widget_side->isHidden())
        ui->widget_title_fullscreen->show();
}

void MainWindow::hideBottomWidgetSlot()
{
    if(m_bottomwidget_enter||
            VolumeDialog::getInstance()->isEnter()||
            PlaySpeedDialog::getInstance()->isEnter()||
            ConfigDialog::getInstance()->isEnter()) {
        return;
    }

    ui->slider_AVPts->setStyleSheet("QSlider::groove:horizontal#slider_AVPts {border:0px;}"
                                    "QSlider::sub-page:horizontal#slider_AVPts{"
                                        "background:rgb(255,92,56);"
                                        "margin-top:14px;"
                                        "margin-bottom:1px;}"
                                    "QSlider::add-page:horizontal#slider_AVPts{"
                                        "background:rgb(83,83,83);"
                                        "margin-top:14px;"
                                        "margin-bottom:1px;}");
    ui->widget_title_fullscreen->hide();
    //ui->btn_showSideWidget->hide();
    ui->widget_bottom_assit->hide();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setBrush(QBrush(QColor(46,46,54)));
    painter.drawRect(rect());
}

MainWindow::~MainWindow()
{
}

void MainWindow::pauseOnBtnClickSlot()
{
    if(ConfigDialog::getInstance()->isFocusIn()||
       VolumeDialog::getInstance()->isFocusIn()||
       PlaySpeedDialog::getInstance()->isFocusIn())
        return;

    switch (m_player->playState()) {
        case AVPlayer::AV_PLAYING:
            m_player->pause(true);
            ui->btn_pauseon->setStyleSheet("QPushButton#btn_pauseon{"
                                           "border-image: url(:/Image/pause_normal.png);}"
                                           "QPushButton:hover#btn_fullscreen{"
                                           "border-image:url(:/Image/pause_hover.png);}");
            break;
        case AVPlayer::AV_PAUSED:
            m_player->pause(false);
            ui->btn_pauseon->setStyleSheet("QPushButton#btn_pauseon{"
                                           "border-image: url(:/Image/play_normal.png);}"
                                           "QPushButton:hover#btn_fullscreen{"
                                           "border-image:url(:/Image/play_hover.png);}");
            break;
        case AVPlayer::AV_STOPPED:
            nextBtnSlot();
            break;
    }
}


void MainWindow::showFullScreenBtnSlot()
{
    static bool isMaximum;
    static bool isShow=false;
    if(isFullScreen()) {
        ui->btn_fullscreen->setStyleSheet("QPushButton#btn_fullscreen{"
                                                   "border: none;"
                                                   "border-image:url(:/Image/fullscreen_normal.png);}"
                                                   "QPushButton:hover#btn_fullscreen{"
                                                   "border-image: url(:/Image/fullscreen_hover.png);}");
        ui->widget_title->show();
        ui->widget_title_fullscreen->hide();
        showNormal();
        if(isMaximum)
            showMaximized();
        else
            showNormal();

        if(isShow) {
            showSideBtnSlot();
            isShow=false;
        }

    } else {
        //需要最大化
        ui->btn_fullscreen->setStyleSheet("QPushButton#btn_fullscreen{"
                                                  "border: none;"
                                                  "border-image: url(:/Image/disfullscreen_normal.png);}"
                                                  "QPushButton:hover#btn_fullscreen{"
                                                  "border-image: url(:/Image/disfullscreen_hover.png);}");
        ui->widget_title->hide();
        if(!ui->widget_side->isHidden()) {
            isShow=true;
            showSideBtnSlot();
        }

        if(isMaximized()) {
            isMaximum=true;
            showNormal();
        } else {
            isMaximum=false;
        }

        showFullScreen();
    }
}

void MainWindow::addFileBtnSlot()
{
    QStringList url=QFileDialog::getOpenFileNames(this, "选择文件", "D:/MusicResources/Music/MV", m_vFmt);
    for(auto& iter:url) {
        //qDebug()<<iter;
        if(ui->listwidget_playlist->isItemExisted(iter))
            continue;
        AVTool::MediaInfo* info=m_player->detectMediaInfo(iter);
        if(!info)
            continue;
        ui->listwidget_playlist->addSelfDefineItem(info->tipImg,info->duration,iter);
        delete info;
    }
    enableRelatedWidgets();
}

void MainWindow::setVolume(int volume)
{
    static int lastVolume=volume;
    m_player->setVolume(volume);
    if(volume==0) {
        ui->btn_volume->setStyleSheet("QPushButton#btn_volume{border: none;border-image: url(:/Image/novolume_normal.png);}"
                                      "QPushButton:hover#btn_volume{border-image: url(:/Image/novolume_hover.png);}");
        lastVolume=0;
    } else if(lastVolume==0) {
        ui->btn_volume->setStyleSheet("QPushButton#btn_volume{border: none;border-image: url(:/Image/volume_normal.png);}"
                                      "QPushButton:hover#btn_volume{border-image: url(:/Image/volume_hover.png);}");
        lastVolume=volume;
    }
}
