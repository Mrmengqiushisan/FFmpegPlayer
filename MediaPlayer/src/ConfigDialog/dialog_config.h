#ifndef DIALOG_CONFIG_H
#define DIALOG_CONFIG_H

#include <QDialog>
#include <QLabel>
#include <QButtonGroup>

namespace Ui {class dialog_config;} // namespace Ui


class ConfigDialog :public QDialog
{
    Q_OBJECT
public:
    static ConfigDialog* getInstance();

    bool isFocusIn()
    {
        return m_isFoucsIn;
    }

    bool isEnter()
    {
        return m_mouseEnter;
    }

    void setPlayMode(int modeId);

    int playMode(int value) const;

    void setScaleRate(int rateId);
    int scaleRate() const;

signals:
    void playModeChanged(int playMode);
    void scaleRateChanged(int scaleRate);
    void mouseEnter();

    //鼠标离开窗口一定时间自动收起对话框
    void overtimeHide();

protected:
    virtual void mousePressEvent(QMouseEvent* e) override;
    virtual void paintEvent(QPaintEvent* e) override;
    virtual void focusInEvent(QFocusEvent* e) override;
    virtual void focusOutEvent(QFocusEvent* e) override;
    virtual void showEvent(QShowEvent* e) override;
    virtual void enterEvent(QEvent *e) override;
    virtual void leaveEvent(QEvent *e) override;

private:
    ConfigDialog(QWidget* parent = Q_NULLPTR);
    ~ConfigDialog();

    void init();

private slots:
    void playModeBtnClickedSlot(int id);
    void scaleRateBtnClickedSlot(int id);

private:
    static ConfigDialog* m_instance;
    Ui::dialog_config* ui;
    QButtonGroup* m_playmodeBtnGroup;
    QButtonGroup* m_scaleRateBtnGroup;

    bool m_mouseEnter;
    bool m_isFoucsIn;
};

#endif
