#ifndef DIALOG_VOLUME_H
#define DIALOG_VOLUME_H

#include <QDialog>
#include <QLabel>
#include "slider_volume.h"

class MusicConsoleWidget;

class VolumeDialog :public QDialog
{
    Q_OBJECT
public:
    static VolumeDialog* getInstance();

    bool isFocusIn()
    {
        return m_isFocusIn;
    }

    bool isEnter()
    {
        return m_mouseEnter;
    }

    void setVolume(int value);
    int getVolume();

signals:
    void volumeValueChanged(int volume_per);
    void volumeStateChanged(bool issilence);
    void mouseEnter();

protected:
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void paintEvent(QPaintEvent* e) override;
    virtual void focusInEvent(QFocusEvent* e) override;
    virtual void focusOutEvent(QFocusEvent* e) override;
    virtual void showEvent(QShowEvent* e) override;
    virtual void enterEvent(QEvent* e) override;
    virtual void leaveEvent(QEvent* e) override;

private:
    VolumeDialog(QWidget* parent = Q_NULLPTR);
    void init();
private:
    static VolumeDialog* m_instance;
    VolumeSliderBar* m_volumesetslider;
    QLabel* showvolumelabel;
    bool m_isFocusIn;
    bool m_mouseEnter;
};

#endif
