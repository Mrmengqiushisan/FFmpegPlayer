#ifndef AVPTSSLIDER_H
#define AVPTSSLIDER_H

#include <QSlider>

class AVPtsSlider : public QSlider{
    Q_OBJECT
public:
    AVPtsSlider(QWidget* parent=nullptr);
    ~AVPtsSlider();
    void setPtsPercent(double percent);
    double ptsPercent();
    inline double cursorXPercent(){
        return m_cursorXPer;
    }
signals:
    void valueChanged(int percent);
    void sliderPressed();
    void sliderReleased();
    void sliderMoved();
protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
private:
    bool is_Enter;
    double m_percent;
    double m_cursorXPer;
};

#endif // AVPTSSLIDER_H
