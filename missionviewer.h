#ifndef MISSIONVIEWER_H
#define MISSIONVIEWER_H

#include <QDialog>
#include <QGraphicsScene>
#include <QMutex>
#include <QtNetwork/QNetworkAccessManager>

#include "imagewidget.h"

class CustomView;

namespace Ui {
class MissionViewer;
}


class MissionViewer : public QDialog
{
    Q_OBJECT

public:
    explicit MissionViewer(QList<ImageWidget *> *items, QWidget *parent = 0);
    ~MissionViewer();

    void closeEvent(QCloseEvent *event);
    void show();
    void refresh();

    LatLon scenePointToLatLon(QPointF scenePoint);

signals:
    void requestUpdate();

private slots:
    void moveToTarget(int row, int column);
    void on_actionrefresh_triggered();
    void mouseMoved(QPointF scenePoint);
    void networkManagerFinished(QNetworkReply *reply);

private:

    // animation parameters
    QMutex animationLock;
    bool animationOngoing = false;
    int refreshInterval = 15, animationDuration = 1000;  // in ms
    float initialScale = 3.35;  // ref val from transform().m11()
    float targetZoomFactor = 6.0;

    int iconLength, tableWidth, rowHeight; // based on user desktop size
    Ui::MissionViewer *ui;

    QRectF viewRect;
    QPointF initialCenter;
    double avgLat, avgLon;
    QList<ImageWidget *> *items;
    QList<TargetData> uniqueTargets;

    QNetworkAccessManager *networkManager;
    void download(const QString &urlStr);

    void drawPath();
    void drawTargets();
    void fillTargetTable();
    void getAvgLatLon();
    QPixmap pixmapFromTarget(QString& folderPath, TargetData& target);
    void removeDuplicates(QList<TargetData>& original);

    // run in a separate thread
    void animateMovement(QPointF start, QPointF end);
};

#endif // MISSIONVIEWER_H
