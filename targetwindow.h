#ifndef TARGETWINDOW_H
#define TARGETWINDOW_H

#include <QDialog>
#include "targetlistitem.h"
#include <QProcess>
#include <QSettings>
#include <atomic>
#include "lifesupport.h"

namespace Ui {
class TargetWindow;
}

class TargetWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TargetWindow(LifeSupport* dataPackage, TargetListItem *targetListItem, QWidget *parent = 0);
    ~TargetWindow();

private slots:

    void on_zbar_pressed();

    void on_classifyButton_pressed();

    void closeEvent(QCloseEvent *event);

private:
    void classify() ;
    void zbar() ;
    LifeSupport* dataPackage;
    Ui::TargetWindow *ui;
    TargetListItem *targetListItem;

    std::atomic<bool> processing;
};

#endif // TARGETWINDOW_H
