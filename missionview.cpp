#include "missionview.h"

#include <QMouseEvent>
#include <QScrollBar>
#include <QDebug>

#define VIEW_CENTER viewport()->rect().center()
#define VIEW_WIDTH viewport()->rect().width()
#define VIEW_HEIGHT viewport()->rect().height()

MissionView::MissionView(QWidget *parent)
    : QGraphicsView(parent)
{
    QGraphicsScene *scene = new QGraphicsScene(0,0,1000,1000);
    this->setScene(scene);

    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setDragMode(DragMode::ScrollHandDrag);
    setMaxSize();
    centerOn(0, 0);

    zoomDelta = 0.1;
    panSpeed = 1.0;
    _doMousePanning = false;
    _scale = 1.0;

    panButton = Qt::LeftButton;
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}

MissionView::~MissionView()
{

}

void MissionView::setMaxSize()
{
    setSceneRect(-1e10, -1e10, 2e10, 2e10);
}

void MissionView::wheelEvent(QWheelEvent *event)
{
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    QPoint scrollAmount = event->angleDelta();

    // Apply zoom.
    scrollAmount.y() > 0 ? zoomIn() : zoomOut();
}

void MissionView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == panButton)
    {
        _lastMousePos = event->pos();
        _doMousePanning = true;
    }

    QGraphicsView::mousePressEvent(event);
}

void MissionView::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == panButton)
    {
        _doMousePanning = false;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void MissionView::mouseMoveEvent(QMouseEvent *event)
{
    if (_doMousePanning)
    {
        QPointF mouseDelta = mapToScene(event->pos()) - mapToScene(_lastMousePos);
        //pan(mouseDelta);
    }

    QGraphicsView::mouseMoveEvent(event);
    _lastMousePos = event->pos();
}

void MissionView::zoomIn()
{
    zoom(1 + zoomDelta);
}

void MissionView::zoomOut()
{
    zoom(1 - zoomDelta);
}

void MissionView::zoom(float scaleFactor)
{
    scale(scaleFactor, scaleFactor);
    _scale *= scaleFactor;
}


void MissionView::pan(QPointF delta)
{
    // Scale the pan amount by the current zoom.
    delta *= _scale;
    delta *= panSpeed;

    qDebug() << delta;

    // Have panning be anchored from the mouse.
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    QPoint newCenter(VIEW_WIDTH / 2 - delta.x(),  VIEW_HEIGHT / 2 - delta.y());
    centerOn(mapToScene(newCenter));

    // For zooming to anchor from the view center.
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
}
