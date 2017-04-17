#include "missionviewer.h"
#include "ui_missionviewer.h"

#include <QCloseEvent>
#include <QDebug>
#include <QToolBar>
#include <QGraphicsEllipseItem>

#include "imagewidget.h"
#include "missionview.h"

#define SCALE_FACTOR 50000.0

MissionViewer::MissionViewer(QList<ImageWidget *> *items, QWidget *parent) :
    QDialog(parent),
    items(items),
    ui(new Ui::MissionViewer)
{
    ui->setupUi(this);

//    QToolBar *menubar = new QToolBar();
//    this->layout()->setMenuBar(menubar);
//    menubar->addAction(ui->actionrefresh);
}

MissionViewer::~MissionViewer()
{
    delete ui;
}

void MissionViewer::closeEvent(QCloseEvent *event)
{
    this->hide();
    event->ignore();
}

void MissionViewer::show()
{
    QDialog::show();
    ui->graphicsView->fitInView(ui->graphicsView->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}

void MissionViewer::refresh()
{
    on_actionrefresh_triggered();
}

void MissionViewer::on_actionrefresh_triggered()
{
    qDebug() << "Refreshing mission viewer";
    QPen imagePen;
    imagePen.setColor(Qt::red);
    QPen pathPen;
    pathPen.setColor(Qt::darkGray);
    pathPen.setWidth(0);
    QPen targetPen;
    targetPen.setColor(Qt::blue);
    double dotWidth = 0.5;
    double imgScale = 0.05;
    int imageWidth = 80;

    ui->graphicsView->scene()->clear();

    if (items->size() == 0)
    {
        return;
    }

    double avgLat = 0;
    for (int i = 0; i < items->size(); ++i)
    {
        avgLat += items->at(i)->getLatLon().lat;
    }
    avgLat /= (double)items->size();

    // Draw path
    QPointF prev;
    for (int i = 0; i < items->size(); ++i)
    {
        QPointF imagePoint = items->at(i)->getLatLon().convertToXY(avgLat);
        imagePoint = imagePoint * SCALE_FACTOR;

        if (i != 0)
        {
            ui->graphicsView->scene()->addLine(prev.x(), -prev.y(), imagePoint.x(), -imagePoint.y(),
                                               pathPen);
        }

        prev = imagePoint;
    }

    // Draw dots
    for (int i = 0; i < items->size(); ++i)
    {
        QPointF imagePoint = items->at(i)->getLatLon().convertToXY(avgLat);
        imagePoint = imagePoint * SCALE_FACTOR;

        QGraphicsEllipseItem *item = ui->graphicsView->scene()->addEllipse(
                    imagePoint.x() - dotWidth/2, -imagePoint.y() - dotWidth/2,
                    dotWidth, dotWidth, QPen(Qt::transparent), QBrush(imagePen.color()));

        item->setToolTip(items->at(i)->getTitle());

        // Draw corresponding targets
        QList<TargetData>& targetData = items->at(i)->getTargetData();
        for (int j = 0; j < targetData.size(); ++j)
        {
            QPointF targetPoint = targetData[j].latlon.convertToXY(avgLat);
            targetPoint = targetPoint * SCALE_FACTOR;

            QString folderPath = items->at(i)->getFolderPath();
            QString imagePath = targetData[j].imagePath;
            QString totalPath = folderPath + "/" + imagePath;

            QImage image(totalPath);
            if (image.isNull()) continue;
            QPixmap pixmap = QPixmap::fromImage(image);
            pixmap = pixmap.scaled(imageWidth, imageWidth, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation) ;
            QGraphicsPixmapItem* imageItem = new QGraphicsPixmapItem(pixmap);
            imageItem->setPos(targetPoint.x() - imgScale * 50 / 2, -targetPoint.y() - imgScale * 50 / 2);
            imageItem->setScale(imgScale);
            ui->graphicsView->scene()->addItem(imageItem);

//            QGraphicsEllipseItem *dotItem = ui->graphicsView->scene()->addEllipse(
//                        targetPoint.x() - dotWidth/2, -targetPoint.y() - dotWidth/2,
//                        dotWidth, dotWidth, targetPen, QBrush());

            imageItem->setToolTip(targetData[j].desc + "\nTaken by: " + items->at(i)->getTitle());
        }
    }

    //ui->graphicsView->fitInView(ui->graphicsView->scene()->itemsBoundingRect(), Qt::KeepAspectRatio);
}
