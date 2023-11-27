#include "playspeeddialog.h"
#include <QPainter>
#include <QRect>
#include <QScrollBar>
#include <QTimer>
#define DEBUG
PlaySpeedDialog::CHelper PlaySpeedDialog::m_helper;

PlaySpeedDialog* PlaySpeedDialog::m_instance=nullptr;

PlaySpeedDialog *PlaySpeedDialog::getInstance(){
    if(m_instance==nullptr){
        m_instance=new PlaySpeedDialog();
    }
    return  m_instance;
}

void PlaySpeedDialog::resizeEvent(QResizeEvent *e)
{
    return QDialog::resizeEvent(e);
}

void PlaySpeedDialog::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QBrush brush;
    brush.setColor(QColor(46,46,46));
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);
    QPen pen;
    pen.setColor(QColor(46,46,46));
    pen.setWidth(1);
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
//    if(!m_shadow.isNull())
//        painter.drawPixmap(this->rect(),m_shadow);
    QRect rect(SHADOW_WIDTH,SHADOW_WIDTH,CONTENT_WIDTH,CONTENT_HEIGHT);
    painter.drawRoundedRect(rect,CONTENT_RADIUS,CONTENT_RADIUS);
    //该类是用于绘制2D图形的类，它可以绘制自定义的形状或者图形界面上创建特定的图案
    QPainterPath path2;
    path2.moveTo(SHADOW_WIDTH + (CONTENT_WIDTH - TRIANGLE_WIDTH) / 2.0 - 6.0, SHADOW_WIDTH + CONTENT_HEIGHT);
    path2.arcTo(SHADOW_WIDTH + (CONTENT_WIDTH - TRIANGLE_WIDTH) / 2.0 - 18.0, SHADOW_WIDTH + CONTENT_HEIGHT, 24, 24, 37, 53);
    path2.lineTo(SHADOW_WIDTH + (CONTENT_WIDTH - TRIANGLE_WIDTH) / 2.0 + 3.6, SHADOW_WIDTH + CONTENT_HEIGHT + 4.8);
    path2.lineTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0 - 0.8, SHADOW_WIDTH + CONTENT_HEIGHT + 14.9);
    path2.arcTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0 - 1.0, SHADOW_WIDTH + CONTENT_HEIGHT + 13.3, 2, 2, 217, 106);
    path2.lineTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0 + 0.8, SHADOW_WIDTH + CONTENT_HEIGHT + 14.9);
    path2.lineTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0 + 8.4, SHADOW_WIDTH + CONTENT_HEIGHT + 4.8);
    path2.arcTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0 + 6.0, SHADOW_WIDTH + CONTENT_HEIGHT, 24, 24, 90, 53);
    path2.lineTo(SHADOW_WIDTH + CONTENT_WIDTH / 2.0 + 18.0, SHADOW_WIDTH + CONTENT_HEIGHT);
    path2.closeSubpath();
    painter.fillPath(path2,QBrush(QColor(46,46,46)));
    pen.setColor(QColor(46,46,46));
    painter.setPen(pen);
    int baseY=62;
    for(int i=0;i<4;i++){
        painter.drawLine(SHADOW_WIDTH+10,baseY,width()-SHADOW_WIDTH-10,baseY);
        baseY+=48;
    }
}

void PlaySpeedDialog::focusInEvent(QFocusEvent *e)
{
#ifdef DEBUG
    qDebug()<<"focus in";
#endif
    m_isFocusIn=true;
}

void PlaySpeedDialog::focusOutEvent(QFocusEvent *e)
{
#ifdef DEBUG
    qDebug()<<"focus out";
#endif
    hide();
    QTimer::singleShot(1000,[this](){
        m_isFocusIn=false;
    });
}

void PlaySpeedDialog::showEvent(QShowEvent *)
{
    this->activateWindow();
    this->setFocus();
}

void PlaySpeedDialog::enterEvent(QEvent *e)
{
#ifdef DEBUG
    qDebug()<<"enter event";
#endif
    m_mouseEnter=true;
    emit mouseEnter();
}

void PlaySpeedDialog::leaveEvent(QEvent *e)
{
#ifdef DEBUG
    qDebug()<<"leave event";
#endif
    m_mouseEnter=false;
    QTimer::singleShot(2000,[this](){
        if(!m_mouseEnter){
            clearFocus();
        }
    });
}


PlaySpeedDialog::PlaySpeedDialog(QWidget *parent):QDialog(parent),m_isFocusIn(false),m_mouseEnter(false){
    this->resize(CONTENT_WIDTH+SHADOW_WIDTH*2,CONTENT_HEIGHT+SHADOW_WIDTH+TRIANGLE_HEIGHT);
    this->setWindowFlags(Qt::FramelessWindowHint|Qt::Tool);
    this->setFocusPolicy(Qt::ClickFocus);
    this->setAttribute(Qt::WA_TranslucentBackground);
    addListItem();
    m_shadow.load(":/resource/image/shadow_playmode_widget.png");
}

PlaySpeedDialog::~PlaySpeedDialog()
{
    if(m_instance)releaseInstance();
}

void PlaySpeedDialog::addListItem()
{
    listwidget=new QListWidget(this);
    listwidget->setGeometry(SHADOW_WIDTH+5,SHADOW_WIDTH+5,CONTENT_WIDTH-10,CONTENT_HEIGHT-10);
    listwidget->setAttribute(Qt::WA_TranslucentBackground);
    listwidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listwidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listwidget->verticalScrollBar()->setDisabled(true);
    listwidget->setAutoScroll(false);
    listwidget->setFont(QFont(QString("微软雅黑"),9));
    listwidget->setSelectionMode(QAbstractItemView::NoSelection);
    listwidget->setStyleSheet("QListWidget{border:none;color:rgb(255,255,255);background:transparent;outline:0px;}"
                              "QListWidget::Item{border:none;background:transparent;height:40px;border:0px;}"
                              "QListWidget::Item:hover{border:none;background:rgb(92,92,92)}");
    QListWidgetItem* item0=new QListWidgetItem(QString::fromLocal8Bit("2.0x"),listwidget);
    QListWidgetItem* item1=new QListWidgetItem(QString::fromLocal8Bit("1.5x"),listwidget);
    QListWidgetItem* item2=new QListWidgetItem(QString::fromLocal8Bit("1.25x"),listwidget);
    QListWidgetItem* item3=new QListWidgetItem(QString::fromLocal8Bit("1.0x"),listwidget);
    QListWidgetItem* item4=new QListWidgetItem(QString::fromLocal8Bit("0.75x"),listwidget);
    QListWidgetItem* item5=new QListWidgetItem(QString::fromLocal8Bit("0.5x"),listwidget);
    item0->setTextAlignment(Qt::AlignCenter);
    item1->setTextAlignment(Qt::AlignCenter);
    item2->setTextAlignment(Qt::AlignCenter);
    item3->setTextAlignment(Qt::AlignCenter);
    item4->setTextAlignment(Qt::AlignCenter);
    item5->setTextAlignment(Qt::AlignCenter);
    connect(listwidget,&QListWidget::currentRowChanged,this,[&](int itemid){
        switch (itemid) {
            case 0:
            emit playSpeedChanged(PlaySpeedDialog::Speed::_2_0);
            break;
            case 1:emit playSpeedChanged(PlaySpeedDialog::Speed::_1_5);
                break;
            case 2:emit playSpeedChanged(PlaySpeedDialog::Speed::_1_25);
                break;
            case 3:emit playSpeedChanged(PlaySpeedDialog::Speed::_1_0);
                break;
            case 4:emit playSpeedChanged(PlaySpeedDialog::Speed::_0_75);
                break;
            case 5:emit playSpeedChanged(PlaySpeedDialog::Speed::_0_5);
                break;
            default:
                break;
        }
    });
}


