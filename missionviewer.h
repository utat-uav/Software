#ifndef MISSIONVIEWER_H
#define MISSIONVIEWER_H

#include <QDialog>
#include <QGraphicsScene>
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

private slots:
    void on_actionrefresh_triggered();
    void mouseMoved(QPointF scenePoint);
    void networkManagerFinished(QNetworkReply *reply);

private:
    int iconLength, tableWidth, rowHeight; // resize based on user desktop size
    Ui::MissionViewer *ui;

    QRectF viewRect;
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
};

#endif // MISSIONVIEWER_H
