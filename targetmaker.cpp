#include "targetmaker.h"
#include "ui_targetmaker.h"
#include <QFileDialog>

TargetMaker::TargetMaker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TargetMaker)
{
    ui->setupUi(this);

    accepted = false;
    defaultNameInput = "";
    defaultCoordInput = "";
    defaultDescInput = "";
    defaultFileInput = "";
}

TargetMaker::~TargetMaker()
{
    delete ui;
}

bool TargetMaker::getAccepted() const
{
    return this->accepted;
}

QString TargetMaker::getImageFilePath() const
{
    return ui->fileInput->text();
}

QString TargetMaker::getName() const
{
    return ui->nameInput->text();
}

QString TargetMaker::getDesc() const
{
    return ui->descriptionInput->toPlainText();
}

QString TargetMaker::getCoord() const
{
    return ui->coordinateInput->text();
}

void TargetMaker::setDefaultInputs()
{
    ui->fileInput->setText(defaultFileInput);
    ui->nameInput->setText(defaultNameInput);
    ui->coordinateInput->setText(defaultCoordInput);
    ui->descriptionInput->setText(defaultDescInput);
}

void TargetMaker::setDefaultNameInput(QString defaultNameInput)
{
    this->defaultNameInput = defaultNameInput;
}

void TargetMaker::setDefaultCoordInput(QString defaultCoordInput)
{
    this->defaultCoordInput = defaultCoordInput;
}

void TargetMaker::setDefaultDescInput(QString defaultDescInput)
{
    this->defaultDescInput = defaultDescInput;
}

void TargetMaker::setDefaultFileInput(QString defaultFileInput)
{
    this->defaultFileInput = defaultFileInput;
}

void TargetMaker::on_buttonBox_accepted()
{
    accepted = true;
}

void TargetMaker::on_buttonBox_rejected()
{
    accepted = false;
}

void TargetMaker::on_toolButton_clicked()
{
    // Creates file selection dialog
    QFileDialog fileDialog(NULL, tr("Select Preview Image"));
    fileDialog.setNameFilter(tr("Images (*.png *.bmp *.jpg)"));
    fileDialog.setModal(true);
    QString fileName = "";
    fileDialog.exec();

    // Sets text
    if (fileDialog.selectedFiles().length() > 0)
        fileName = fileDialog.selectedFiles().at(0);
    ui->fileInput->setText(fileName);
}
