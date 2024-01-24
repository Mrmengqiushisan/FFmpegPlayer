#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QWidget>
#include <QButtonGroup>
#include <QDialog>
namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    inline bool isFocusIn(){return m_isFocusIn;}
    inline bool isEnter(){return m_mouseEnter;}
    inline static ConfigDialog* getInstance(){
        if(m_instance==nullptr)
            m_instance=new ConfigDialog();
        return m_instance;
    }
    void setPlayMode(int modeId);
    int  playMode()const;
    void setScaleRate(int rateId);
    int  scaleRate()const;
signals:
    void playModeChanged(int playMode);
    void scaleRateChanged(int scaleRate);
    void mouseEnter();
    void overtimeHide();
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
private:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();
    static void releaseInstance();
    void init();
private slots:
    void playModeBtnClickedSlot(int id);
    void scaleRateBtnClickedSlot(int id);
private:
    Ui::ConfigDialog *ui;
    QButtonGroup* m_playmodeBtnGroup;
    QButtonGroup* m_scaleRateBtnGroup;
    static ConfigDialog* m_instance;
    bool m_mouseEnter;
    bool m_isFocusIn;
};

#endif // CONFIGDIALOG_H
