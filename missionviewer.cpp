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
            QString imagePath = targetData[j].imagePath;
            QString totalPath = folderPath + "/" + imagePath;

            QImage image(totalPath);
            if (image.isNull()) continue;
            QPixmap pixmap = QPixmap::fromImage(image);
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
