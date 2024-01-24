#include "volumedialog.h"
#include <QDebug>
#include <QTimer>
#include <QPainter>
#define SHADOW_WIDTH 0
#define CONTENT_WIDTH 60
#define CONTENT_HEIGHT 200
#define CONTENT_RADIUS 10
#define TRIANGLE_WIDTH 24
#define TRIANGLE_HEIGHT 16

VolumeDialog* VolumeDialog::m_instance=nullptr;


VolumeDialog::VolumeDialog(QWidget* parent):QDialog(parent),m_isFouesin(false),m_mouseEnter(false)
{
    init();
}

VolumeDialog::~VolumeDialog()
{
    if(m_instance!=nullptr)
        releaseInstance();
}

VolumeDialog *VolumeDialog::getInstance()
{
    if(m_instance==nullptr){
        m_instance=new VolumeDialog();
    }
    return  m_instance;
}

int VolumeDialog::getVolume()
{
    return m_vloumesetslider->currentPercent();
}

void VolumeDialog::setVolume(int value)
{
    m_vloumesetslider->setPercentValue(value);
}

void VolumeDialog::releaseInstance()
{
    if(m_instance!=nullptr){
        delete m_instance;
        m_instance=nullptr;
        qDebug()<<"volume dialog has release";
    }
}

void VolumeDialog::init()
{
    this->resize(CONTENT_WIDTH+SHADOW_WIDTH*2,CONTENT_HEIGHT+SHADOW_WIDTH+TRIANGLE_HEIGHT);
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground);
    showvolumelabel = new QLabel("", this);
    showvolumelabel->setObjectName("showvolumelabel");
    showvolumelabel->setStyleSheet("QLabel#showvolumelabel{font-family:Microsoft YaHei;\
                        font-size:15px;background:transparent;color:rgb(255,255,255);}");
    showvolumelabel->setAlignment(Qt::AlignCenter);
    showvolumelabel->setGeometry(QRect(10, 170, 40, 30));

    m_vloumesetslider=new VolumeSliderBar(this);
    m_vloumesetslider->setObjectName(QStringLiteral("slider_volume"));
    m_vloumesetslider->setFocusPolicy(Qt::NoFocus);
    m_vloumesetslider->setGeometry(QRect(24, 10, 12, 160));
    m_vloumesetslider->setMinimum(0);
    m_vloumesetslider->setMaximum(160);
    m_vloumesetslider->setSingleStep(1);
    m_vloumesetslider->setInvertedAppearance(false);
    m_vloumesetslider->setOrientation(Qt::Vertical);
    m_vloumesetslider->setStyleSheet("QSlider::groove:vertical#slider_volume{border:0px;}"
                                     "QSlider::add-page:vertical#slider_volume{background:rgb(241,241,241);margin-left:3px;margin-right:3px;}"
                                     "QSlider::sub-page:vertical#slider_volume{background:rgb(92,92,92);margin-left:3px;margin-right:3px;}"
                                     "QSlider::handle:vertical#slider_volume{background:rgb(255,255,255);border:none;height:12px;border-radius:6px;}");
    connect(m_vloumesetslider,&VolumeSliderBar::percentageChanged,this,[&](int per){
        showvolumelabel->setText(QString("%1%").arg(per));
        emit volumeValueChanged(per);
    });
    m_vloumesetslider->setPercentValue(30);
}

void VolumeDialog::mousePressEvent(QMouseEvent *e)
{
    QDialog::mousePressEvent(e);
}

void VolumeDialog::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QBrush brush;
    QPen pen;

    brush.setColor(QColor(46, 46, 46));
    brush.setStyle(Qt::SolidPattern);
    pen.setColor(QColor(46, 46, 46));
    pen.setWidth(1);
    painter.setBrush(brush);
    painter.setPen(pen);
    painter.drawRoundedRect(SHADOW_WIDTH, SHADOW_WIDTH, CONTENT_WIDTH, CONTENT_HEIGHT, CONTENT_RADIUS, CONTENT_RADIUS);

    QPainterPath path2;
    path2.moveTo(SHADOW_WIDTH+(CONTENT_WIDTH-TRIANGLE_WIDTH)/2.0-6.0, SHADOW_WIDTH+CONTENT_HEIGHT);
    path2.arcTo(SHADOW_WIDTH + (CONTENT_WIDTH - TRIANGLE_WIDTH) / 2.0-18.0, SHADOW_WIDTH + CONTENT_HEIGHT, 24, 24, 37, 53);
    path2.lineTo(SHADOW_WIDTH + (CONTENT_WIDTH - TRIANGLE_WIDTH) / 2.0+3.6, SHADOW_WIDTH + CONTENT_HEIGHT+4.8);
    path2.lineTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0-0.8, SHADOW_WIDTH + CONTENT_HEIGHT+14.9);
    path2.arcTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0 - 1.0, SHADOW_WIDTH + CONTENT_HEIGHT + 13.3, 2, 2, 217, 106);
    path2.lineTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0+0.8, SHADOW_WIDTH + CONTENT_HEIGHT + 14.9);
    path2.lineTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0 + 8.4, SHADOW_WIDTH + CONTENT_HEIGHT + 4.8);
    path2.arcTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0 + 6.0, SHADOW_WIDTH + CONTENT_HEIGHT, 24, 24, 90, 53);
    path2.lineTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0+18.0, SHADOW_WIDTH + CONTENT_HEIGHT);
    path2.closeSubpath();
    painter.fillPath(path2, QBrush(QColor(46, 46, 46)));
}

void VolumeDialog::focusInEvent(QFocusEvent *e)
{
    m_isFouesin=true;
}

void VolumeDialog::focusOutEvent(QFocusEvent *e)
{
    hide();
    QTimer::singleShot(1000,[this](){
        m_isFouesin=false;
    });
}

void VolumeDialog::showEvent(QShowEvent *e)
{
    activateWindow();
    setFocus();
}

void VolumeDialog::enterEvent(QEvent *e)
{
    m_mouseEnter=true;
    emit mouseEnter();
}

void VolumeDialog::leaveEvent(QEvent *e)
{
    m_mouseEnter=false;
    QTimer::singleShot(1000,[this](){
        if(!m_mouseEnter)
            clearFocus();
    });
}
