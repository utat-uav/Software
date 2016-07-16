#ifndef ITEMMAKER_H
#define ITEMMAKER_H

#include <QDialog>
#include <QFileDialog>

namespace Ui {
class ItemMaker;
}

class ItemMaker : public QDialog
{
    Q_OBJECT

public:
    explicit ItemMaker(QWidget *parent = 0);
    QString getTitle();
    QString getFilePath();
    void setTitle(QString title);
    void setFilePath(QString path);
    bool getAccepted();
    ~ItemMaker();

private slots:
    void on_browseButton_clicked();

    void on_buttonBox_accepted();

private:
    bool accepted;
    Ui::ItemMaker *ui;
};

#endif // ITEMMAKER_H
