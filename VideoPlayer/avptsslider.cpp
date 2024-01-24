#include "avptsslider.h"
#include <QEvent>
#include <QMouseEvent>
AVPtsSlider::AVPtsSlider(QWidget *parent):QSlider(parent),is_Enter(false),m_percent(0.00),m_cursorXPer(0.00){
    setMouseTracking(true);
}
AVPtsSlider::~AVPtsSlider(){

}
void AVPtsSlider::setPtsPercent(double percent){
    if(percent<=0.0||percent>=100)return;
    m_percent=percent;
    int value=(int)(this->width()*percent);
    this->setValue(value);
}

double AVPtsSlider::ptsPercent(){
    double percent=0.0;
    percent=this->value()/this->width();
    return  percent;
}

bool AVPtsSlider::event(QEvent *e){
    if(e->type()==QEvent::Enter){
        this->setStyleSheet(QString("QSlider::sub-page:horizontal#slider_AVPts{"
                                    "background:rgb(255,92,56);"
                                    "margin-top:5px;"
                                    "margin-bottom:5px;}"
                                    "QSlider::add-page:horizontal#slider_AVPts{"
                                    "background:rgb(83,83,83);"
                                    "margin-top:5px;"
                                    "margin-bottom:5px;}"
                                    "QSlider::handle:horizontal#slider_AVPts{"
                                     "background-color:rgb(255, 79, 74);"
                                     "border-radius:9px;"
                                     "border:none;"
                                     "width:18px;"
                                     "margin-top:0px;"
                                     "margin-bottom:0px;}"));
    }else if(e->type()==QEvent::Leave){
        this->setStyleSheet(QString("QSlider::sub-page:horizontal#slider_AVPts{"
                                    "background:rgb(255,92,56);"
                                    "margin-top:7px;"
                                    "margin-bottom:7px;}"
                                    "QSlider::add-page:horizontal#slider_AVPts{"
                                    "background:rgb(83,83,83);"
                                    "margin-top:7px;"
                                    "margin-bottom:7px;}"
                                    "QSlider::handle:horizontal#slider_AVPts{"
                                    "border-radius:0px;"
                                    "border:none;"
                                    "width:0px;"
                                    "margin-top:7px;"
                                    "margin-bottom:7px;}"));
    }else if(e->type()==QEvent::Resize){
        setMaximum(this->width()-1);
        setPtsPercent(m_percent);
        m_percent=ptsPercent();
    }
    return QSlider::event(e);
}

void AVPtsSlider::mousePressEvent(QMouseEvent *ev){
    this->setValue(ev->pos().x());
    emit sliderPressed();
    is_Enter=true;
}

void AVPtsSlider::mouseReleaseEvent(QMouseEvent *ev){
    Q_UNUSED(ev);
    m_percent=ptsPercent();
    emit sliderReleased();
    is_Enter=false;
}

void AVPtsSlider::mouseMoveEvent(QMouseEvent *ev){
    int x=ev->pos().x();
    if(x>width())x=width();
    if(x<0)x=0;
    m_cursorXPer=(double)(x)/width();
    if(is_Enter){
        setValue(x);
    }
    emit sliderMoved();
}

void AVPtsSlider::paintEvent(QPaintEvent *ev)
{
    return QSlider::paintEvent(ev);
}
