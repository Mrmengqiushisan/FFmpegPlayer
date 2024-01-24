#ifndef PLAYSPEEDDIALOG_H
#define PLAYSPEEDDIALOG_H

#include <QObject>
#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QPaintEvent>
#include <QFocusEvent>
#include <QShowEvent>
#include <QEvent>
#include <QDebug>

#define SHADOW_WIDTH    10
#define CONTENT_WIDTH   80
#define CONTENT_HEIGHT  250
#define CONTENT_RADIUS  10
#define TRIANGLE_WIDTH  24
#define TRIANGLE_HEIGHT 16

class PlaySpeedDialog : public QDialog
{
    Q_OBJECT
public:
    enum class Speed{
        _0_5,
        _0_75,
        _1_0,
        _1_25,
        _1_5,
        _2_0
    };
    static PlaySpeedDialog* getInstance();
    inline bool isFocusIn(){return m_isFocusIn;}
    inline bool isEnter(){return m_mouseEnter;}
signals:
    void playSpeedChanged(PlaySpeedDialog::Speed mode);
    void mouseEnter();
protected:
    void resizeEvent(QResizeEvent *e) override;
    void paintEvent(QPaintEvent* e)override;
    void focusInEvent(QFocusEvent* e)override;
    void focusOutEvent(QFocusEvent* e)override;
    void showEvent(QShowEvent *) override;
    void enterEvent(QEvent* e)override;
    void leaveEvent(QEvent* e)override;
private:
    static PlaySpeedDialog* m_instance;
    static void releaseInstance(){
        if(m_instance!=nullptr){
            delete  m_instance;
            m_instance=nullptr;
            qDebug()<<"playspeeddialog controler has released";
        }
    }
    bool            m_isFocusIn;
    bool            m_mouseEnter;
    QListWidget*    listwidget;
    QPixmap         m_shadow;
private:
    PlaySpeedDialog(QWidget* parent=nullptr);
    ~PlaySpeedDialog();
    void addListItem();
};

#endif // PLAYSPEEDDIALOG_H
