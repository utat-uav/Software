#include "targetlistitem.h"

TargetListItem::TargetListItem(QString imageFilePath, QTableWidgetItem *i, QTableWidgetItem *n, QTableWidgetItem *c, QTableWidgetItem *d, int x, int y)
{
    image = i;
    name = n;
    coord = c;
    desc = d;
    this->x = x;
    this->y = y;

    this->imageFilePath = imageFilePath;

    if (desc->text()=="")
        desc->setText("File Path: " + this->imageFilePath);
}

TargetListItem::~TargetListItem()
{
    delete image;
    delete name;
    delete coord;
    delete desc;
}
