#pragma once
#include <QSlider>


class VolumeSliderBar :public QSlider
{
    Q_OBJECT

public:
    VolumeSliderBar(QWidget* parent = Q_NULLPTR);
    ~VolumeSliderBar();

    void setPercentValue(int per);
    int currentPercent() const;

signals:
    void sliderMoved(int per);
    void percentageChanged(int per);

protected:
    bool event(QEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void paintEvent(QPaintEvent* e);

private:
    int lastpos;
    int m_currentper;
};
