#ifndef VOLUMEDIALOG_H
#define VOLUMEDIALOG_H

#include <QObject>
#include <QDialog>
#include <QWidget>
#include <QMouseEvent>
#include <QLabel>
#include "volumesliderbar.h"
class VolumeDialog : public QDialog
{
    Q_OBJECT
public:
    static VolumeDialog* getInstance();
    inline bool isFocusIn(){return m_isFouesin;}
    inline bool isEnter(){return m_mouseEnter;}
    int getVolume();
    void setVolume(int value);
private:
    VolumeDialog(QWidget* parent=nullptr);
    ~VolumeDialog();
    static void releaseInstance();
    void init();
signals:
    void volumeValueChanged(int volume_per);
    void volumeStateChaged(bool issilence);
    void mouseEnter();
protected:
    void mousePressEvent(QMouseEvent* e) override;
    void paintEvent(QPaintEvent* e)override;
    void focusInEvent(QFocusEvent* e)override;
    void focusOutEvent(QFocusEvent* e)override;
    void showEvent(QShowEvent *e) override;
    void enterEvent(QEvent* e)override;
    void leaveEvent(QEvent* e)override;
private:
    static VolumeDialog* m_instance;
    QLabel* showvolumelabel;
    VolumeSliderBar* m_vloumesetslider;
    bool m_isFouesin;
    bool m_mouseEnter;

};

#endif // VOLUMEDIALOG_H
