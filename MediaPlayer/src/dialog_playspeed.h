#ifndef DIALOG_PLAYSPEED_H
#define DIALOG_PLAYSPEED_H

#include <QDialog>
#include <QListWidget>
#include <QListWidgetItem>


class PlaySpeedDialog :public QDialog
{
	Q_OBJECT

public:
    enum class Speed
	{
        _0_5,
        _0_75,
        _1_0,
        _1_25,
        _1_5,
        _2_0
	};

    static PlaySpeedDialog* getInstance();

    bool isFocusIn()
    {
        return m_isFocusIn;
    }

    bool isEnter()
    {
        return m_mouseEnter;
    }

signals:
    void playSpeedChanged(PlaySpeedDialog::Speed mode);
    void mouseEnter();

protected:
	void resizeEvent(QResizeEvent* event);
	void paintEvent(QPaintEvent* e);
	void focusInEvent(QFocusEvent* e);
	void focusOutEvent(QFocusEvent* e);
	void showEvent(QShowEvent* e);
    virtual void enterEvent(QEvent* e) override;
    virtual void leaveEvent(QEvent* e) override;

private:
    static PlaySpeedDialog* m_instance;
	QListWidget* listwidget;
	void addListItem();
	QPixmap m_shadow;
    bool m_isFocusIn;
    bool m_mouseEnter;

    PlaySpeedDialog(QWidget* parent = Q_NULLPTR);
    ~PlaySpeedDialog();
};

#endif
