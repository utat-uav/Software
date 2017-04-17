#ifndef CUSTOMVIEW_H
#define CUSTOMVIEW_H

#include <QGraphicsView>

class MissionView : public QGraphicsView
{
public:
    MissionView(QWidget *parent);
    ~MissionView();

private:
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void wheelEvent(QWheelEvent *event);

    void zoomIn();
    void zoomOut();
    void zoom(float scaleFactor);
    void pan(QPointF delta);
    void setMaxSize();

    QPoint _lastMousePos;
    double zoomDelta;
    double panSpeed;
    bool _doMousePanning;
    int _numScheduledScalings;
    double _scale;

    int panButton;
};

#endif // CUSTOMVIEW_H
