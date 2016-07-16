#include "itemmaker.h"
#include "ui_itemmaker.h"

ItemMaker::ItemMaker(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ItemMaker)
{
    accepted = false;
    ui->setupUi(this);
}

ItemMaker::~ItemMaker()
{
    delete ui;
}

void ItemMaker::on_browseButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Files (*.*)"));
    ui->URLInput->setText(fileName);
}

void ItemMaker::on_buttonBox_accepted()
{
    accepted = true;
}

bool ItemMaker::getAccepted()
{
    return this->accepted;
}

QString ItemMaker::getTitle()
{
    return ui->titleInput->text();
}

QString ItemMaker::getFilePath()
{
    return ui->URLInput->text();
}

void ItemMaker::setTitle(QString title)
{
    ui->titleInput->setText(title);
}

void ItemMaker::setFilePath(QString path)
{
    ui->URLInput->setText(path);
}
