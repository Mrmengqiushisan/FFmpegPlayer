#include "mediaplayer.h"
#include "configdialog.h"
#include "volumedialog.h"
#include <QFileDialog>
#include <QPainter>
#include <QKeyEvent>
#include <QTime>
//#define DEBUG
MediaPlayer::MediaPlayer(QWidget *parent) :
    DragableWidget(parent),
    m_duration(1),
    m_seekTarget(0),
    m_ptsSliderPressed(false),
    m_bottomwidget_enter(false),
    m_vFmt("视频文件(*.mp4 *.mov *.avi *.mkv *.wmv *.flv *.webm *.mpeg *.mpg *.3gp *.m4v *.rmvb *.vob *.ts *.mts *.m2ts *.f4v *.divx *.xvid)")
{
    ui->slider_AVPts->setEnabled(false);
    ui->btn_next->setEnabled(false);
    ui->btn_pauseon->setEnabled(false);
    ui->label_central_volume->setAlignment(Qt::AlignCenter);
    ui->label_central_volume->hide();
    ui->widget_center->installEventFilter(this);
    ui->widget_side->installEventFilter(this);
    ui->widget_bottom->installEventFilter(this);
    //自定义数据类型在槽中作为参数传递需要先注册
    qRegisterMetaType<QSharedPointer<YUV422Frame>>("QSharedPointer<YUV422Frame>");
    m_player=new AVPlayer;
    connect(m_player,&AVPlayer::frameChanged,ui->opengl_Widget,&OpenGLWidget::onShowYUV,Qt::QueuedConnection);
    connect(ui->btn_pauseon,&QPushButton::clicked,this,&MediaPlayer::pauseOnBtnClickSlot);
    connect(ui->listWidget_plalylist,&PlayListWidget::currentItemChanged,this,&MediaPlayer::playlistSelectionChangedSlot);
    connect(ui->btn_next,&QPushButton::clicked,this,&MediaPlayer::nextBtnSlot);
    connect(ui->btn_showSlidewidget,&QPushButton::clicked,this,&MediaPlayer::showSideBtnSlot);
    connect(ui->btn_addfile,&QPushButton::clicked,this,&MediaPlayer::addFileBtnSlot);
    connect(ui->btn_opendir,&QPushButton::clicked,this,&MediaPlayer::addFileBtnSlot);
    connect(m_player,&AVPlayer::AVDurationChanged,this,&MediaPlayer::durationChangedSlot);
    connect(m_player,&AVPlayer::AVPtsChanged,this,&MediaPlayer::ptsChangedSlot);
    connect(m_player,&AVPlayer::AVTerminate,this,&MediaPlayer::terminateSlot,Qt::QueuedConnection);

    connect(ui->slider_AVPts,&AVPtsSlider::sliderPressed,this,&MediaPlayer::ptsSliderPressSlot);
    connect(ui->slider_AVPts,&AVPtsSlider::sliderReleased,this,&MediaPlayer::ptsSliderReleaseSlot);
    connect(ui->slider_AVPts,&AVPtsSlider::sliderMoved,this,&MediaPlayer::ptsSliderMovedSlot);

    connect(ui->opengl_Widget,&OpenGLWidget::mouseDoubleClicked,this,[&](){
        if(isFullScreen()&&!ui->widget_side->isHidden()){
            showSideBtnSlot();
        }
        showFullScreenBtnSlot();
    });
    connect(ui->opengl_Widget,&OpenGLWidget::mouseClicked,[this](){
       if(isFullScreen()&&!ui->widget_side->isHidden()){
           showSideBtnSlot();
           return ;
       }
       if(ui->listWidget_plalylist->currentRow()!=-1){
           pauseOnBtnClickSlot();
       }
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
    connect(ui->btn_volume,&QPushButton::clicked,[this](){
        QPoint point=ui->widget_bottom->mapToGlobal(QPoint(ui->btn_volume->x(),0));
        point+=QPoint(-15,-VolumeDialog::getInstance()->height()-15);
        VolumeDialog::getInstance()->move(point);
        VolumeDialog::getInstance()->show();
        if(isFullScreen())
            setGeometry(m_actualFullScreenRect);
    });
    connect(ui->btn_config,&QPushButton::clicked,[this](){
        QPoint pos=ui->widget_bottom->mapToGlobal(QPoint(ui->btn_config->x(),0));
        pos+=QPoint(-ConfigDialog::getInstance()->width()*3/4,-ConfigDialog::getInstance()->height()-15);
        ConfigDialog::getInstance()->move(pos);
        ConfigDialog::getInstance()->show();
        if(isFullScreen())
            setGeometry(m_actualFullScreenRect);
    });
    connect(PlaySpeedDialog::getInstance(),&PlaySpeedDialog::playSpeedChanged,this,&MediaPlayer::playSpeedSlot);
    connect(VolumeDialog::getInstance(),&VolumeDialog::volumeValueChanged,this,&MediaPlayer::setVolume);
    connect(ConfigDialog::getInstance(),&ConfigDialog::playModeChanged,this,&MediaPlayer::playModeSlot);
    connect(ConfigDialog::getInstance(),&ConfigDialog::scaleRateChanged,this,&MediaPlayer::scaleRateSlot);
    m_showBtnTimer.setInterval(2*1000);
    connect(&m_showBtnTimer,&QTimer::timeout,[this](){
        ui->btn_showSlidewidget->hide();
        m_showBtnTimer.stop();
    });
    connect(&m_showBtnTimer,&QTimer::timeout,this,&MediaPlayer::hideBottomWidgetSlot);
    m_showVolumeTimer.setInterval(2*1000);
    connect(&m_showVolumeTimer,&QTimer::timeout,[this](){
        ui->label_central_volume->hide();
        m_showVolumeTimer.stop();
    });
    connect(VolumeDialog::getInstance(),&VolumeDialog::mouseEnter,this,&MediaPlayer::showBottomWidgetSlot);
    connect(PlaySpeedDialog::getInstance(),&PlaySpeedDialog::mouseEnter,this,&MediaPlayer::showBottomWidgetSlot);
    connect(ConfigDialog::getInstance(),&ConfigDialog::mouseEnter,this,&MediaPlayer::showBottomWidgetSlot);

    connect(ui->btn_clearlist,&QPushButton::clicked,this,&MediaPlayer::clearlistBtnSlot);
    connect(ui->listWidget_plalylist,&PlayListWidget::selectedItemRemoved,this,&MediaPlayer::removeSingleItemSlot);
    connect(ui->btn_fullscreen,&QPushButton::clicked,this,&MediaPlayer::showFullScreenBtnSlot);

    ui->btn_showSlidewidget->hide();
    ui->widget_side->hide();
    ui->widget_bottom_assis->hide();
    ui->widget_title_fullscreen->hide();
    ui->widget_space_side->hide();

    //默认列表循环
    ConfigDialog::getInstance()->setPlayMode(0);
    //原始比例渲染
    ConfigDialog::getInstance()->setScaleRate(0);
}

MediaPlayer::~MediaPlayer()
{

}

void MediaPlayer::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setBrush(QBrush(QColor(46,46,54)));
    painter.drawRect(this->rect());
}

void MediaPlayer::resizeEvent(QResizeEvent *event)
{
#ifdef DEBUG
    qDebug()<<"change brfore:";
    qDebug()<<"ui->openglwidget_container: "<<ui->openglwidget_container->geometry();
    qDebug()<<"ui->widget_center: "<<ui->widget_center->geometry();
#endif
    ui->openglwidget_container->setGeometry(0,0,ui->widget_center->width(),ui->widget_center->height());
#ifdef DEBUG
    qDebug()<<"change after: ";
    qDebug()<<"ui->openglwidget_container: "<<ui->openglwidget_container->geometry();
    qDebug()<<"ui->widget_center: "<<ui->widget_center->geometry();
#endif
    if(ui->widget_side->isHidden()){
        ui->btn_showSlidewidget->move(ui->widget_center->width()-ui->btn_showSlidewidget->width(),\
                                      (ui->widget_center->height()-ui->btn_showSlidewidget->height())/2);
    }else{
        ui->btn_showSlidewidget->move(ui->widget_center->width()-ui->widget_side->width()-ui->btn_showSlidewidget->width(),\
                                      (ui->widget_center->height()-ui->btn_showSlidewidget->height())/2);
    }
    ui->widget_side->setGeometry(ui->widget_center->width()-ui->widget_side->width(),0,\
                                 ui->widget_side->width(),ui->widget_center->height());
}

void MediaPlayer::keyReleaseEvent(QKeyEvent *event)
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

bool MediaPlayer::eventFilter(QObject *obj, QEvent *event)
{
    if(obj==ui->widget_center){
        if(event->type()==QEvent::MouseMove||event->type()==QEvent::Enter){
            showBottomWidgetSlot();
            m_showBtnTimer.start();
        }else if(event->type()==QEvent::Wheel){
            QMouseEvent* mouse=(QMouseEvent*)(event);
            QWidget* child=ui->widget_center->childAt(mouse->pos());
            if(!(child==ui->opengl_Widget||child==ui->widget_bottom))
                return true;
            QWheelEvent* e=dynamic_cast<QWheelEvent*>(event);
            m_showVolumeTimer.start();
            int volume;
            if(e->angleDelta().y()>0){
                volume=qMax(0,qMin(VolumeDialog::getInstance()->getVolume()+2,100));
            }else{
                volume=qMax(0,qMin(VolumeDialog::getInstance()->getVolume()-2,100));
            }
            VolumeDialog::getInstance()->setVolume(volume);
            ui->label_central_volume->setText(QString("音量：%1%").arg(volume));
            ui->label_central_volume->show();
        }else if(event->type()==QEvent::Leave){
            hideBottomWidgetSlot();
        }
    }else if(obj==ui->widget_bottom){
        if(event->type()==QEvent::Enter){
            m_bottomwidget_enter=true;
            showBottomWidgetSlot();
        }else if(event->type()==QEvent::Leave){
            m_bottomwidget_enter=false;
        }
        return true;
    }
    return DragableWidget::eventFilter(obj,event);
}

void MediaPlayer::closeBtnClickSlot()
{

}

void MediaPlayer::setVolume(int volume)
{
    static int lastVolume=volume;
    m_player->setVolume(volume);
    if(volume==0){
        ui->btn_volume->setStyleSheet("QPushButton#btn_volume{border: none;border-image: url(:/resource/image/novolume_normal.png);}"
                                      "QPushButton:hover#btn_volume{border-image: url(:/resource/image/novolume_hover.png);}");
        lastVolume=0;
    }else if(lastVolume==0){
        ui->btn_volume->setStyleSheet("QPushButton#btn_volume{border: none;border-image: url(:/resource/image/volume_normal.png);}"
                                      "QPushButton:hover#btn_volume{border-image: url(:/resource/image/volume_hover.png);}");
        lastVolume=volume;
    }
}

void MediaPlayer::setWidgetDefaultState()
{
    ui->label_pts->setText(QString("00:00:00"));
    ui->label_duration->setText(QString("00:00:00"));
    ui->label_playing->setText(QString(""));
    ui->label_playing_fullscreen->setText(QString(""));
    ui->btn_pauseon->setStyleSheet("QPushButton#btn_pauseon{"
                                   "border-image:url(:/resource/image/pause_normal.png)};"
                                   "QPushButton:hover#btn_pauseon{"
                                   "border-image:url(:/resource/image/pause_hover.png)};");
    ui->slider_AVPts->setPtsPercent(0.00);
    ui->slider_AVPts->setEnabled(false);
    ui->opengl_Widget->resetFrame();
    m_player->clearPlayer();
}

void MediaPlayer::enableReatedWidgets()
{
    if(ui->listWidget_plalylist->count()){
        ui->btn_pauseon->setEnabled(true);
        ui->btn_next->setEnabled(true);
        ui->btn_opendir->hide();
        ui->label_addfileTip->hide();
        if(ui->widget_side->isHidden()){
            showSideBtnSlot();
        }
    }else{
        ui->btn_pauseon->setEnabled(false);
        ui->btn_next->setEnabled(false);
        ui->btn_opendir->show();
        ui->label_addfileTip->show();
    }
}

void MediaPlayer::showPlayList()
{
    if(!isFullScreen()){
        ui->widget_space_side->show();
    }else{
        ui->widget_title_fullscreen->hide();
        ui->widget_bottom_assis->hide();
        ui->slider_AVPts->hide();
    }
    ui->btn_showSlidewidget->move(ui->widget_center->width()-ui->widget_side->width()-ui->btn_showSlidewidget->width(),\
                                  (ui->widget_center->height()-ui->btn_showSlidewidget->height())/2);
    ui->widget_side->show();
}

void MediaPlayer::hidePlayList()
{
    ui->widget_space_side->hide();
    ui->btn_showSlidewidget->move(ui->widget_center->width()-ui->btn_showSlidewidget->width(),\
                                  (ui->widget_center->height()-ui->btn_showSlidewidget->height())/2);
    ui->widget_side->hide();
    ui->widget_bottom_assis->show();
    ui->slider_AVPts->show();
}

void MediaPlayer::pauseOnBtnClickSlot()
{
    if(ConfigDialog::getInstance()->isFocusIn()||
            VolumeDialog::getInstance()->isFocusIn()||
            PlaySpeedDialog::getInstance()->isFocusIn()){
        return;
    }
    switch (m_player->playState()) {
    case AVPlayer::AV_PLAYING:
        m_player->pause(true);
        ui->btn_pauseon->setStyleSheet("QPushButton#btn_pauseon{"
                                       "border-image:url(:/resource/image/pause_normal.png)};"
                                       "QPushButton:hover#btn_pauseon{"
                                       "border-image:url(:/resource/image/pause_hover.png)};");
        break;
    case AVPlayer::AV_PAUSED:
        m_player->pause(false);
        ui->btn_pauseon->setStyleSheet("QPushButton#btn_pauseon{"
                                       "border-image:url(:/resource/image/play_normal.png)};"
                                       "QPushButton:hover#btn_pauseon{"
                                       "border-image:url(:/resource/image/play_hover.png)};");
        break;
    case AVPlayer::AV_STOPPED:
        nextBtnSlot();
        break;
    }
}

void MediaPlayer::showFullScreenBtnSlot()
{
    static bool isMaximum=false;
    static bool isShow=false;
    if(isFullScreen()){
        ui->btn_fullscreen->setStyleSheet("QPushButton#btn_fullscreen{"
                                          "border:none;"
                                          "border-image:url(:/resource/image/fullscreen_normal.png);}"
                                          "QPushButton:hover#btn_fullscreen{"
                                          "border-image:url(:/resource/image/fullscreen_hover.png);}");
        ui->widget_title->show();
        ui->widget_title_fullscreen->hide();
        showNormal();
        if(isMaximum)showMaximized();
        else showNormal();
        if(isShow){
            showSideBtnSlot();
            isShow=false;
        }
    }else{
        ui->btn_fullscreen->setStyleSheet("QPushButton#btn_fullscreen{"
                                          "border:none;"
                                          "border-image:url(:/resource/image/disfullscreen_normal.png);}"
                                          "QPushButton:hover#btn_fullscreen{"
                                          "border-image:url(:/resource/image/disfullscreen_hover.png);}");
        ui->widget_title->hide();
        if(!ui->widget_side->isHidden()){
            isShow=true;
            showSideBtnSlot();
        }
        if(isMaximized()){
            isMaximum=true;
            showNormal();
        }else{
            isMaximum=false;
        }
        showFullScreen();
    }
}

void MediaPlayer::addFileBtnSlot()
{
    QStringList url=QFileDialog::getOpenFileNames(this,"选择文件","E:/",m_vFmt);
    for(auto& iter:url){
        if(ui->listWidget_plalylist->isItemExisted(iter)){
            continue;
        }
        AVTool::MediaInfo* info=m_player->detectMediaInfo(iter);
        if(!info){
            continue;
        }
        ui->listWidget_plalylist->addSelfDefineItem(info->tipImg,info->duration,iter);
        delete  info;
    }
    enableReatedWidgets();
}

void MediaPlayer::ptsChangedSlot(unsigned int pts)
{
    if(m_ptsSliderPressed)return;
    ui->slider_AVPts->setPtsPercent((double)pts/m_duration);
    ui->label_pts->setText(QString("%1:%2:%3")\
                           .arg(pts/3600,2,10,QLatin1Char('0'))\
                           .arg(pts/60%60,2,10,QLatin1Char('0'))\
                           .arg(pts%60,2,10,QLatin1Char('0')));
}

void MediaPlayer::durationChangedSlot(unsigned int duration)
{
    m_duration=duration;
    ui->label_duration->setText(QString("%1:%2:%3")\
                                .arg(duration/3600,2,10,QLatin1Char('0'))\
                                .arg(duration/60%60,2,10,QLatin1Char('0'))\
                                .arg(duration%60,2,10,QLatin1Char('0')));
}

void MediaPlayer::terminateSlot()
{
    setWidgetDefaultState();
    playNextMedia();
}

void MediaPlayer::playNextMedia()
{
    int _row{0};
    switch (m_playMode) {
    case PlayMode::SINGLE_STOP:
        ui->listWidget_plalylist->setCurrentRow(-1);
        break;
    case PlayMode::LIST_LOOP:
        nextBtnSlot();
        break;
    case PlayMode::SEQUENCE:
        _row=ui->listWidget_plalylist->currentRow()+1;
        if(_row>=ui->listWidget_plalylist->count())
            _row=-1;
        ui->listWidget_plalylist->setCurrentRow(_row);
        break;
    case PlayMode::SINGLE_LOOP:
        _row=ui->listWidget_plalylist->currentRow();
        ui->listWidget_plalylist->setCurrentRow(-1);
        ui->listWidget_plalylist->setCurrentRow(_row);
        break;
    }
}

void MediaPlayer::ptsSliderPressSlot()
{
    m_ptsSliderPressed=true;
    m_seekTarget=(int)(ui->slider_AVPts->ptsPercent()*m_duration);
}

void MediaPlayer::ptsSliderMovedSlot()
{
    m_seekTarget=(int)(ui->slider_AVPts->ptsPercent()*m_duration);
    const QString& ptsStr=QString("%1:%2:%3").\
            arg(m_seekTarget/60/60,2,10,QLatin1Char('0')).\
            arg(m_seekTarget/60%60,2,10,QLatin1Char('0')).\
            arg(m_seekTarget%60,2,10,QLatin1Char('0'));
    if(m_ptsSliderPressed)
        ui->label_pts->setText(ptsStr);
    else
        ui->label_pts->setToolTip(ptsStr);
}

void MediaPlayer::ptsSliderReleaseSlot()
{
    if(m_ptsSliderPressed){
        m_player->seekTo(m_seekTarget);
        m_ptsSliderPressed=false;
    }
}

void MediaPlayer::seekForwardSlot()
{
    m_player->seekBy(6);
    if(m_player->playState()==AVPlayer::AV_PAUSED)
        m_player->pause(false);
}

void MediaPlayer::nextBtnSlot()
{
    if(ui->listWidget_plalylist->count()){
        int _row=(ui->listWidget_plalylist->currentRow()+1)%ui->listWidget_plalylist->count();
        if(_row==0){
            ui->listWidget_plalylist->setCurrentRow(-1);
        }
        ui->listWidget_plalylist->setCurrentRow(_row);
    }
}

void MediaPlayer::seekBackSlot()
{
    m_player->seekBy(-6);
    if(m_player->playState()==AVPlayer::AV_PAUSED)
        m_player->pause(false);
}

void MediaPlayer::playSpeedSlot(PlaySpeedDialog::Speed speed)
{
    switch (speed) {
    case PlaySpeedDialog::Speed::_0_5:
        m_player->setPlaySpeed(0.5);
        ui->btn_playspeed->setText("0.5x");
        break;
    case PlaySpeedDialog::Speed::_0_75:
        m_player->setPlaySpeed(0.75);
        ui->btn_playspeed->setText("0.75x");
        break;
    case PlaySpeedDialog::Speed::_1_0:
        m_player->setPlaySpeed(1.0);
        ui->btn_playspeed->setText("倍速");
        break;
    case PlaySpeedDialog::Speed::_1_25:
        m_player->setPlaySpeed(1.25);
        ui->btn_playspeed->setText("1.25x");
        break;
    case PlaySpeedDialog::Speed::_1_5:
        m_player->setPlaySpeed(1.5);
        ui->btn_playspeed->setText("1.5x");
        break;
    case PlaySpeedDialog::Speed::_2_0:
        m_player->setPlaySpeed(2.0);
        ui->btn_playspeed->setText("2.0x");
        break;
    default:
        break;
    }
}

void MediaPlayer::playModeSlot(int modeId)
{
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

void MediaPlayer::scaleRateSlot(int scaleRate)
{
    switch (scaleRate) {
    case 0:
        ui->opengl_Widget->setScaleRate(OpenGLWidget::ScaleRate::RATE_ORIGIN);
        break;
    case 1:
        ui->opengl_Widget->setScaleRate(OpenGLWidget::ScaleRate::RATE_4_3);
        break;
    case 2:
        ui->opengl_Widget->setScaleRate(OpenGLWidget::ScaleRate::RATE_16_9);
        break;
    case 3:
        ui->opengl_Widget->setScaleRate(OpenGLWidget::ScaleRate::RATE_FULLSCREEN);
        break;
    default:
        break;
    }
}

void MediaPlayer::showSideBtnSlot()
{
    if(ui->widget_side->isHidden()){
        ui->btn_showSlidewidget->setStyleSheet("QPushButton#btn_showSlidewidget{"
                                               "background-color:rgb(0,0,0,100);"
                                               "border-image:url(:/resource/image/hideside_normal.png);}"
                                               "QPushButton:hover#btn_showSlidewidget{"
                                               "border-image:url(:/resource/image/hideside_hover.png);}");
        showPlayList();
    }else{
        ui->btn_showSlidewidget->setStyleSheet("QPushButton#btn_showSlidewidget{"
                                               "background-color:rgb(0,0,0,100)"
                                               "border-image:url(:/resource/image/showside_normal.png);}"
                                               "QPushButton:hover#btn_showSlidewidget{"
                                               "border-image:url(:/resource/image/showside_hover.png);}");
        hidePlayList();
    }
}

void MediaPlayer::playlistSelectionChangedSlot(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(previous)
        setWidgetDefaultState();
    if(!current)return;
    PlayListWidgetItem* _item=dynamic_cast<PlayListWidgetItem*>(current);
    if(!m_player->Play(_item->url)){
        ui->listWidget_plalylist->setCurrentRow(-1);
        return;
    }
    ui->btn_pauseon->setStyleSheet("QPushButton#btn_pauseon{"
                                   "border-image: url(:/resource/image/play_normal.png);}"
                                   "QPushButton:hover#btn_pauseon{"
                                   "border-image:url(:/resource/image/play_hover.png);}");
    ui->slider_AVPts->setEnabled(true);
    ui->label_playing->setText(_item->filename);
    ui->label_playing_fullscreen->setText(_item->filename);
}

void MediaPlayer::clearlistBtnSlot()
{
    if(ui->listWidget_plalylist->currentRow()>=0){
        setWidgetDefaultState();
    }
    for(int i=0;i<ui->listWidget_plalylist->count();i++){
        PlayListWidgetItem* item=dynamic_cast<PlayListWidgetItem*>(ui->listWidget_plalylist->item(i));
        ui->listWidget_plalylist->removeItemWidget(item);
        delete  item->widget;
    }
    ui->listWidget_plalylist->clear();
    enableReatedWidgets();
}

void MediaPlayer::removeSingleItemSlot()
{
    enableReatedWidgets();
}

void MediaPlayer::showBottomWidgetSlot()
{
    ui->slider_AVPts->setStyleSheet("QSlider::groover:horizontal#slider_AVPts{border:0px;}"
                                    "QSlider::sub-page:horizontal#slider_AVPts{"
                                    "background-color:rgb(255,92,56);"
                                    "margin-top:7px;"
                                    "margin-bottom:7px;}"
                                    "QSlider::add-page:horizontal#slider_AVPts{"
                                    "background-color:rgb(83,83,83);"
                                    "margin-top:7px;"
                                    "margin-bottom:7px;}");
    QTime time=QTime::currentTime();
    ui->label_current_time->setText(QString("%1:%2").arg(time.hour(),2,10,QLatin1Char('0')).arg(time.minute(),2,10,QLatin1Char('0')));
    ui->btn_showSlidewidget->show();
    if(!(isFullScreen()&&!ui->widget_side->isHidden()))//如果是全屏显示且视频列表框也在的话那就不会显示该slider
        ui->widget_bottom_assis->show();//展示辅助选项
    if(isFullScreen()&&ui->widget_side->isHidden())
        ui->widget_title_fullscreen->show();
}

void MediaPlayer::hideBottomWidgetSlot()
{
    if(m_bottomwidget_enter||
            VolumeDialog::getInstance()->isEnter()||
            PlaySpeedDialog::getInstance()->isEnter()||
            ConfigDialog::getInstance()->isEnter()){
        return ;
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
    ui->widget_bottom_assis->hide();
}
