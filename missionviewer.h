#ifndef MISSIONVIEWER_H
#define MISSIONVIEWER_H

#include <QDialog>
#include <QGraphicsScene>
#include <QtNetwork/QNetworkAccessManager>

#include "imagewidget.h"

class MissionView;

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
    Ui::MissionViewer *ui;

    double avgLat, avgLon;
    QList<ImageWidget *> *items;

    QNetworkAccessManager *networkManager;
    void download(const QString &urlStr);
};

#endif // MISSIONVIEWER_H
