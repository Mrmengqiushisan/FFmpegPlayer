#include "volumesliderbar.h"
#include <QMouseEvent>
#include <QDebug>
VolumeSliderBar::VolumeSliderBar(QWidget* parent):QSlider(parent),lastPos(0)
{

}

VolumeSliderBar::~VolumeSliderBar()
{

}

void VolumeSliderBar::setPercentValue(int per)
{
    if(per<0||per>100)return;
    double temp=per/100.0;
    int set_value=height()*temp;
    setValue(set_value);
    m_currentper=per;
    lastPos=value();
    emit percentageChanged(per);
}

int VolumeSliderBar::currentPercent() const
{
    return m_currentper;
}

bool VolumeSliderBar::event(QEvent *event)
{
    return QSlider::event(event);
}

void VolumeSliderBar::mousePressEvent(QMouseEvent *ev)
{
    setValue(height()-ev->pos().y());
    int value_toper=value()*100/(height()-1);
    qDebug()<<"height:"<<height()<<" value:"<<value()<<" pos.y:"<<ev->pos().y()<<" value_toper:"<<value_toper;
    emit percentageChanged(value_toper);
    m_currentper=value_toper;
    lastPos=value();
}

void VolumeSliderBar::mouseMoveEvent(QMouseEvent *ev)
{
    setValue(height()-ev->pos().y());
    int per_tap=(lastPos-value())*100/(height()-1);
    if(per_tap>=1||per_tap<=-1){
        int value_toper=value()*100/(height()-1);
        lastPos=value();
        m_currentper=value_toper;
        emit sliderMoved(value_toper);
        emit percentageChanged(value_toper);
    }
}

void VolumeSliderBar::mouseReleaseEvent(QMouseEvent *ev)
{
    emit sliderReleased();
}

void VolumeSliderBar::paintEvent(QPaintEvent *ev)
{
    QSlider::paintEvent(ev);
}
