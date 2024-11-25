#include "playlistwidget.h"
#include <QDebug>
#include <QMouseEvent>
#include <QTimer>

PlayListWidget::PlayListWidget(QWidget *parent)
    : QListWidget(parent)
    , m_lastHoverItem(Q_NULLPTR)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    connect(this,&QListWidget::currentItemChanged,this,&PlayListWidget::selectionChangedSlot);
    connect(this,&QListWidget::itemEntered,this,&PlayListWidget::itemEnteredSlot);
}

bool PlayListWidget::isItemExisted(const QString &text)
{
    for(int i=0;i<this->count();i++) {
        PlayListWidgetItem* _item=dynamic_cast<PlayListWidgetItem*>(item(i));
        if(_item->url.compare(text)==0)
            return true;
    }
    return false;
}

void PlayListWidget::addSelfDefineItem(const QImage &img, int duration, const QString &text)
{

    PlayListWidgetItem* _item=new PlayListWidgetItem(img,duration,text,this);
    connect(_item->ui.btn_remove,&QPushButton::clicked,[this,_item](){
        bool selected=this->isItemSelected(_item);
        if(m_lastHoverItem==_item)
            m_lastHoverItem=Q_NULLPTR;
        delete _item->widget;
        delete _item;
        if(selected) {
            emit selectedItemRemoved();
        }
    });
}

void PlayListWidget::selectionChangedSlot(QListWidgetItem *current, QListWidgetItem *previous)
{
    if(previous) {
        PlayListWidgetItem* _item=dynamic_cast<PlayListWidgetItem*>(previous);
        if(itemWidget(_item))
            _item->ui.label_filename->setStyleSheet("QLabel#label_filename{color:rgb(255,255,255);font-size:17px;}");
    }
    if(current) {
        PlayListWidgetItem* _item=dynamic_cast<PlayListWidgetItem*>(current);
        _item->ui.label_filename->setStyleSheet("QLabel#label_filename{color:rgb(255,92,56);font-size:17px;}");
    }
}



void PlayListWidget::itemEnteredSlot(QListWidgetItem* item)
{
    PlayListWidgetItem* curItem = static_cast<PlayListWidgetItem*>(item);
    curItem->ui.btn_remove->show();
    if (m_lastHoverItem) {
        m_lastHoverItem->ui.btn_remove->hide();
    }
    m_lastHoverItem = curItem;
}

bool PlayListWidget::event(QEvent *e)
{

    if (e->type() == QEvent::Leave) {
        if (m_lastHoverItem) {
            m_lastHoverItem->ui.btn_remove->hide();
            m_lastHoverItem = nullptr;
        }
    }
    return QListWidget::event(e);
}

void PlayListWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(count()) {
        if(e->pos().y()>90*count()) {
            if(m_lastHoverItem)
                m_lastHoverItem->ui.btn_remove->hide();
                m_lastHoverItem = nullptr;
        }
    }
    return QListWidget::mouseMoveEvent(e);
}


//*********************************************************
PlayListWidgetItem::PlayListWidgetItem(const QImage &img, int duration, const QString &text,QListWidget *view)
    : widget(Q_NULLPTR)
{
    PlayListWidget* _view=dynamic_cast<PlayListWidget*>(view);
    widget=new QWidget;
    widget->setMouseTracking(true);
    ui.setupUi(widget);

    //label设置鼠标穿透，否则会使下层item接收不到鼠标事件导致悬停显示按钮失效
    ui.label_filename->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui.label_image->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui.label_duration->setAttribute(Qt::WA_TransparentForMouseEvents);

    ui.label_duration->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    ui.label_filename->setWordWrap(true);

    ui.btn_remove->setStyleSheet("QPushButton#btn_remove{"
                                 "border: none;"
                                 "border-image:url(:/Image/delete_normal.png);}"
                                 "QPushButton:hover#btn_remove{"
                                 "border-image:url(:/Image/delete_hover.png);}");
    ui.btn_remove->hide();

    if(!img.isNull())
        ui.label_image->setPixmap(QPixmap::fromImage(img.scaled(ui.label_image->size(),\
                                        Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
    ui.label_duration->setText(QString("%1:%2:%3").\
                               arg(duration/60/60,2,10,QLatin1Char('0')).\
                               arg(duration/60%60,2,10,QLatin1Char('0')).\
                               arg(duration%60,2,10,QLatin1Char('0')));
    url=text;
    parseFileName(text);
    ui.label_filename->setText(ui.label_filename->fontMetrics().elidedText(filename,\
                  Qt::ElideRight,ui.label_filename->width()*2.5,Qt::TextShowMnemonic));
    this->setToolTip(filename);
    view->addItem(this);
    view->setItemWidget(this,widget);
}

PlayListWidgetItem::~PlayListWidgetItem()
{
}

void PlayListWidgetItem::parseFileName(const QString &url)
{
    int index=url.lastIndexOf('/');
    if(index>=0)
        filename=url.mid(index+1);
}
