#include "imagesetprocessor.h"
#include "ui_imagesetprocessor.h"

#include <QCloseEvent>
#include <QDebug>

ImageSetProcessor::ImageSetProcessor(const QString &cnnPath, QWidget *parent) :
    QDialog(parent),
    cnnPath(cnnPath),
    ui(new Ui::ImageSetProcessor)
{
    ui->setupUi(this);
    isProcessing = false;
    worker = NULL;
}

ImageSetProcessor::~ImageSetProcessor()
{
    delete ui;
}

void ImageSetProcessor::on_browseImageSetFolder_clicked()
{
    ui->imageSetFolder->setText(QFileDialog::getExistingDirectory(this, tr("Load Directory..."), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void ImageSetProcessor::on_browseProcessedSaveFolder_clicked()
{
}

void ImageSetProcessor::listFiles(QDir directory, QString indent, QList<QString> &files)
{
    indent += "\t";
    QDir dir(directory);
    QFileInfoList list = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
    foreach(QFileInfo finfo, list) {
        files.append(finfo.filePath());
        if(finfo.isDir()) {
            listFiles(QDir(finfo.absoluteFilePath()), indent, files);
        }
    }
}

void ImageSetProcessor::on_buttonBox_accepted()
{
}

void ImageSetProcessor::on_browseProcessedFolder_clicked()
{
    ui->processedFolderName->setText(QFileDialog::getExistingDirectory(this, tr("Load Directory..."), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
}

void ImageSetProcessor::on_pushButton_clicked()
{
    ui->gpsLogPath->setText(QFileDialog::getOpenFileName(this,
            tr("Open Image"), "", tr("Log Files (*.log)")));
}

void ProcessThread::run()
{
    // Get application path
    QString exePath = QDir::currentPath();

    // Get potential files for classifier exe
    QList<QString> files;
    ImageSetProcessor::listFiles(exePath, "", files);

    QString classifierPath;

    // Find classifier exe
    bool found = false;
    for (int i = 0; i < files.size(); ++i)
    {
        if (files[i].contains("SoftmaxRegression.exe"))
        {
            classifierPath = files[i];
            found = true;
            break;
        }
    }
    if (!found)
    {
        QMessageBox msgBox;
        msgBox.setText("Classifier Script Not Found.");
        msgBox.exec();
        return;
    }

    // Get files
    QList<QString> images;
    ImageSetProcessor::listFiles(imageFolder, "", images);

    #pragma omp parallel for
    for (int i = 0; i < images.size(); i++)
    {
        if (!process) continue;

        // Check file name
        if (images[i].toStdString().find(".jpg") == std::string::npos &&
                images[i].toStdString().find(".png") == std::string::npos &&
                images[i].toStdString().find(".JPG") == std::string::npos &&
                images[i].toStdString().find(".PNG") == std::string::npos)
            continue;

        // Args:
        // ImageName GPSLogName OutputFolderName

        // Get args
        QStringList args;
        if (!aio)
        {
            args << "-identify" << images[i] << gpsLogFolder << outputFolder;
        }
        else
        {
            args << "-aio" << images[i] << gpsLogFolder << outputFolder << cnnPath;
        }

        // Make Process
        QProcess scriptProcess;

        // Start and wait
        scriptProcess.start(classifierPath, args);
        scriptProcess.waitForFinished(-1);
    }
}

void ImageSetProcessor::on_processButton_clicked()
{
    if (worker != NULL)
    {
        return;
    }

    // Get Info
    QString imageFolder = ui->imageSetFolder->text();
    QString gpsLogFolder = ui->gpsLogPath->text();
    QString outputFolder = ui->processedFolderName->text();

    if (imageFolder == "" || outputFolder == "")
    {
        return;
    }

    worker = new ProcessThread(imageFolder, gpsLogFolder,
                               outputFolder, cnnPath, ui->aioCheckbox->isChecked());

    this->setEnabled(false);
    isProcessing = true;

    connect(worker, &ProcessThread::finished, worker, &ProcessThread::deleteLater);
    connect(worker, &ProcessThread::destroyed, this, [=]()
    {
        this->setEnabled(true);
        this->isProcessing = false;
        this->worker = NULL;
    });

    worker->start();
}

void ImageSetProcessor::closeEvent(QCloseEvent *event)
{
    if (isProcessing)
    {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Terminate Processing",
                                                tr("Things are processing. Are you sure you want to cancel?"),
                                                QMessageBox::No | QMessageBox::Yes,
                                                QMessageBox::Yes);
        if (reply == QMessageBox::Yes)
        {
            if (worker)
            {
                worker->process = false;
                worker->wait();
            }
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->accept();
    }
}
