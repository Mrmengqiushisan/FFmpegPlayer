#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QWidget>

namespace Ui {
class ConfigDialog;
}

class ConfigDialog : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigDialog(QWidget *parent = nullptr);
    ~ConfigDialog();

private:
    Ui::ConfigDialog *ui;
};

#endif // CONFIGDIALOG_H
