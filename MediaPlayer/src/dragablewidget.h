/***************************************************************************************
 * 项目名: Mediaplayer
 * 作者: Leime-syc
 * 创建时间: 2023年10月24日
 * 描述: 这是一个简单的视频播放器，此项目采用FFMPEG+QT+SDL编写，可实现主流视频格式的解码播放
 * 所有的解封装解码操作都创建了单独的子线程执行，赋予了主线程足够的性能刷新界面保保障界面的流畅度
 * 音频流解封装解码后采用SDL回调送入声卡进行播放，视频帧则送入OpenGL窗口进行渲染，由于OpenGL采
 * 用硬件渲染，CPU的占用率非常低
 ***************************************************************************************/
#ifndef DRAGABLEWIDGET_H
#define DRAGABLEWIDGET_H

#include "ui_mainwindow.h"

class DragableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DragableWidget(QWidget *parent = nullptr);
    virtual ~DragableWidget();
protected:
    virtual bool nativeEvent(const QByteArray &eventType, void *message, long *result) override;
    void changeEvent(QEvent *event) override;
    virtual void resizeEvent(QResizeEvent* event) override;
    virtual bool eventFilter(QObject* obj,QEvent* event) override;

    //关闭按钮点击
    virtual void closeBtnClickSlot();

    //最大化按钮点击
    virtual void maxBtnClickSlot();

    //最小化按钮点击
    virtual void minBtnClickSlot();

protected:
    Ui::Widget *ui;

    QRect m_fullScreenRect;

    QRect m_actualFullScreenRect;

private:
    //最大化标志位
    bool m_isMaximized;

private:
    //初始化
    void initWidgets();

    //计算鼠标光标区域
    LRESULT calculateBorder(const QPoint &pt);

    //信号关联对应处理槽函数
    void singnalsLinkToSlots();
};

#endif // DRAGABLEWIDGET_H
