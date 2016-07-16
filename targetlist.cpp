#include "targetlist.h"
#include "targetlistitem.h"
#include <QTableWidgetItem>
#include <qDebug>
#include <QLabel>
#include <QHeaderView>

TargetList::TargetList(QTableWidget *targetListTable)
{
    table = targetListTable;

    rows = new QList<TargetListItem*>;
    defaultImagePath = ":/files/Untitled.png";
}

TargetList::~TargetList()
{
    delete rows;
}

QList<TargetListItem *>* TargetList::getRows() const
{
    return this->rows;
}

void TargetList::refreshTable()
{
    // Empty the table first
    for (int row = 0; row < table->rowCount(); row++) {
        table->takeItem(row, 0);
        table->takeItem(row, 1);
        table->takeItem(row, 2);
        table->takeItem(row, 3);
    }

    int rowCount = rows->length();
    table->setRowCount(rowCount);

    // Insert table items
    for (int row = 0; row < rowCount; row++) {
        // Preview Image
        table->setItem(row, 0, rows->at(row)->image);

        // Name
        table->setItem(row, 1, rows->at(row)->name);

        // Coordinates
        table->setItem(row, 2, rows->at(row)->coord);

        // Description
        table->setItem(row, 3, rows->at(row)->desc);
    }
}

void TargetList::addNewRow(QString fileName, QString name, QString coordinates, QString description, int x, int y, bool refresh)
{
    // Creates image preview item
    QTableWidgetItem *image = new QTableWidgetItem();
    QBrush brush;
    QImage brushImage;
    if (fileName == "")
        brushImage.load(defaultImagePath);
    else
        brushImage.load(fileName);
    // Resize image
    int width = 100;
    int height = 100;
    QImage scaledBrushImage = brushImage.scaled(width, height, Qt::IgnoreAspectRatio);
    // Apply resized image
    brush.setTextureImage(scaledBrushImage);
    image->setBackground(brush);

    // Creates name item
    QFont font("Segoe UI", 11, QFont::Bold);
    QTableWidgetItem *nameItem = new QTableWidgetItem();
    nameItem->setText(name);
    nameItem->setFont(font);

    // Creates coordinate item
    QTableWidgetItem *coordItem = new QTableWidgetItem();
    coordItem->setText(coordinates);

    // Creates description item
    QTableWidgetItem *descItem = new QTableWidgetItem();
    descItem->setText(description);

    // Places item in a TargetListItem and adds it to the target list
    TargetListItem *newItem = new TargetListItem(fileName, image, nameItem, coordItem, descItem, x, y);
    rows->append(newItem);

    if (refresh)
        refreshTable();
}

void TargetList::editRow(int row, QString fileName, QString name, QString coordinates, QString description)
{
    // Creates image preview item
    QBrush brush;
    QImage brushImage;
    if (fileName == "")
        brushImage.load(defaultImagePath);
    else
        brushImage.load(fileName);
    // Resize image
    int width = 100;
    int height = 100;
    QImage scaledBrushImage = brushImage.scaled(width, height, Qt::IgnoreAspectRatio);
    // Apply resized image
    brush.setTextureImage(scaledBrushImage);

    rows->at(row)->image->setBackground(brush);
    rows->at(row)->name->setText(name);
    rows->at(row)->coord->setText(coordinates);
    rows->at(row)->desc->setText(description);

    refreshTable();
}

void TargetList::deleteRow(int row)
{
    rows->removeAt(row);

    refreshTable();
}

void TargetList::moveUp(int row)
{
    if (row-1 >= 0) {
        rows->move(row, row-1);

        refreshTable();
    }
}

void TargetList::moveDown(int row)
{
    if (row+1 <= rows->length()-1) {
        rows->swap(row, row+1);

        refreshTable();
    }
}

void TargetList::sortName(int low, int high)
{
    const int MOVING_LEFT = 0;
    const int MOVING_RIGHT = 1;
    if (low < high) {
        int left = low, right = high, currentDirection = MOVING_LEFT;
        QString pivot = rows->at(low)->name->text();
        while (left < right) {
            if (currentDirection == MOVING_LEFT) {
                while (QString::compare(rows->at(right)->name->text(), pivot, Qt::CaseInsensitive) >= 0  && left < right)
                    right--;
                rows->swap(left, right);
                currentDirection = MOVING_RIGHT;
            }
            if (currentDirection == MOVING_RIGHT) {
                while (QString::compare(rows->at(left)->name->text(), pivot, Qt::CaseInsensitive) <= 0  && left < right)
                    left++;
                rows->swap(right, left);
                currentDirection = MOVING_LEFT;
            }
        }
        sortName(low, left-1);
        sortName(right+1, high);
    }
}

