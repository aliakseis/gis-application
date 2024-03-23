#pragma once

#include <QWidget>
#include <QList>

#include "giscoordinatesconvertersimple.h"
#include "gisfilereaderconvertdecorator.h"
#include "gisfilereaders.h"

namespace Ui {
class MainWidget;
}

class QGraphicsScene;
class QGraphicsPolygonItem;
class QGraphicsLineItem;
class QGraphicsRectItem;
class QGraphicsEllipseItem;

class MainWidget : public QWidget {

    enum Mode { ModeTrajectorySelecting, ModeMapClipping };

    Q_OBJECT

   public:
    explicit MainWidget(QWidget *parent = nullptr);
    virtual ~MainWidget() override;
    virtual bool eventFilter(QObject *target, QEvent *event) override;
    bool processEventScrollBarWheel(QEvent *event);
    bool processEventViewportMouse(QEvent *event);

   private slots:
    void on_pushOpenMap_clicked();
    void on_lineGeoCenterLong_editingFinished();
    void on_lineGeoCenterLat_editingFinished();
    void on_radioTrajectory_clicked();
    void on_radioClipping_clicked();
    void on_pushRestoreMap_clicked();

   private:
    void windowToCenter();
    void drawMap();
    void clearMap();
    void clearClippingItems();
    void clearTrajectoryItems();
    void fitViewUnderCurrentMap();
    void drawCurrentMapBoundingRect();
    void readMap(const QString &filename);
    void calculateDiameterPrimitives(double sizeFactor = 0.005);
    void addClippingBeginPoint(const QPoint &point);
    void addClippingEndPoint(const QPoint &point);
    void clipMap();
    void addTrajectoryPoint(const QPoint &point);
    void addTrajectoryPointBegin(const QPointF &point);
    void addTrajectoryPointEnd(const QPointF &point);
    void updateTrajectoryDataGui();
    void updateConverter(double mapCenterLongitude, double mapCenterLatitude);
    void updateConverter();
    void clearClippingRectangleLines();
    void showCursorCoordinates(const QPoint &point);
    void redrawMapAfterChangeCenter();

    Ui::MainWidget *ui;
    GisFileReaderConvertDecorator *readerConvertDecorator_;
    double diameterPrimitives_;
    GAPoint clippingPointBegin_;
    GAPoint clippingPointEnd_;
    QRectF clippingRect_;
    GAPoint trajectoryPointBegin_;
    GAPoint trajectoryPointEnd_;
    QGraphicsScene *scene_;
    QList<QGraphicsPolygonItem *> mapItems_;
    QGraphicsRectItem *clippingRectItem_;
    QGraphicsEllipseItem *trajectoryBeginItem_;
    QGraphicsEllipseItem *trajectoryEndItem_;
    QGraphicsLineItem *trajectoryLineItem_;
    Mode mode_;
};
