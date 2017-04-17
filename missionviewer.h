#ifndef MISSIONVIEWER_H
#define MISSIONVIEWER_H

#include <QDialog>
#include <QGraphicsScene>

class ImageWidget;
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

private slots:
    void on_actionrefresh_triggered();

private:
    Ui::MissionViewer *ui;

    QList<ImageWidget *> *items;
};

#endif // MISSIONVIEWER_H
