#include "missionviewer.h"
#include "ui_missionviewer.h"

#include <QCloseEvent>
#include <QDebug>
#include <QToolBar>
#include <QGraphicsEllipseItem>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include "imagewidget.h"
#include "customview.h"
#include <QSet>
#include <QTableWidget>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <iostream>
#include <string>

#define SCALE_FACTOR 50000.0

//https://maps.googleapis.com/maps/api/staticmap?maptype=satellite&center=43.83461,-79.2413&zoom=17&size=1280x1280&scale=2

MissionViewer::MissionViewer(QList<ImageWidget *> *items, QWidget *parent) :
    QDialog(parent),
    items(items),
    avgLat(0),
    avgLon(0),
    ui(new Ui::MissionViewer)
{
    ui->setupUi(this);

//    QToolBar *menubar = new QToolBar();
//    this->layout()->setMenuBar(menubar);
//    menubar->addAction(ui->actionrefresh);

    // desktop-dependent sizing
    QRect rec = QApplication::desktop()->screenGeometry();
    tableWidth = rec.width() * 0.35;
    rowHeight = rec.height() * 0.25;

    // not too small or too large
    tableWidth = std::max(tableWidth, 300);
    tableWidth = std::min(tableWidth, 1000);
    rowHeight = std::max(rowHeight, 200);
    rowHeight = std::min(rowHeight, 400);
    iconLength = rowHeight;

    connect(ui->graphicsView, &CustomView::mouseMoved, this, &MissionViewer::mouseMoved);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkManagerFinished(QNetworkReply*)));
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
    if (viewRect.width() * viewRect.height() > 0)
    {
        ui->graphicsView->fitInView(viewRect, Qt::KeepAspectRatio);
        QPalette palette = ui->graphicsView->palette();
        palette.setColor(ui->graphicsView->backgroundRole(), QColor(230, 230, 230));
        ui->graphicsView->setPalette(palette);
    }
}

void MissionViewer::refresh()
{
    on_actionrefresh_triggered();
}

void MissionViewer::mouseMoved(QPointF scenePoint)
{
    LatLon latlon = scenePointToLatLon(scenePoint);

    this->setWindowTitle(QString("Mission Viewer - (%1, %2)").arg(latlon.lat).arg(latlon.lon));
}

LatLon MissionViewer::scenePointToLatLon(QPointF scenePoint)
{
    scenePoint = scenePoint / SCALE_FACTOR;
    scenePoint.setY(-scenePoint.y());

    return LatLon::xyToLatLon(scenePoint, avgLat);
}

void MissionViewer::getAvgLatLon()
{
    // Get avg latlon
    avgLat = 0;
    avgLon = 0;
    for (int i = 0; i < items->size(); ++i)
    {
        avgLat += items->at(i)->getLatLon().lat;
        avgLon += items->at(i)->getLatLon().lon;
    }
    avgLat /= (double)items->size();
    avgLon /= (double)items->size();
}

void MissionViewer::drawPath()
{
    QPen imagePen;
    imagePen.setColor(Qt::red);
    QPen pathPen;
    pathPen.setColor(Qt::darkGray);
    pathPen.setWidth(0);
    double dotWidth = 0.5;

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
    }
}

void MissionViewer::drawTargets()
{
    QPen targetPen;
    targetPen.setColor(Qt::blue);
    double imgScale = 0.05;
    int imageWidth = 80;

    // Draw targets
    for (int i = 0; i < items->size(); ++i)
    {
        QList<TargetData>& targetData = items->at(i)->getTargetData();
        for (int j = 0; j < targetData.size(); ++j)
        {
            QPointF targetPoint = targetData[j].latlon.convertToXY(avgLat);
            targetPoint = targetPoint * SCALE_FACTOR;

            QString folderPath = items->at(i)->getFolderPath();
            QPixmap pixmap = pixmapFromTarget(folderPath, targetData[j]);
            pixmap = pixmap.scaled(imageWidth, imageWidth, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
            QGraphicsPixmapItem* imageItem = new QGraphicsPixmapItem(pixmap);
            imageItem->setPos(targetPoint.x() - imgScale * imageWidth / 2,
                              -targetPoint.y() - imgScale * imageWidth / 2);
            imageItem->setScale(imgScale);
            ui->graphicsView->scene()->addItem(imageItem);

            imageItem->setToolTip(targetData[j].desc + "\nTaken by: " + items->at(i)->getTitle());
        }
    }
}


void MissionViewer::removeDuplicates(QList<TargetData>& original)
{
    // criteria: unique target identified by <letter, shape, shape color>
    QSet<QString> seenTargets;

    for (int i = 0; i < original.size(); ++i)
    {
        if (seenTargets.contains(original[i].idStr()))
        {
            original.removeAt(i);
            --i;
        }
        else
        {
            seenTargets.insert(original[i].idStr());
        }
    }
}


void MissionViewer::fillTargetTable()
{
    // set table rows based on number of unique items
    int numTargets = uniqueTargets.size();
    ui->tableWidget->setRowCount(numTargets);

    // set header names
    QTableWidgetItem* header1 = new QTableWidgetItem("TARGET");
    QTableWidgetItem* header2 = new QTableWidgetItem("TARGET INFO");
    ui->tableWidget->setHorizontalHeaderItem(0, header1);
    ui->tableWidget->setHorizontalHeaderItem(1, header2);
    ui->tableWidget->verticalHeader()->setVisible(false);

    int rowNum = 0;
    for (int i = 0; i < uniqueTargets.size(); ++i)
    {
        // set subtable of classification info
        QTableWidget* classificationTable = new QTableWidget(6, 1);
        //classificationTable->setAttribute(Qt::WA_NoMousePropagation);
        QString latlonStr = QString::number(uniqueTargets[i].latlon.lat) + " , " + QString::number(uniqueTargets[i].latlon.lat);
        QTableWidgetItem *latlonDesc = new QTableWidgetItem(latlonStr);
        QTableWidgetItem *charDesc = new QTableWidgetItem(uniqueTargets[i].alphanumeric);
        QTableWidgetItem *shapeDesc = new QTableWidgetItem(uniqueTargets[i].shape);
        QTableWidgetItem *charColorDesc = new QTableWidgetItem(uniqueTargets[i].alphanumericColor);
        QTableWidgetItem *shapeColorDesc = new QTableWidgetItem(uniqueTargets[i].shapeColor);
        QTableWidgetItem *orientationDesc = new QTableWidgetItem(uniqueTargets[i].orientation);
        QTableWidgetItem* subheader1 = new QTableWidgetItem("Lat, Lon");
        QTableWidgetItem* subheader2 = new QTableWidgetItem("Char");
        QTableWidgetItem* subheader3 = new QTableWidgetItem("Shape");
        QTableWidgetItem* subheader4 = new QTableWidgetItem("Char Color");
        QTableWidgetItem* subheader5 = new QTableWidgetItem("Shape Color");
        QTableWidgetItem* subheader6 = new QTableWidgetItem("Orientation");
        classificationTable->setVerticalHeaderItem(0, subheader1);
        classificationTable->setVerticalHeaderItem(1, subheader2);
        classificationTable->setVerticalHeaderItem(2, subheader3);
        classificationTable->setVerticalHeaderItem(3, subheader4);
        classificationTable->setVerticalHeaderItem(4, subheader5);
        classificationTable->setVerticalHeaderItem(5, subheader6);
        classificationTable->horizontalHeader()->setVisible(false);
        classificationTable->setItem(0, 0, latlonDesc);
        classificationTable->setItem(1, 0, charDesc);
        classificationTable->setItem(2, 0, shapeDesc);
        classificationTable->setItem(3, 0, charColorDesc);
        classificationTable->setItem(4, 0, shapeColorDesc);
        classificationTable->setItem(5, 0, orientationDesc);
        classificationTable->horizontalHeader()->setStretchLastSection(true);

        // get total path of roi, set icon
        QLabel *roi = new QLabel;
        QString folderPath = items->at(i)->getFolderPath();
        QPixmap pixmap = pixmapFromTarget(folderPath, uniqueTargets[i]);
        pixmap = pixmap.scaled(iconLength, iconLength);
        roi->setPixmap(pixmap);
        roi->setAlignment(Qt::AlignHCenter);
        //roi->setStyleSheet("QLabel { background-color : silver; selection-background-color : black; }");

        ui->tableWidget->setCellWidget(rowNum, 0, roi);
        ui->tableWidget->setCellWidget(rowNum, 1, classificationTable);
        ++rowNum;
    }
}

void MissionViewer::on_actionrefresh_triggered()
{
    qDebug() << "Refreshing mission viewer";

    ui->graphicsView->scene()->clear();

    if (items->size() == 0)
    {
        return;
    }

    getAvgLatLon();
    drawPath();
    drawTargets();

    viewRect = ui->graphicsView->scene()->itemsBoundingRect();
    ui->graphicsView->fitInView(viewRect, Qt::KeepAspectRatio);

    QString imagePath = "https://maps.googleapis.com/maps/api/staticmap?maptype=satellite&center=" + QString::number(avgLat, 'f', 10) +
            "," + QString::number(avgLon, 'f', 10) + "&zoom=16&size=1280x1280&scale=2";
    download(imagePath);

    // get unique targets
    for (int i = 0; i < items->size(); ++i)
    {
        QList<TargetData>& targetData = items->at(i)->getTargetData();
        for (int j = 0; j < targetData.size(); ++j)
        {
            uniqueTargets.append(targetData[j]);
        }
    }
    removeDuplicates(uniqueTargets);

    // fill table with targets and resize tableWidget properly
    fillTargetTable();
    ui->tableWidget->setFixedWidth(tableWidth);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    QHeaderView *verticalHeader = ui->tableWidget->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(rowHeight);
    ui->tableWidget->setIconSize(QSize(iconLength, iconLength));
}

void MissionViewer::download(const QString &urlStr)
{
    QUrl url(urlStr);
    QNetworkRequest request(url);
    networkManager->get(request);
}

void MissionViewer::networkManagerFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error in" << reply->url() << ":" << reply->errorString();
        return;
    }
    QVariant attribute = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (attribute.isValid())
    {
        QUrl url = attribute.toUrl();
        qDebug() << "must go to:" << url;
        return;
    }
    qDebug() << "ContentType:" << reply->header(QNetworkRequest::ContentTypeHeader).toString();
    QByteArray jpegData = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData(jpegData);

    LatLon avgLL;
    avgLL.lat = avgLat;
    avgLL.lon = avgLon;
    QPointF xy = avgLL.convertToXY(avgLat) * SCALE_FACTOR;
    int width = 1280;
    double imgScale = 0.41;

    pixmap = pixmap.scaled(width, width, Qt::KeepAspectRatioByExpanding, Qt::FastTransformation);
    QGraphicsPixmapItem* imageItem = new QGraphicsPixmapItem(pixmap);
    imageItem->setPos(xy.x() - imgScale * width / 2, -xy.y() - imgScale * width / 2);
    imageItem->setScale(imgScale);
    imageItem->setZValue(-1);
    ui->graphicsView->scene()->addItem(imageItem);
}


QPixmap MissionViewer::pixmapFromTarget(QString& folderPath, TargetData& target)
{
    QString imagePath = target.imagePath;
    QString totalPath = folderPath + "/" + imagePath;
    QImage image(totalPath);
    QPixmap pixmap = QPixmap::fromImage(image);
    return pixmap;
}
