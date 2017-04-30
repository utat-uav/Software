#include "missionviewer.h"
#include "ui_missionviewer.h"

#include <QCloseEvent>
#include <QDebug>
#include <QToolBar>
#include <QPushButton>
#include <QGraphicsEllipseItem>
#include <QtConcurrent/QtConcurrent>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QSet>
#include <QTableWidget>
#include <QHBoxLayout>
#include <QDesktopWidget>
#include <iostream>
#include <string>
#include <QNetworkCookie>
#include <QMessageBox>
#include <QNetworkCookieJar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStringList>
#include <QString>

#include "imagewidget.h"
#include "customview.h"
#include "interoplogin.h"

#define SCALE_FACTOR 50000.0
#define PI 3.141592653589793238462643383279502884L

//https://maps.googleapis.com/maps/api/staticmap?maptype=satellite&center=43.83461,-79.2413&zoom=17&size=1280x1280&scale=2

MissionViewer::MissionViewer(QList<ImageWidget *> *items, QWidget *parent) :
    QDialog(parent),
    items(items),
    avgLat(0),
    avgLon(0),
    ui(new Ui::MissionViewer)
{
    ui->setupUi(this);

    QToolBar *menubar = new QToolBar();
    menubar->setStyleSheet("QToolBar { background : white }");
    this->layout()->setMenuBar(menubar);

    loginAction = new QAction(QString("Login"), menubar);
    menubar->addAction(loginAction);
    connect(loginAction, &QAction::triggered, this, &MissionViewer::login);

    postAllTargetsAction = new QAction(QString("Upload all targets"), menubar);
    menubar->addAction(postAllTargetsAction);
    connect(postAllTargetsAction, &QAction::triggered, this, &MissionViewer::postAllTargets);
    postAllTargetsAction->setEnabled(false);

    iconLength = 180;

    connect(this, SIGNAL(requestUpdate()), this, SLOT(update()), Qt::ConnectionType::QueuedConnection);
    connect(ui->graphicsView, &CustomView::mouseMoved, this, &MissionViewer::mouseMoved);

    mapNetworkManager = new QNetworkAccessManager(this);
    connect(mapNetworkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(mapNetworkManagerFinished(QNetworkReply*)));

    ui->tableWidget->setEnabled(false);
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

void MissionViewer::moveToTarget(int row, int column)
{
    animationLock.tryLock();
    if (animationOngoing)
    {
        animationLock.unlock();
        return;
    }
    animationOngoing = true;
    animationLock.unlock();

    QPointF curCenter = ui->graphicsView->mapToScene(ui->graphicsView->viewport()->rect().center());
    QPointF targetPoint = uniqueTargets[row].latlon.convertToXY(avgLat);
    targetPoint = targetPoint * SCALE_FACTOR;
    QPointF targetCenter(targetPoint.x(), -targetPoint.y());
    float resetZoom = initialScale / ui->graphicsView->transform().m11();

    // optional and somewhat buggy
    QtConcurrent::run(this, &MissionViewer::animateMovement, curCenter, targetCenter);
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
        QString latlonStr = QString::number(uniqueTargets[i].latlon.lat, 'f', 7) + ", " +
                            QString::number(uniqueTargets[i].latlon.lon, 'f', 7);
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
        classificationTable->setFrameStyle(QFrame::NoFrame);
        classificationTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        classificationTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        for (int j = 0; j < classificationTable->rowCount(); ++j)
        {
            classificationTable->setRowHeight(j, 30);
        }

        // get total path of roi, set icon
        QPushButton *roi = new QPushButton;
        QString folderPath = items->at(i)->getFolderPath();
        QPixmap pixmap = pixmapFromTarget(folderPath, uniqueTargets[i]);
        pixmap = pixmap.scaled(iconLength, iconLength);
        roi->setIcon(QIcon(pixmap));
        roi->setIconSize(QSize(iconLength, iconLength));
        roi->setStyleSheet("QPushButton:focus { background-color : lightblue;}");

        // pushbutton will ignore event as tableWidget is more qualified to handle it
        roi->setAttribute(Qt::WA_TransparentForMouseEvents);

        ui->tableWidget->setCellWidget(rowNum, 0, roi);
        ui->tableWidget->setCellWidget(rowNum, 1, classificationTable);
        ++rowNum;
    }

    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked, this, &MissionViewer::moveToTarget);
    ui->tableWidget->setEnabled(true);
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
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->setIconSize(QSize(iconLength, iconLength));
}

void MissionViewer::download(const QString &urlStr)
{
    QUrl url(urlStr);
    QNetworkRequest request(url);
    mapNetworkManager->get(request);
}

void MissionViewer::mapNetworkManagerFinished(QNetworkReply *reply)
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
    //qDebug() << "ContentType:" << reply->header(QNetworkRequest::ContentTypeHeader).toString();
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
    delete reply;
}


QPixmap MissionViewer::pixmapFromTarget(QString& folderPath, TargetData& target)
{
    QString imagePath = target.imagePath;
    QString totalPath = folderPath + "/" + imagePath;
    QImage image(totalPath);
    QPixmap pixmap = QPixmap::fromImage(image);
    return pixmap;
}


void MissionViewer::animateMovement(QPointF start, QPointF end)
{
    double startZoom = ui->graphicsView->transform().m11() / initialScale;
    int iterations = double(animationDuration) / refreshInterval;
    double ds = 1.0 / double(iterations);
    double minZoom;

    if (startZoom > 2.0)
        minZoom = 0.6 * startZoom;
    else
        minZoom = -1.0;

    for (int i = 1; i <= iterations; ++i)
    {
        double s = ds * i;
        QPointF curCenter;

        // reset scale
        double currentScale = ui->graphicsView->transform().m11();
        double resetZoom = initialScale / currentScale;
        double curZoom;

        if (minZoom == -1.0)
        {
            curCenter = (end - start) * pow(sin(PI * s / 2.0), 0.5) + start;
            curZoom = (targetZoomFactor - startZoom) * s + startZoom;
        }
        else
        {
            // pull out a bit before going in
            curCenter = (end - start) * pow(sin(PI * s / 2.0), 2.0) + start;
            if (i <= (iterations / 2.0))
            {
                curZoom = (minZoom - startZoom) * sin(PI * s) + startZoom;
            }
            else
            {
                curZoom = 0.5 * (targetZoomFactor - minZoom) * cos(2.0 * PI * s) + (targetZoomFactor + minZoom) / 2.0;
            }
        }

        // set views
        ui->graphicsView->scale(curZoom * resetZoom, curZoom * resetZoom);
        ui->graphicsView->centerOn(curCenter);
        emit requestUpdate();
        QThread::msleep(refreshInterval);
    }
    animationOngoing = false;
}

void MissionViewer::doLogout()
{
    loginAction->setText("Login");
    serverInfo.loggedIn = false;
    postAllTargetsAction->setEnabled(false);
}

void MissionViewer::doLogin()
{
    this->loginAction->setText("Logout");
    serverInfo.loggedIn = true;
    postAllTargetsAction->setEnabled(true);
}

void MissionViewer::login()
{
    // Ensure that the text is "login" if we are not logged in
    if ((loginAction->text() == "Login") != (!serverInfo.loggedIn))
    {
        qFatal("(loginAction->text() == \"Login\") != (!serverInfo.loggedIn)");
    }

    // If already logged in, then we should prompt for logout
    if (serverInfo.loggedIn)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Logout", "Are you sure you want to log out?",
                                      QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            doLogout();
        }

        return;
    }

    // Login code
    InteropLogin interopLogin(this);
    if (interopLogin.exec())
    {
        InteropLogin::ServerInfo temp = interopLogin.getServerInfo();

        serverInfo = temp;

        QString data = ("username=" + serverInfo.username +
                        "&password=" + serverInfo.password);

        QByteArray replyData;
        if (auvsiRequest("/api/login", POST, data.toUtf8(), "", replyData))
        {
            qDebug() << "Success" << replyData;

            // Do the actual login ui changes
            if (serverInfo.loggedIn)
            {
                qFatal("serverInfo.loggedIn");
            }
            doLogin();
        }
        else
        {
            QMessageBox::warning(this, tr("Error"),
                               tr("Login failed."),
                               QMessageBox::Ok);
        }
    }
}

/*
 * Returns true if success or false if fail
 * api = "/api/login" for example
 * requestType = POST for example
 *
 * Logs out the user if an invalid request is made
 */
bool MissionViewer::auvsiRequest(const QString &api, const int requestType, const QByteArray &data,
                                 const QString &contentTypeHeader, QByteArray &replyData)
{
    if ((loginAction->text() == "Login") != (!serverInfo.loggedIn))
    {
        qFatal("(loginAction->text() == \"Login\") != (!serverInfo.loggedIn)");
    }

    QString requestAddr = QString("http://" + serverInfo.ip + ":" + QString::number(serverInfo.port)) + api;
    QUrl url(requestAddr);
    QNetworkRequest request(url);
    QNetworkAccessManager manager;

    // AUVSI uses urlencoded POST
    if (requestType == POST)
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    }
    else if (contentTypeHeader != "")
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, contentTypeHeader);
    }

    if (serverInfo.loggedIn)
    {
        QString str = "sessionid=" + serverInfo.cookie.value();
        request.setRawHeader("Cookie", str.toUtf8());
    }

    // Connect an event loop for blocking later
    QEventLoop eventLoop;
    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));

    // Do request
    QNetworkReply *reply;
    switch (requestType)
    {
    case POST:
        reply = manager.post(request, data);
        break;
    case GET:
        reply = manager.get(request);
        break;
    default:
        qDebug() << "REQUEST TYPE NOT RECOGNIZED";
        return false;
    }

    // Blocking starts here
    QTimer timer;
    timer.setSingleShot(true);
    connect(&timer, SIGNAL(timeout()), &eventLoop, SLOT(quit()));
    timer.start(2000); // 2 seconds
    eventLoop.exec();

    // If the timer is done, then we timed out
    if (!timer.isActive())
    {
        disconnect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
        delete reply;
        return false;
    }
    timer.stop();

    // Success
    if (reply->error() == QNetworkReply::NoError)
    {
        // Check if new cookie (cached login)
        if (reply->hasRawHeader("Set-Cookie"))
        {
            QVariant cookieVar = reply->header(QNetworkRequest::SetCookieHeader);
            if (cookieVar.isValid())
            {
                QList<QNetworkCookie> cookies = cookieVar.value<QList<QNetworkCookie>>();
                foreach (const QNetworkCookie &cookie, cookies)
                {
                    serverInfo.cookie = cookie;
                }
            }
        }

        replyData = reply->readAll();
        delete reply;
        return true;
    }
    // Failure
    else
    {
        if (serverInfo.loggedIn)
        {
            doLogout();
        }

        replyData = reply->errorString().toUtf8();
        qDebug() << "Error" << replyData;
        delete reply;
        return false;
    }

    // Should never get here
    return false;
}

void MissionViewer::postAllTargets()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Upload Targets",
                                  "Do you want to upload all the targets to the AUVSI interop server?",
                                  QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes)
    {
        return;
    }

    for (int i = 0; i < ui->tableWidget->rowCount(); ++i)
    {
        postTargetIdx(i);
    }
}

/*
 * Idx is the row of the table
 */
void MissionViewer::postTargetIdx(int idx)
{
    QTableWidget* classificationTable = dynamic_cast<QTableWidget*>(ui->tableWidget->cellWidget(idx, 1));

    if (classificationTable == NULL)
    {
        qDebug() << "classificationTable == NULL";
        return;
    }

    QTableWidgetItem *latlonDesc = dynamic_cast<QTableWidgetItem*>(classificationTable->item(0, 0));
    QTableWidgetItem *charDesc = dynamic_cast<QTableWidgetItem*>(classificationTable->item(1, 0));
    QTableWidgetItem *shapeDesc = dynamic_cast<QTableWidgetItem*>(classificationTable->item(2, 0));
    QTableWidgetItem *charColorDesc = dynamic_cast<QTableWidgetItem*>(classificationTable->item(3, 0));
    QTableWidgetItem *shapeColorDesc = dynamic_cast<QTableWidgetItem*>(classificationTable->item(4, 0));
    QTableWidgetItem *orientationDesc = dynamic_cast<QTableWidgetItem*>(classificationTable->item(5, 0));

    if ((latlonDesc == NULL) ||
        (charDesc == NULL) ||
        (shapeDesc == NULL) ||
        (charColorDesc == NULL) ||
        (shapeColorDesc == NULL) ||
        (orientationDesc == NULL))
    {
        qDebug() << "classificationTable items == NULL";
        return;
    }

    QString latlonText = latlonDesc->text();
    QStringList latlonList = latlonText.split(",");

    if (latlonList.size() != 2)
    {
        qDebug() << "latlonList.size() != 2";
        return;
    }

    double lat = latlonList[0].toDouble();
    double lon = latlonList[1].toDouble();

    QJsonObject targetJsonObj;
    targetJsonObj["type"] = "standard";
    targetJsonObj["latitude"] = lat;
    targetJsonObj["longitude"] = lon;
    targetJsonObj["background_color"] = shapeColorDesc->text();
    targetJsonObj["alphanumeric_color"] = charColorDesc->text();
    targetJsonObj["alphanumeric"] = charDesc->text();
    targetJsonObj["autonomous"] = true;

    QJsonDocument jsonDoc(targetJsonObj);
    QString targetJsonStr = jsonDoc.toJson();

    QByteArray replyData;
    if (auvsiRequest("/api/targets", POST, targetJsonStr.toUtf8(), "application/json", replyData))
    {
        //qDebug() << "Success" << replyData;

        QJsonObject replyObj = QJsonDocument::fromJson(replyData).object();
        int targetID = replyObj["id"].toInt();
        //qDebug() << "id is " << targetID;

        QPushButton *roiPushButton = dynamic_cast<QPushButton*>(ui->tableWidget->cellWidget(idx, 0));
        if (roiPushButton == NULL)
        {
            qFatal("roiPushButton == NULL");
        }

        QPixmap thumbPixmap = roiPushButton->icon().pixmap(QSize(iconLength, iconLength));
        QByteArray bArray;
        QBuffer buffer(&bArray);
        buffer.open(QIODevice::WriteOnly);
        thumbPixmap.save(&buffer, "JPEG");
//        thumbPixmap.save("C:/Users/Davis/Desktop/test.jpeg", "JPEG");
//        qDebug() << bArray;
//        qDebug() << bArray.size();

        if (auvsiRequest("/api/targets/" + QString::number(targetID) + "/image",
                         POST, bArray, "application/jpeg", replyData))
        {
            qDebug() << "Success" << replyData;
        }
    }
    else
    {
        qDebug() << "Failure" << replyData;
    }
}

