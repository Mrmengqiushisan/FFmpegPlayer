#ifndef VOLUMESLIDERBAR_H
#define VOLUMESLIDERBAR_H

#include <QObject>
#include <QSlider>

class VolumeSliderBar : public QSlider
{
    Q_OBJECT
public:
    VolumeSliderBar(QWidget* parent=nullptr);
    ~VolumeSliderBar();

    void setPercentValue(int per);
    int  currentPercent()const;

signals:
    void sliderMoved(int per);
    void percentageChanged(int per);

protected:
    bool event(QEvent *event) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void paintEvent(QPaintEvent *ev) override;
private:
    int lastPos;
    int m_currentper;
};

#endif // VOLUMESLIDERBAR_H
