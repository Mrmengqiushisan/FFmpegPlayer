#include "dialog_config.h"
#include "ui_configdialog.h"
#include <QPainter>
#include <QDebug>
#include <QTimer>
#include <QTime>
ConfigDialog* ConfigDialog::m_instance = nullptr;

ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::dialog_config)
    , m_mouseEnter(false)
    , m_isFoucsIn(false)
{
    init();
}

ConfigDialog::~ConfigDialog()
{
    delete ui;
}

void ConfigDialog::init()
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint|windowFlags()|Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);

    m_playmodeBtnGroup=new QButtonGroup(this);
    m_playmodeBtnGroup->addButton(ui->btn_mode_0,0);
    m_playmodeBtnGroup->addButton(ui->btn_mode_1,1);
    m_playmodeBtnGroup->addButton(ui->btn_mode_2,2);
    m_playmodeBtnGroup->addButton(ui->btn_mode_3,3);
    //设置互斥不可同时选中
    m_playmodeBtnGroup->setExclusive(true);

    connect(m_playmodeBtnGroup,QOverload<int>::of(&QButtonGroup::buttonClicked),this,&ConfigDialog::playModeBtnClickedSlot);

    m_scaleRateBtnGroup=new QButtonGroup(this);
    m_scaleRateBtnGroup->addButton(ui->btn_rate_0,0);
    m_scaleRateBtnGroup->addButton(ui->btn_rate_1,1);
    m_scaleRateBtnGroup->addButton(ui->btn_rate_2,2);
    m_scaleRateBtnGroup->addButton(ui->btn_rate_3,3);
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

ConfigDialog* ConfigDialog::getInstance()
{
    if (!m_instance) {
        m_instance = new ConfigDialog;
    }
    return m_instance;
}

void ConfigDialog::setPlayMode(int modeId)
{
    switch (modeId) {
        case 0:
            ui->btn_mode_0->setChecked(true);
            break;
        case 1:
            ui->btn_mode_1->setChecked(true);
            break;
        case 2:
            ui->btn_mode_2->setChecked(true);
            break;
        case 3:
            ui->btn_mode_3->setChecked(true);
            break;
        default:
            break;
    }
    emit playModeChanged(modeId);
}

int ConfigDialog::playMode(int value) const
{
    return m_playmodeBtnGroup->checkedId();
}

void ConfigDialog::setScaleRate(int rateId)
{
    switch (rateId) {
        case 0:
            ui->btn_rate_0->setChecked(true);
            break;
        case 1:
            ui->btn_rate_1->setChecked(true);
            break;
        case 2:
            ui->btn_rate_2->setChecked(true);
            break;
        case 3:
            ui->btn_rate_3->setChecked(true);
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

void ConfigDialog::mousePressEvent(QMouseEvent* e)
{
    QDialog::mousePressEvent(e);
}

void ConfigDialog::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QBrush brush;
    brush.setColor(QColor(66, 66, 66,128));
    brush.setStyle(Qt::SolidPattern);
    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(),10,10);
}

void ConfigDialog::focusInEvent(QFocusEvent* e)
{
    qDebug()<<"focusIn";
    m_isFoucsIn=true;
}

void ConfigDialog::focusOutEvent(QFocusEvent* e)
{
    qDebug()<<"focusout";
    hide();
    QTimer::singleShot(1000,[this]{
        m_isFoucsIn=false;
    });
}

void ConfigDialog::showEvent(QShowEvent* e)
{
    qDebug()<<"showEvent";
    activateWindow();
    setFocus();
}

void ConfigDialog::enterEvent(QEvent *e)
{
    qDebug()<<"enterEvent";
    m_mouseEnter=true;
    emit mouseEnter();
}

void ConfigDialog::leaveEvent(QEvent *e)
{
    qDebug()<<"leaveEvent";
    m_mouseEnter=false;
    QTimer::singleShot(2000,[this](){
        if(!m_mouseEnter)
            clearFocus();
    });
}


