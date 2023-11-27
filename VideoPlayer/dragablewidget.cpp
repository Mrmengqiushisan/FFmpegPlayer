#include "dragablewidget.h"
#include <QDesktopWidget>
#include <QDebug>

#pragma comment(lib, "Dwmapi.lib")
#include <QtWinExtras/QtWin>
#include <dwmapi.h>

#pragma comment (lib, "user32.lib")

#ifndef GET_X_LPARAM
#define GET_X_LPARAM(lParam)    ((int)(short)LOWORD(lParam))
#endif
#ifndef GET_Y_LPARAM
#define GET_Y_LPARAM(lParam)    ((int)(short)HIWORD(lParam))
#endif

#ifdef IS_DEBUG
#define IS_DEBUG 0
#endif

//调整尺寸判定边界内缩距离
const int DRAG_BORDER = 6;

DragableWidget::DragableWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaPlayer)
{
    ui->setupUi(this);
    initWidgets();
    singnalsLinkToSlots();
}

DragableWidget::~DragableWidget()
{
    delete  ui;
}
//当前只处理Window下的系统事件
bool DragableWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN
    if(eventType!="windows_generic_MSG")
        return false;
    MSG* msg=static_cast<MSG*>(message);
    QWidget* widget=QWidget::find(reinterpret_cast<WId>(msg->hwnd));
    if(!widget)return false;
    switch (msg->message) {
    //用于在窗口大小或位置发生变化时通知窗口框架进行非客户区大小的计算。
    case WM_NCCALCSIZE:{
        *result=0;
        return true;
    }
    //用于确定鼠标指针在非客户区（Non-Client Area）的位置，并判断该位置对应的区域。
    case WM_NCHITTEST:{
        int x=GET_X_LPARAM(msg->lParam);
        int y=GET_Y_LPARAM(msg->lParam);
        //用于将全局坐标转换为窗口坐标
        QPoint pt=mapFromGlobal(QPoint(x,y));
        //全屏时通知windows窗体全为客户区
        if(isFullScreen()){
            *result=HTCLIENT;
            return true;
        }
        *result=calculateBorder(pt);
        if(*result==HTCLIENT){
            QWidget* tempWidget=this->childAt(pt.x(),pt.y());
            if(tempWidget==ui->widget_title||tempWidget==ui->label_playing){
                *result=HTCAPTION;//表示鼠标目前在窗口标题栏上
            }
        }
        return true;
    }
    //用于在窗口大小变化时通知窗口框架获取关于窗口最小化和最大化信息的数据。
    case WM_GETMINMAXINFO:{
        MINMAXINFO* pMinMaxInfo=(MINMAXINFO*)msg->lParam;
        //设置最小和最大尺寸
        pMinMaxInfo->ptMinTrackSize.x=960;
        pMinMaxInfo->ptMinTrackSize.y=640;
        if(::IsZoomed(msg->hwnd)){
            m_isMaximized=true;
            RECT frame={0,0,0,0};
            //false 表示不需要菜单栏
            AdjustWindowRectEx(&frame,WS_OVERLAPPEDWINDOW,FALSE,0);
            frame.left=abs(frame.left);
            frame.top=abs(frame.bottom);
            widget->setContentsMargins(frame.left,frame.top,frame.right,frame.bottom);
        }else{
            widget->setContentsMargins(0,0,0,0);
            m_isMaximized=false;
        }
        *result=::DefWindowProc(msg->hwnd,msg->message,msg->wParam,msg->lParam);
        return true;
    }
        break;
    default:
        break;
    }
#endif
    return QWidget::nativeEvent(eventType, message, result);
}

void DragableWidget::changeEvent(QEvent *event)
{
    if(event->type()==QEvent::WindowStateChange){
        if(isMaximized()){
            ui->btn_max->setToolTip("还原");
            ui->btn_max->setStyleSheet("QPushButton#btn_max{"
                                       "border-image:url(:/resource/image/max_resume_normal.png);}"
                                       "QPushButton:hover#btn_max{"
                                       "border-image:url(:/resource/image/max_resume_hover.png)");
        }else if(isMinimized()){
            qDebug()<<"window minimized";
        }else{
            //窗口回复原始大小
            ui->btn_max->setToolTip("最大化");
            ui->btn_max->setStyleSheet("QPushButton#btn_max{"
                                       "border-image:url(:/resource/image/max_normal.png);}"
                                       "QPushButton:hover#btn_max{"
                                       "border-image:url(:/resource/image/max_hover.png)");
        }
    }
}

void DragableWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
}

bool DragableWidget::eventFilter(QObject *obj, QEvent *event)
{
    return QWidget::eventFilter(obj,event);
}

void DragableWidget::closeBtnClickSlot()
{
    close();
}

void DragableWidget::maxBtnClickSlot()
{
    if(isMaximized()) {
        ui->btn_max->setToolTip("最大化");
        showNormal();
        ui->btn_max->setStyleSheet("QPushButton{"
                                   "border-image:url(:/resource/image/max_normal.png);}"
                                   "QPushButton:hover#btn_max{"
                                   "border-image:url(:/resource/image/max_hover.png)");
        return;
    }
    ui->btn_max->setToolTip("还原");
    showMaximized();
    ui->btn_max->setStyleSheet("QPushButton{"
                               "border-image:url(:/resource/image/max_resume_normal.png);}"
                               "QPushButton:hover#btn_max{"
                               "border-image:url(:/resource/image/max_resume_hover.png)");
}

void DragableWidget::minBtnClickSlot()
{
    showMinimized();
}

void DragableWidget::on_btn_close_clicked()
{
    this->close();
}

void DragableWidget::on_btn_close_fullscreen_clicked()
{
    this->close();
}

bool DragableWidget::initWidgets()
{
    this->setWindowFlags(Qt::FramelessWindowHint|windowFlags());
    ui->label_sperator->setAlignment(Qt::AlignCenter);
    ui->label_pts->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->label_duration->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    //获取原生操作系统的句柄
    HWND hwnd=(HWND)this->winId();
    DWORD style=::GetWindowLong(hwnd,GWL_STYLE);
    ::SetWindowLong(hwnd,GWL_STYLE,style|WS_MAXIMIZEBOX|WS_THICKFRAME|WS_CAPTION);
    m_actualFullScreenRect=qApp->desktop()->screenGeometry();
    m_actualFullScreenRect.setY(-1);
    m_fullScreenRect=qApp->desktop()->screenGeometry();
}

LRESULT DragableWidget::calculateBorder(const QPoint &pt)
{
    //用于检查窗口是否处于最大化状态
    if(::IsZoomed((HWND)this->winId())){
        return HTCLIENT;
    }
    int bordersize=DRAG_BORDER;
    int cx=this->size().width();
    int cy=this->size().height();
    QRect rectTopLeft(0,0,bordersize,bordersize);
    if(rectTopLeft.contains(pt)){
        return HTTOPLEFT;
    }
    QRect rectLeft(0,bordersize,bordersize,cy-bordersize*2);
    if(rectLeft.contains(pt)){
        return HTLEFT;
    }
    QRect rectTopRight(cx-bordersize,0,bordersize,bordersize);
    if(rectTopRight.contains(pt)){
        return HTTOPRIGHT;
    }
    QRect rectRight(cx-bordersize,bordersize,bordersize,cy-bordersize*2);
    if(rectRight.contains(pt)){
        return HTRIGHT;
    }
    QRect rectTop(bordersize,0,cx-bordersize*2,bordersize);
    if(rectTop.contains(pt)){
        return HTTOP;
    }
    QRect rectBottomLeft(0,cy-bordersize,bordersize,bordersize);
    if(rectBottomLeft.contains(pt)){
        return HTBOTTOMLEFT;
    }
    QRect rectBottomRight(cx-bordersize,cy-bordersize,bordersize,bordersize);
    if(rectBottomRight.contains(pt)){
        return HTBOTTOMRIGHT;
    }
    QRect rectBottom(bordersize,cy-bordersize,cx-bordersize*2,bordersize);
    if(rectBottom.contains(pt)){
        return HTBOTTOM;
    }
    return HTCLIENT;
}

void DragableWidget::singnalsLinkToSlots()
{
    connect(ui->btn_max,&QPushButton::clicked,this,&DragableWidget::maxBtnClickSlot);
    connect(ui->btn_min,&QPushButton::clicked,this,&DragableWidget::minBtnClickSlot);
    connect(ui->btn_close,&QPushButton::clicked,this,&DragableWidget::closeBtnClickSlot);

    connect(ui->btn_close_fullscreen,&QPushButton::clicked,this,&DragableWidget::closeBtnClickSlot);
    connect(ui->btn_min_fullscreen,&QPushButton::clicked,this,&DragableWidget::minBtnClickSlot);
}
