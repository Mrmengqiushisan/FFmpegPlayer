#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H
#include <QListWidget>
#include "ui_playlistitemwidget.h"


class PlayListWidgetItem;

class PlayListWidget : public QListWidget
{
    Q_OBJECT
public:
    PlayListWidget(QWidget *parent = Q_NULLPTR);

    bool isItemExisted(const QString &text);

    void addSelfDefineItem(const QImage& img,int duration,const QString& text);

signals:
    //选中item被移除信号
    void selectedItemRemoved();

protected:
    virtual bool event(QEvent *e) override;
    virtual void mouseMoveEvent(QMouseEvent* e) override;

private:
    PlayListWidgetItem* m_lastHoverItem;

private slots:
    void selectionChangedSlot(QListWidgetItem *current, QListWidgetItem *previous);

    void itemEnteredSlot(QListWidgetItem* item);

};



class PlayListWidgetItem : public QListWidgetItem
{
public:
    explicit PlayListWidgetItem(const QImage &img, int duration, const QString &text,QListWidget *view = Q_NULLPTR);
    virtual ~PlayListWidgetItem();

    Ui::Form ui;

    QString url;

    QString filename;

    QWidget* widget;
private:
    void parseFileName(const QString& url);
};

#endif // PLAYLISTWIDGET_H
