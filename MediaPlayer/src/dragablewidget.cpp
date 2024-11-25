#include "dragablewidget.h"
#include <QString>
#include <QFile>
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

#ifndef IS_DEBUG
#define IS_DEBUG 0
#endif

//调整尺寸判定边界内缩距离
const int DRAG_BORDER = 6;

DragableWidget::DragableWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    initWidgets();
    singnalsLinkToSlots();
}

DragableWidget::~DragableWidget()
{
    delete ui;
}

void DragableWidget::initWidgets()
{
    setWindowFlags(Qt::FramelessWindowHint|windowFlags());

    ui->label_seperator->setAlignment(Qt::AlignCenter);
    ui->label_pts->setAlignment(Qt::AlignRight|Qt::AlignVCenter);
    ui->label_duration->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);

    HWND hwnd = (HWND)this->winId();
    DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
    ::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION);
    //const MARGINS shadow={1,1,1,1};
    //DwmExtendFrameIntoClientArea(HWND(winId()),&shadow);
    m_actualFullScreenRect = qApp->desktop()->screenGeometry();
    m_actualFullScreenRect.setY(-1);
    qDebug()<<"DragableWidget m_fullScreenRect: "<<m_actualFullScreenRect;
    m_fullScreenRect=qApp->desktop()->screenGeometry();
}

void DragableWidget::singnalsLinkToSlots()
{
    connect(ui->btn_max,&QPushButton::clicked,this,&DragableWidget::maxBtnClickSlot);
    connect(ui->btn_min,&QPushButton::clicked,this,&DragableWidget::minBtnClickSlot);
    connect(ui->btn_close,&QPushButton::clicked,this,&DragableWidget::closeBtnClickSlot);

    connect(ui->btn_close_fullscreen,&QPushButton::clicked,this,&DragableWidget::closeBtnClickSlot);
    connect(ui->btn_min_fullscreen,&QPushButton::clicked,this,&DragableWidget::minBtnClickSlot);
}

bool DragableWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
#ifdef Q_OS_WIN
    if (eventType != "windows_generic_MSG")
        return false;

    MSG* msg = static_cast<MSG*>(message);

    QWidget* widget = QWidget::find(reinterpret_cast<WId>(msg->hwnd));
    if (!widget)
        return false;

    switch (msg->message) {
    case WM_NCCALCSIZE: {
        *result = 0;
        return true;
    }

    case WM_NCHITTEST: {
        int x = GET_X_LPARAM(msg->lParam);
        int y = GET_Y_LPARAM(msg->lParam);

        QPoint pt = mapFromGlobal(QPoint(x, y));
        //全屏时候通知windows窗体全为客户区
        if(isFullScreen()) {
            *result=HTCLIENT;
            return true;
        }
        *result = calculateBorder(pt);
        if (*result == HTCLIENT) {
            QWidget* tempWidget = this->childAt(pt.x(), pt.y());
            if (tempWidget == ui->widget_title||
                tempWidget==ui->label_playing) {
                *result = HTCAPTION;
            }
        }
        return true;
    }

    case WM_GETMINMAXINFO: {
        MINMAXINFO* pMinMaxInfo = (MINMAXINFO*)msg->lParam;

        // 设置最小和最大尺寸
        pMinMaxInfo->ptMinTrackSize.x = 960; // 最小宽度
        pMinMaxInfo->ptMinTrackSize.y = 640; // 最小高度

        if (::IsZoomed(msg->hwnd)) {
            m_isMaximized = true;
            RECT frame = { 0, 0, 0, 0 };
            AdjustWindowRectEx(&frame, WS_OVERLAPPEDWINDOW, FALSE, 0);
            frame.left = abs(frame.left);
            frame.top = abs(frame.bottom);
            widget->setContentsMargins(frame.left, frame.top, frame.right, frame.bottom);
        }
        else {
            widget->setContentsMargins(0, 0, 0, 0);
            m_isMaximized = false;
        }

        *result = ::DefWindowProc(msg->hwnd, msg->message, msg->wParam, msg->lParam);
        return true;
    }
    break;

    default:
        break;
    }

#endif

    return QWidget::nativeEvent(eventType, message, result);
}

void DragableWidget::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (isMaximized()) {
            ui->btn_max->setToolTip("还原");
            ui->btn_max->setStyleSheet("QPushButton#btn_max{"
                                       "border-image: url(:/Image/max_resume_normal.png);}"
                                       "QPushButton:hover#btn_max{"
                                       "border-image: url(:/Image/max_resume_hover.png);}");
        } else if (isMinimized()) {
            // 窗口最小化
            //qDebug() << "Window minimized";
        } else {
            // 窗口恢复原始大小
            ui->btn_max->setToolTip("最大化");
            ui->btn_max->setStyleSheet("QPushButton#btn_max{"
                                       "border-image: url(:/Image/max_normal.png);}"
                                       "QPushButton:hover#btn_max{"
                                       "border-image: url(:/Image/max_hover.png);}");
        }
    }
}


LRESULT DragableWidget::calculateBorder(const QPoint &pt)
{
    if (::IsZoomed((HWND)this->winId())) {
        return HTCLIENT;
    }
    int borderSize = DRAG_BORDER;
    int cx = this->size().width();
    int cy = this->size().height();

    QRect rectTopLeft(0, 0, borderSize, borderSize);
    if (rectTopLeft.contains(pt)) {
        return HTTOPLEFT;
    }

    QRect rectLeft(0, borderSize, borderSize, cy - borderSize * 2);
    if (rectLeft.contains(pt)) {
        return HTLEFT;
    }

    QRect rectTopRight(cx - borderSize, 0, borderSize, borderSize);
    if (rectTopRight.contains(pt)) {
        return HTTOPRIGHT;
    }

    QRect rectRight(cx - borderSize, borderSize, borderSize, cy - borderSize * 2);
    if (rectRight.contains(pt)) {
        return HTRIGHT;
    }

    QRect rectTop(borderSize, 0, cx - borderSize * 2, borderSize);
    if (rectTop.contains(pt)) {
        return HTTOP;
    }

    QRect rectBottomLeft(0, cy - borderSize, borderSize, borderSize);
    if (rectBottomLeft.contains(pt)) {
        return HTBOTTOMLEFT;
    }

    QRect rectBottomRight(cx - borderSize, cy - borderSize, borderSize, borderSize);
    if (rectBottomRight.contains(pt)) {
        return HTBOTTOMRIGHT;
    }

    QRect rectBottom(borderSize, cy - borderSize, cx - borderSize * 2, borderSize);
    if (rectBottom.contains(pt)) {
        return HTBOTTOM;
    }

    return HTCLIENT;
}

void  DragableWidget::resizeEvent(QResizeEvent* event)
{
    Q_UNUSED(event)
}

bool DragableWidget::eventFilter(QObject* obj,QEvent* event)
{
    return QWidget::eventFilter(obj,event);
}

void DragableWidget::maxBtnClickSlot()
{
    if(isMaximized()) {
        ui->btn_max->setToolTip("最大化");
        showNormal();
        ui->btn_max->setStyleSheet("QPushButton#btn_max{"
                                   "border-image: url(:/Image/max_normal.png);}"
                                   "QPushButton:hover#btn_max{"
                                   "border-image: url(:/Image/max_hover.png);}");
        return;
    }
    ui->btn_max->setToolTip("还原");
    showMaximized();
    ui->btn_max->setStyleSheet("QPushButton#btn_max{"
                               "border-image: url(:/Image/max_resume_normal.png);}"
                               "QPushButton:hover#btn_max{"
                               "border-image: url(:/Image/max_resume_hover.png);}");
}

void DragableWidget::minBtnClickSlot()
{
    showMinimized();
}

void DragableWidget::closeBtnClickSlot()
{
    close();
}

