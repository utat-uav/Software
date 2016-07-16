#ifndef IMAGEWIDGET_H
#define IMAGEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPixmap>
#include <QSize>
#include "targetlistwindow.h"
#include <QMouseEvent>
#include <QTabWidget>
#include "mainwindow.h"
#include "lifesupport.h"

namespace Ui {
class ImageWidget;
}

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImageWidget(LifeSupport* dataPackage, MainWindow *parent, bool initTargetList = true);

    // Setters
    void setTitle(QString name);
    void setImage(QString imagePath);
    void setImage(QPixmap &resizedImage);
    void setFolderPath(QString folderPath) ;
    void setFilePath(QString filePath) ;
    void setImagePath(QString imagePath);
    void setTabWidget(QTabWidget* tabWidget ) ;
    void setNumTargets(int numTargets);
    void setSeen(bool seen);

    // Getters
    QString getTitle() const;
    QString getImagePath() const;
    QPixmap& getImage();
    QString getFolderPath() const;
    QString getFilePath() const;
    TargetListWindow* getTargetList() const;
    int getNumTargets() const;
    bool isInitialized() const;
    bool getSeen() const;

    void deleteTargetListWindow();

    void changeTargetListWindow(TargetListWindow* targetList, bool alreadyInitialized = true);

    ~ImageWidget();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
    QString title;
    int numTargets;
    QString filePath ;
    QString imagePath;
    QString folderPath;
    QPixmap image;
    TargetListWindow *targetList;
    bool targetListInitialized;
    bool seen;

private slots:


    void on_pinButton_clicked();

private:
    LifeSupport* dataPackage ;
    Ui::ImageWidget *ui;
    MainWindow* mainWindow ;
};

#endif // IMAGEWIDGET_H
