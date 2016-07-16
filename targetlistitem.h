#ifndef TARGETLISTITEM_H
#define TARGETLISTITEM_H
#include <QTableWidgetItem>
#include <QLabel>
#include <QWidget>

// NOTE: This is a glorified struct. All it does is store information

class TargetListItem
{
public:
    TargetListItem(QString imageFilePath, QTableWidgetItem *i, QTableWidgetItem *n, QTableWidgetItem *c, QTableWidgetItem *d, int x, int y);
    QString imageFilePath; //= "";
    QTableWidgetItem *image;
    QTableWidgetItem *name;
    QTableWidgetItem *coord;
    QTableWidgetItem *desc;
    int x, y;
    ~TargetListItem();
};

#endif // TARGETLISTITEM_H
