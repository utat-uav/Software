#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>
#include <QDebug>
#include <QThread>
#include <math.h>
#include <QItemSelectionModel>
#include <QModelIndexList>
#include <QtAlgorithms>
#include "itemmaker.h"
#include <QDirIterator>
#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QTableWidget>
#include <QProcess>
#include <QCompleter>
#include <QMessageBox>
#include <QStringListModel>
#include "lifesupport.h"
#include "imagesetprocessor.h"
#include "loadingbardialog.h"

namespace Ui {
class MainWindow;
}
class ImageWidget ;

class MainWindow;

// LOADER THREAD
class MainWindowLoader : public QThread
{
    Q_OBJECT
public:
    explicit MainWindowLoader(MainWindow *mainWindow, QString dir)
        : mainWindow(mainWindow), dir(dir) {}
    void run();
signals:
    void statusUpdate(int value);
private:
    MainWindow *mainWindow;
    QString dir;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    friend class MainWindowLoader;

    explicit MainWindow(QWidget *parent = 0);
    void addTab(QWidget* newTab, QString title);
    bool findTab (QWidget* tab);
    void refreshTable();
    QList<ImageWidget*>* getItems();
    ~MainWindow();
    LoadingBarDialog* loadingBarDialog;

signals:
    void statusUpdate(int value);

private slots:
    void on_loadButton_clicked();

    void on_addItemButton_clicked();

    void on_editButton_clicked();

    void on_deleteItemButton_clicked();

    void ReadOut();
    void ReadErr();

    friend class MainLoader;

    void on_MainWindow_destroyed();

    void on_tabWidget_tabCloseRequested(int index);

    void on_consoleCommander_returnPressed();

    void on_actionProcess_Image_Set_triggered();

protected:
    void resizeEvent(QResizeEvent *e);
    void resizeTable();
    void addItem(QString filePath);
    void appendItem(QString folderPath, QString filePath, QString imagePath, QString title, int numTargets);
    void setColumnCount(int col);
    void listFiles(QDir directory, QString indent, QList<QString> &files);
    void indexToCoordinates(int index, int *r, int *c);


private:
    Ui::MainWindow *ui;
    QList<ImageWidget *> *items;
    LifeSupport* dataPackage;
    QProcess *classifier;
    QCompleter *completer;
    QStringList prevCommands;
    QString currentFile, currentImage;
    int tableWidth;
    int cellHeight;
    int cellWidth;
    int rowCount;
    int colCount;
    bool noTabs; //if no images are open or if all of them are closed
    std::atomic<bool> loading;
};

#endif // MAINWINDOW_H
