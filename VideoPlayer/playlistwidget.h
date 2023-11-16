#ifndef PLAYLISTWIDGET_H
#define PLAYLISTWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMouseEvent>

namespace Ui {
class PlayListWidget;
}

class PlayListWidgetItem;


class PlayListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit PlayListWidget(QWidget *parent = nullptr);

    bool isItemExisted(const QString& text);

    void addSelfDefineItem(const QImage& img,int duration,const QString& text);

    ~PlayListWidget();
signals:
    void selectedItemRemoved();
protected:

    bool event(QEvent *e) override;

    void mouseMoveEvent(QMouseEvent* e)override;
private slots:

    void selectionChangedSlot(QListWidgetItem* current,QListWidgetItem* previous);

    void itemEnteredSlot(QListWidgetItem* item);
private:
    PlayListWidgetItem* m_lastHoverItem;
};

class PlayListWidgetItem:public QListWidgetItem{
public:
    explicit PlayListWidgetItem(const QImage& img,int duration,const QString& text,QListWidget* view=nullptr);
    virtual ~PlayListWidgetItem();

    Ui::PlayListWidget *ui;

    QString url;

    QString filename;

    QWidget* widget;
private:
    void parseFileName(const QString& url);
};



#endif // PLAYLISTWIDGET_H
