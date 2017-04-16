#ifndef TARGETLISTWINDOW_H
#define TARGETLISTWINDOW_H

#include <QDialog>
#include "targetmaker.h"
#include "targetwindow.h"
#include "targetlist.h"
#include <QSettings>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QThread>
#include "lifesupport.h"

class ImageWidget;
class TargetListWindow;

// LOADER THREAD
class Loader : public QThread
{
public:
    explicit Loader(TargetList *targetList, ImageWidget *imageWidget,
                    QString folderPath, QString filePath)
    {
        this->imageWidget = imageWidget;
        this->targetList = targetList;
        this->folderPath = folderPath;
        this->filePath = filePath;
    }
    void run();
private:
    ImageWidget *imageWidget;
    TargetList *targetList;
    QString folderPath, filePath;
};

namespace Ui {
    class TargetListWindow;
    class CustomLabel;
}

class TargetListWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TargetListWindow(LifeSupport* dataPackage, QWidget *parent = 0);
    Ui::TargetListWindow *ui ;
    QWidget *parent;
    ImageWidget *parentWidget;
    ~TargetListWindow();
    void setMainPic (QString imagePath) ;
    void loadTargets (QString folderPath, QString filePath) ;
    //QString selected();
    //void changeDesc(QString desc) ;

private slots:
    void on_newItem_clicked();
    void on_edit_clicked();
    void on_deleteButton_clicked();
    void on_upButton_clicked();
    void on_downButton_clicked();
    void sort(int col);
    void on_targetListTable_doubleClicked(const QModelIndex &index);
    void on_targetListTable_clicked(const QModelIndex &index);

private:
    //TargetMaker *targetMaker;
    TargetMaker *targetEditor;
    Loader *loader;
    QPixmap mainpic ;
    int mainPicWidth, mainPicHeight;
    QSettings *resultFile;
    TargetList *targetList;
    int colCount;
    LifeSupport* data;
};

class CustomLabel : public QLabel
{
    Q_OBJECT
    signals:
        void mousePressed(const QPoint& );
    public:
        CustomLabel( QWidget* parent=0, Qt::WindowFlags f =0 );
        CustomLabel( const QString& text, QWidget* parent = 0, Qt::WindowFlags f = 0);

        void mousePressedEvent( QMouseEvent* ev);
};

#endif // TARGETLISTWINDOW_H
