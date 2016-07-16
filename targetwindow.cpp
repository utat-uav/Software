#include "targetwindow.h"
#include "ui_targetwindow.h"

TargetWindow::TargetWindow(LifeSupport *dataPackage, TargetListItem *targetListItem, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TargetWindow)
{
    this->targetListItem = targetListItem;
    ui->setupUi(this);

    // Set image
    QPixmap *pix = new QPixmap();
    QPixmap pic;
    pix->load(targetListItem->imageFilePath);
    pic = pix->scaled(280, 280, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation) ;
    ui->targetPic->setPixmap(pic);
    delete pix;

    this->dataPackage = dataPackage;

    // Set all other information
    ui->nameLabel->setText(targetListItem->name->text());
    ui->coordinatesLabel->setText(targetListItem->coord->text());
    ui->description->moveCursor(QTextCursor::End);
    ui->description->insertPlainText(targetListItem->desc->text());
}

TargetWindow::~TargetWindow()
{
    delete ui;
}

void TargetWindow::on_zbar_pressed()
{
    QString str = targetListItem->imageFilePath ;
    dataPackage->classifier->write("zbar "+str.toLatin1()+"\n");
    connect(dataPackage->consoleOutput,&QTextBrowser::textChanged, this, &TargetWindow::zbar) ;
}

void TargetWindow::on_classifyButton_pressed()
{
    dataPackage->classifier->write("classify "+targetListItem->imageFilePath.toLatin1()+"\n");
    connect(dataPackage->consoleOutput, &QTextBrowser::textChanged, this, &TargetWindow::classify);
}

void TargetWindow::on_classifyWithRotation_clicked()
{
    dataPackage->classifier->write("classifyWithRotation "+targetListItem->imageFilePath.toLatin1()+"\n");
    connect(dataPackage->consoleOutput, &QTextBrowser::textChanged, this, &TargetWindow::classify);
}

void TargetWindow::classify(){
    QString str = dataPackage->consoleOutput->toHtml() ;
    str.remove(0,str.lastIndexOf("Classified as")) ;
    if ( !str.contains("valid")){
        str.truncate(str.indexOf("confidence")+10) ;
        QSettings resultFile(dataPackage->filePath, QSettings::IniFormat);
        for ( int i = 1 ; i <= resultFile.value("Crop Info/Number of Crops").toInt() ; i ++ ){
            QString imageName = resultFile.value("Crop "+QString::number(i)+"/Image Name").toString() ;
            if ( imageName == dataPackage->imagePath ){
                resultFile.setValue("Crop "+QString::number(i)+"/Description",str);
            }
        }
        targetListItem->desc->setText(str);
    }
    disconnect(dataPackage->consoleOutput, &QTextBrowser::textChanged, this, &TargetWindow::classify);
    this->accept();
}

void TargetWindow::zbar(){
    QString str = dataPackage->consoleOutput->toHtml() ;
    str.remove(0,str.lastIndexOf("zbar"));
    if ( !str.contains("valid")){
        str.truncate(str.indexOf("<")) ;
        QSettings resultFile(dataPackage->filePath, QSettings::IniFormat);
        for ( int i = 1 ; i <= resultFile.value("Crop Info/Number of Crops").toInt() ; i ++ ){
            QString imageName = resultFile.value("Crop "+QString::number(i)+"/Image Name").toString() ;
            if ( dataPackage->imagePath == imageName ){
                resultFile.setValue("Crop "+QString::number(i)+"/Description",str);
            }
        }
        targetListItem->desc->setText(str);
    }
    disconnect(dataPackage->consoleOutput,&QTextBrowser::textChanged, this, &TargetWindow::zbar) ;
    this->accept();
}

void TargetWindow::on_zbar_clicked()
{

}
