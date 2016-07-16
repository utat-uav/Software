#include "imagesetprocessor.h"
#include "ui_imagesetprocessor.h"

ImageSetProcessor::ImageSetProcessor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImageSetProcessor)
{
    ui->setupUi(this);
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

void ImageSetProcessor::on_browsePythonPath_clicked()
{
    ui->pythonPath->setText(QFileDialog::getExistingDirectory(this, tr("Load Directory..."), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks));
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

void ImageSetProcessor::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button->text() == "OK") {
        // Get application path
        QString exePath = QDir::currentPath();

        // Get potential files for classifier exe
        QList<QString> files;
        listFiles(exePath, "", files);

        QString scriptPath;

        // Find classifier exe
        bool found = false;
        for (int i = 0; i < files.size(); ++i)
        {
            if (files[i].contains("fextract.py"))
            {
                scriptPath = files[i];
                found = true;
            }
        }
        if (!found)
        {
            QMessageBox msgBox;
            msgBox.setText("Python Script Not Found.");
            msgBox.exec();
            return;
        }

        // Get Info
        QString imageFolder = ui->imageSetFolder->text();
        QString gpsLogFolder = ui->gpsLogPath->text();
        QString outputFolder = ui->processedFolderName->text();

        // Get files
        QList<QString> images;
        listFiles(imageFolder, "", images);

        #pragma omp parallel for
        for (int i = 0; i < images.size(); i++)
        {
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
            args << scriptPath << images[i] << gpsLogFolder << outputFolder;

            // Make Process
            QProcess scriptProcess;

            // Set process working directory
            int pos = scriptPath.toStdString().find_last_of('/');
            QString workingDirectory = QString::fromStdString(scriptPath.toStdString().substr(0, pos));
            scriptProcess.setWorkingDirectory(workingDirectory);

            // Start and wait
            scriptProcess.start(ui->pythonPath->text() + "\\python.exe", args);
            scriptProcess.waitForFinished(-1);

            //QMessageBox msgBox;
            //msgBox.setText(scriptProcess.readAllStandardError());
            //msgBox.exec();
        }
    }
}
