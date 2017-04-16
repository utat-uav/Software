#ifndef IMAGESETPROCESSOR_H
#define IMAGESETPROCESSOR_H

#include <QDialog>
#include <QFileDialog>
#include <QProcess>
#include <QMessageBox>
#include <QThread>
#include <atomic>

namespace Ui {
class ImageSetProcessor;
}

class ProcessThread;

class ImageSetProcessor : public QDialog
{
    Q_OBJECT

public:
    static void listFiles(QDir directory, QString indent, QList<QString> &files);

    explicit ImageSetProcessor(const QString &cnnPath, QWidget *parent = 0);
    ~ImageSetProcessor();

private slots:
    void on_browseImageSetFolder_clicked();

    void on_browseProcessedSaveFolder_clicked();

    void on_buttonBox_accepted();

    void on_browseProcessedFolder_clicked();

    void on_pushButton_clicked();

    void on_processButton_clicked();

    void closeEvent(QCloseEvent *event);

    void on_aioCheckbox_toggled(bool checked);

private:
    Ui::ImageSetProcessor *ui;

    std::atomic<int> imagesCompleted;
    std::atomic<bool> isProcessing;
    QString cnnPath;
    ProcessThread *worker;
};

/*
 * Quick class definition for the processing thread
 */
class ProcessThread : public QThread
{
    Q_OBJECT
public:
    ProcessThread(const QString &imageFolder, const QString &gpsLogFolder,
                  const QString &outputFolder, const QString &cnnPath, bool aio,
                  bool removeFalsePositives)
        : imageFolder(imageFolder), gpsLogFolder(gpsLogFolder), numImages(0),
          outputFolder(outputFolder), cnnPath(cnnPath), process(true), aio(aio),
          removeFalsePositives(removeFalsePositives)
    {}

    std::atomic<bool> process;
    std::atomic<int> numImages;

signals:
    void imageDone();

protected:
    void run();

    QString imageFolder;
    QString gpsLogFolder;
    QString outputFolder;
    QString cnnPath;

    std::atomic<bool> aio;
    std::atomic<bool> removeFalsePositives;
};

#endif // IMAGESETPROCESSOR_H
