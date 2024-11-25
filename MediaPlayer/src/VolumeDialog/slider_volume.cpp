#include "slider_volume.h"
#include <QMouseEvent>
#include <QPainter>
#include <QDebug>


VolumeSliderBar::VolumeSliderBar(QWidget* parent) :
    QSlider(parent),
    lastpos(0)
{

}

VolumeSliderBar::~VolumeSliderBar()
{
}

void VolumeSliderBar::setPercentValue(int per)
{
    if (per < 0 || per>100) return;
    float temp = per / 100.0;
    int per_tovalue =  temp* height();
    setValue(per_tovalue);
    m_currentper = per;
    emit percentageChanged(per);
}

int VolumeSliderBar::currentPercent() const
{
    return m_currentper;
}

bool VolumeSliderBar::event(QEvent* e)
{
    return QSlider::event(e);
}

void VolumeSliderBar::mousePressEvent(QMouseEvent* e)
{
    setValue(height()-e->pos().y());
    int value_toper = value()*100 / (height() - 1);
    qDebug()<<"pos().y():"<<e->pos().y();
    qDebug()<<"height:"<<height()<<" value:"<<value()<<" value_toper:"<<value_toper;
    emit percentageChanged(value_toper);
    m_currentper = value_toper;
    lastpos = value();
}

void VolumeSliderBar::mouseMoveEvent(QMouseEvent* e)
{
    setValue(height() - e->pos().y());
    int per_gap = (lastpos- value()) * 100 / (height()-1);
    if (per_gap >= 1|| per_gap <= -1) {
        int value_toper = value() * 100 / (height() - 1);
        lastpos=value();
        m_currentper = value_toper;
        emit sliderMoved(value_toper);
        emit percentageChanged(value_toper);
    }
}



void VolumeSliderBar::mouseReleaseEvent(QMouseEvent* e)
{
    emit sliderReleased();
}

void VolumeSliderBar::paintEvent(QPaintEvent* e)
{
    return  QSlider::paintEvent(e);
}
