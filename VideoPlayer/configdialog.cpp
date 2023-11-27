#include "configdialog.h"
#include "ui_configdialog.h"
#include <QDebug>
#include <QPainter>
#include <QTimer>
ConfigDialog* ConfigDialog::m_instance=nullptr;

ConfigDialog::CHelper ConfigDialog::m_helper;

void ConfigDialog::setPlayMode(int modeId)
{
    switch (modeId) {
        case 0:
            ui->btn_mode0->setChecked(true);
            break;
        case 1:
            ui->btn_mode1->setChecked(true);
            break;
        case 2:
            ui->btn_mode2->setChecked(true);
            break;
        case 3:
            ui->btn_mode3->setChecked(true);
            break;
        default:
            break;
    }
    emit playModeChanged(modeId);
}

int ConfigDialog::playMode() const
{
    return  m_playmodeBtnGroup->checkedId();
}

void ConfigDialog::setScaleRate(int rateId)
{
    switch (rateId) {
        case 0:
            ui->btn_rate0->setChecked(true);
            break;
        case 1:
            ui->btn_rate1->setChecked(true);
            break;
        case 2:
            ui->btn_rate2->setChecked(true);
            break;
        case 3:
            ui->btn_rate3->setChecked(true);
            break;
        default:
            break;
    }
    emit scaleRateChanged(rateId);
}

int ConfigDialog::scaleRate() const
{
    return m_scaleRateBtnGroup->checkedId();
}

void ConfigDialog::mousePressEvent(QMouseEvent *event)
{
    QDialog::mousePressEvent(event);
}

void ConfigDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QBrush brush;
    brush.setColor(QColor(66,66,66,128));
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(),10,10);
}

void ConfigDialog::focusInEvent(QFocusEvent *event)
{
    qDebug()<<"focusIn";
    m_isFocusIn=true;
}

void ConfigDialog::focusOutEvent(QFocusEvent *event)
{
    qDebug()<<"focusout";
    hide();
    QTimer::singleShot(1000,[this](){
        m_isFocusIn=false;
    });
}

void ConfigDialog::showEvent(QShowEvent *event)
{
    qDebug()<<"showEvent";
    activateWindow();
    setFocus();
}

void ConfigDialog::enterEvent(QEvent *event)
{
    qDebug()<<"enterEvent";
    m_mouseEnter=true;
    emit mouseEnter();
}

void ConfigDialog::leaveEvent(QEvent *event)
{
    qDebug()<<"leaveEvent";
    m_mouseEnter=false;
    QTimer::singleShot(2000,[this](){
        if(!m_mouseEnter)clearFocus();
    });
}

ConfigDialog::ConfigDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigDialog),
    m_mouseEnter(false),
    m_isFocusIn(false)
{
    init();
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
    if(m_instance!=nullptr)
        releaseInstance();
}

void ConfigDialog::releaseInstance()
{
    if(m_instance){
        delete m_instance;
        m_instance=nullptr;
        qDebug()<<"config dialog control release";
    }
}

void ConfigDialog::init()
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint|windowFlags()|Qt::Tool);
    this->setAttribute(Qt::WA_TranslucentBackground);

    m_playmodeBtnGroup=new QButtonGroup(this);
    m_playmodeBtnGroup->addButton(ui->btn_mode0,0);
    m_playmodeBtnGroup->addButton(ui->btn_mode1,1);
    m_playmodeBtnGroup->addButton(ui->btn_mode2,2);
    m_playmodeBtnGroup->addButton(ui->btn_mode3,3);
    //设置互斥不可以同时选中
    m_playmodeBtnGroup->setExclusive(true);

    connect(m_playmodeBtnGroup,QOverload<int>::of(&QButtonGroup::buttonClicked),this,&ConfigDialog::playModeBtnClickedSlot);

    m_scaleRateBtnGroup=new QButtonGroup(this);
    m_scaleRateBtnGroup->addButton(ui->btn_rate0,0);
    m_scaleRateBtnGroup->addButton(ui->btn_rate1,1);
    m_scaleRateBtnGroup->addButton(ui->btn_rate2,2);
    m_scaleRateBtnGroup->addButton(ui->btn_rate3,3);
    m_scaleRateBtnGroup->setExclusive(true);

    connect(m_scaleRateBtnGroup,QOverload<int>::of(&QButtonGroup::buttonClicked),this,&ConfigDialog::scaleRateBtnClickedSlot);

}

void ConfigDialog::playModeBtnClickedSlot(int id)
{
    emit playModeChanged(id);
}

void ConfigDialog::scaleRateBtnClickedSlot(int id)
{
    emit scaleRateChanged(id);
}
