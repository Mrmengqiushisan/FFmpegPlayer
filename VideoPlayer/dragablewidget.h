#ifndef DRAGABLEWIDGET_H
#define DRAGABLEWIDGET_H
#include "ui_mediaplayer.h"
#include <QWidget>

namespace Ui {
class MediaPlayer;
}

class DragableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DragableWidget(QWidget *parent = nullptr);
    virtual ~DragableWidget();
protected:
    //用于处理原生操作系统事件的函数
    //eventType 事件类型 message 原生事件数据结构的指针 result: 用来设置或修改事件的结果或状态
    bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    //这个函数用于捕捉和处理窗口的各种状态变化事件。当窗口状态发生变化时（比如最小化、最大化、失去焦点等），Qt 会发送对应的事件到窗口对象，
    void changeEvent(QEvent *) override;
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject* obj,QEvent* event)override;
protected slots:
    //关闭按钮点击
    virtual void closeBtnClickSlot();
    //最大化按钮点击
    virtual void maxBtnClickSlot();
    //最小化按钮点击
    virtual void minBtnClickSlot();
    void on_btn_close_clicked();
    void on_btn_close_fullscreen_clicked();
signals:
protected:
    Ui::MediaPlayer* ui;
    QRect m_fullScreenRect;
    QRect m_actualFullScreenRect;
private:
    bool m_isMaximized;//最大化标志位
private:
    bool    initWidgets();
    LRESULT calculateBorder(const QPoint& pt);//计算鼠标光标位置
    void    singnalsLinkToSlots();  //信号关联对应处理槽函数
};

#endif // DRAGABLEWIDGET_H
