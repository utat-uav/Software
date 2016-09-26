#ifndef LOADINGBARDIALOG_H
#define LOADINGBARDIALOG_H

#include <QDialog>

namespace Ui {
class LoadingBarDialog;
}

class LoadingBarDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoadingBarDialog(QWidget *parent = 0);
    ~LoadingBarDialog();
    void setStatus(QString status);
    void setPercent(int percent); // 0 to 100

private:
    Ui::LoadingBarDialog *ui;
};

#endif // LOADINGBARDIALOG_H
