#ifndef IMAGESETPROCESSOR_H
#define IMAGESETPROCESSOR_H

#include <QDialog>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>

namespace Ui {
class ImageSetProcessor;
}

class ImageSetProcessor : public QDialog
{
    Q_OBJECT

public:
    static void listFiles(QDir directory, QString indent, QList<QString> &files);

    explicit ImageSetProcessor(QWidget *parent = 0);
    ~ImageSetProcessor();

private slots:
    void on_browseImageSetFolder_clicked();

    void on_browseProcessedSaveFolder_clicked();

    void on_buttonBox_accepted();

    void on_browsePythonPath_clicked();

    void on_browseProcessedFolder_clicked();

    void on_pushButton_clicked();

    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::ImageSetProcessor *ui;

};

#endif // IMAGESETPROCESSOR_H
