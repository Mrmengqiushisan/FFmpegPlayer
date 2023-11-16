#include "VideoPlayer.h"
#include "ui_VideoPlayerget.h"
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QString>
#include <QSharedPointer>
#include <QUrl>
#include <QFileDialog>
#include <QKeyEvent>
Q_DECLARE_METATYPE(QSharedPointer<YUV422Frame>)

VideoPlayer::VideoPlayer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VideoPlayer)
    , m_duration(1)
    , m_seekTarget(0)
    , m_ptsSliderPressed(false)
    , m_vFmt("视频文件(*.mp4 *.mov *.avi *.mkv *.wmv *.flv *.webm *.mpeg *.mpg *.3gp *.m4v *.rmvb *.vob *.ts *.mts *.m2ts *.f4v *.divx *.xvid)"){
    ui->setupUi(this);
    this->setWindowTitle(QString("AideoPlayer"));
    this->setWindowIcon(QIcon(":/resource/image/imageLogo.png"));
    ui->label_volume->setAlignment(Qt::AlignCenter);
    ui->label_duration->setAlignment(Qt::AlignCenter);
    ui->label_pts->setAlignment(Qt::AlignCenter);
    ui->lineEdit_input->setAlignment(Qt::AlignCenter);
    //设置默认选项
    ui->btn_pauseon->setEnabled(false);
    ui->btn_forward->setEnabled(false);
    ui->btn_back->setEnabled(false);
    ui->slider_AVPts->setEnabled(false);
    //数据注册
    qRegisterMetaType<QSharedPointer<YUV422Frame>>("QSharedPointer<YUV422Frame>");
    //设置默认值
    m_player=new AVPlayer();
    ui->lineEdit_input->setText(QString("E:/AudioVProject/xcz.mp4"));
    connect(ui->btn_addfile,&QPushButton::clicked,this,&VideoPlayer::openFile);
    connect(m_player,&AVPlayer::frameChanged,ui->opengl_widget,&OpenGLWidget::onShowYUV,Qt::QueuedConnection);
    connect(ui->btn_play,&QPushButton::clicked,this,[&](){
        QString str=ui->lineEdit_input->text();
        if(!str.isEmpty()){
            if(m_player->Play(str)){
                ui->btn_pauseon->setEnabled(true);
                ui->btn_forward->setEnabled(true);
                ui->btn_back->setEnabled(true);
                ui->slider_AVPts->setEnabled(true);
            }
        }
    });
    connect(ui->slider_volume,&QSlider::valueChanged,this,&VideoPlayer::setVolume);
    connect(ui->btn_pauseon,&QPushButton::clicked,this,&VideoPlayer::pauseOnBtnClickSlot);
    connect(ui->opengl_widget,&OpenGLWidget::mouseDoubleClicked,this,[&](){
        if(this->isMaximized()){
            this->showNormal();
        }else{
            this->showMaximized();
        }
    });
    connect(ui->opengl_widget,&OpenGLWidget::mouseClicked,this,&VideoPlayer::pauseOnBtnClickSlot);
    connect(ui->btn_forward,&QPushButton::clicked,this,&VideoPlayer::seekForwardSlot);
    connect(ui->btn_back,&QPushButton::clicked,this,&VideoPlayer::seekBackSlot);
    connect(m_player,&AVPlayer::AVDurationChanged,this,&VideoPlayer::durationChangedSlot);
    connect(m_player,&AVPlayer::AVPtsChanged,this,&VideoPlayer::ptsChangedSlot);
    connect(m_player,&AVPlayer::AVTerminate,this,&VideoPlayer::terminateSlot);
    connect(ui->slider_AVPts,&AVPtsSlider::sliderPressed,this,&VideoPlayer::ptsSliderPressSlot);
    connect(ui->slider_AVPts,&AVPtsSlider::sliderMoved,this,&VideoPlayer::ptsSliderMovedSlot);
    connect(ui->slider_AVPts,&AVPtsSlider::sliderReleased,this,&VideoPlayer::ptsSliderReleaseSlot);

    ui->slider_volume->installEventFilter(this);
}

VideoPlayer::~VideoPlayer(){
    delete ui;
}

void VideoPlayer::paintEvent(QPaintEvent *event){
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setBrush(QBrush(QColor(46,46,54)));
    painter.drawRect(rect());
}

void VideoPlayer::keyReleaseEvent(QKeyEvent *event){
    switch (event->key())
    {
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

void VideoPlayer::resizeEvent(QResizeEvent *event){

}

bool VideoPlayer::eventFilter(QObject *watched, QEvent *event){
    if(event->type()==QEvent::KeyPress||event->type()==QEvent::KeyRelease){
        if(watched==ui->slider_volume){
            event->ignore();
            return  true;
        }
    }
    return QObject::eventFilter(watched,event);
}

void VideoPlayer::openFile(){
    QString url=QFileDialog::getOpenFileName(this,"选择文件","E:/",m_vFmt);
    ui->lineEdit_input->setText(url);
}

void VideoPlayer::setVolume(int volume){
    m_player->setVolume(volume);
    ui->label_volume->setText(QString("%1%").arg(volume));
}

void VideoPlayer::ptsChangedSlot(unsigned int pts){
    if(m_ptsSliderPressed)return;
    ui->slider_AVPts->setPtsPercent((double)pts/m_duration);
    ui->label_pts->setText(QString("%1:%2").arg(pts/60,2,10,QLatin1Char('0'))\
                           .arg(pts%60,2,10,QLatin1Char('0')));

}

void VideoPlayer::durationChangedSlot(unsigned int duration){
    ui->label_duration->setText(QString("%1:%2").arg(duration/60,2,10,QLatin1Char('0')).arg(duration%60,2,10,QLatin1Char('0')));
    m_duration=duration;
}

void VideoPlayer::terminateSlot(){
    ui->label_pts->setText(QString("00:00"));
    ui->label_duration->setText(QString("00:00"));
    ui->slider_AVPts->setEnabled(false);
    ui->btn_back->setEnabled(false);
    ui->btn_forward->setEnabled(false);
    ui->btn_pauseon->setEnabled(false);
    m_player->clearPlayer();
}

void VideoPlayer::ptsSliderPressSlot(){
    m_ptsSliderPressed=true;
    m_seekTarget=(int)(ui->slider_AVPts->ptsPercent()*m_duration);
}

void VideoPlayer::ptsSliderMovedSlot(){
    m_seekTarget=(int)(ui->slider_AVPts->cursorXPercent()*m_duration);
    const QString& ptsstr=QString("%1:%2").arg(m_seekTarget/60,2,10,QLatin1Char('0'))\
            .arg(m_seekTarget%60,2,10,QLatin1Char('0'));
    if(m_ptsSliderPressed)ui->label_pts->setText(ptsstr);
    else
        ui->slider_AVPts->setToolTip(ptsstr);
}

void VideoPlayer::ptsSliderReleaseSlot(){
    m_player->seekTo(m_seekTarget);
    m_ptsSliderPressed=false;
}

void VideoPlayer::seekForwardSlot(){
    m_player->seekBy(6);
    if(m_player->playState()==AVPlayer::AV_PAUSED)
        m_player->pause(false);
}

void VideoPlayer::seekBackSlot(){
    m_player->seekBy(-6);
    if(m_player->playState()==AVPlayer::AV_PAUSED)
        m_player->pause(false);
}

void VideoPlayer::pauseOnBtnClickSlot(){
    switch (m_player->playState()) {
        case AVPlayer::AV_PLAYING:
            m_player->pause(1);
            ui->btn_pauseon->setText(QString("继续"));
            break;
        case AVPlayer::AV_PAUSED:
            m_player->pause(0);
            ui->btn_pauseon->setText(tr("暂停"));
            break;
        default:
            break;
    }
}

