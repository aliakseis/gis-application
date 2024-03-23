#include "mainwidget.h"

#include "ui_mainwidget.h"

#include "gavector.h"

#include <QDebug>
#include <QDoubleValidator>
#include <QScrollBar>
#include <QWheelEvent>
#include <QFileDialog>
#include <QStyle>
#include <QScreen>

static const int converterDefaultZoneNum_ = 35;
static const bool converterDefaultIsNorth_ = true;
static constexpr double mapCenterDefaultLongitude = 27;
static constexpr double mapCenterDefaultLatitude = 51;

MainWidget::MainWidget(QWidget *parent)
    : QWidget(parent),
      ui(new Ui::MainWidget),
      readerConvertDecorator_(new GisFileReaderConvertDecorator),
      diameterPrimitives_(0),
      scene_(new QGraphicsScene(this)),
      clippingRectItem_(nullptr),
      trajectoryBeginItem_(nullptr),
      trajectoryEndItem_(nullptr),
      trajectoryLineItem_(nullptr),
      mode_(ModeTrajectorySelecting) {
    ui->setupUi(this);
    windowToCenter();

    ui->radioTrajectory->setChecked(true);
    ui->radioClipping->setChecked(false);

    ui->lineGeoCenterLong->setValidator(new QDoubleValidator(-180, 180, 5));
    ui->lineGeoCenterLat->setValidator(new QDoubleValidator(-90, 90, 5));

    ui->lineGeoCenterLong->setText(QString::number(mapCenterDefaultLongitude, 'f', 5));
    ui->lineGeoCenterLat->setText(QString::number(mapCenterDefaultLatitude, 'f', 5));

    updateConverter(mapCenterDefaultLongitude, mapCenterDefaultLatitude);

    ui->graphicsView->setScene(scene_);
    ui->graphicsView->setSceneRect(scene_->sceneRect());

    ui->graphicsView->verticalScrollBar()->installEventFilter(this);
    ui->graphicsView->viewport()->installEventFilter(this);
    ui->graphicsView->installEventFilter(this);

    ui->graphicsView->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                                     QPainter::TextAntialiasing);

    ui->graphicsView->setTransform(QTransform(1, 0, 0, 0, -1, 0, 0, 0, 1));
}

MainWidget::~MainWidget() { delete ui; }

bool MainWidget::eventFilter(QObject *target, QEvent *event) {
    if (target == ui->graphicsView->verticalScrollBar() && event->type() == QEvent::Wheel) {
        return processEventScrollBarWheel(event);
    }
    if (target == ui->graphicsView->viewport()) {
        return processEventViewportMouse(event);
    }

    return QWidget::eventFilter(target, event);
}

bool MainWidget::processEventScrollBarWheel(QEvent *event) {
    auto *wheelEvent = static_cast<QWheelEvent *>(event);
    double scale = wheelEvent->delta() > 0 ? 1.1 : 0.9;
    ui->graphicsView->scale(scale, scale);

    return true;
}

bool MainWidget::processEventViewportMouse(QEvent *event) {
    static int lastMousePressPosX;
    static int lastMousePressPosY;
    static ulong mousePressTimestamp;
    static const ulong thresholdTime = 3000;
    static bool isMousePressed = false;

    int deltaX = 0;
    int deltaY = 0;

    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease ||
        event->type() == QEvent::MouseMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            switch (mouseEvent->type()) {
                case QEvent::MouseButtonPress:
                    isMousePressed = true;

                    lastMousePressPosX = mouseEvent->x();
                    lastMousePressPosY = mouseEvent->y();
                    mousePressTimestamp = mouseEvent->timestamp();
                    break;
                case QEvent::MouseButtonRelease:
                    isMousePressed = false;

                    if (mouseEvent->timestamp() - mousePressTimestamp < thresholdTime) {
                        switch (mode_) {
                            case ModeTrajectorySelecting:
                                addTrajectoryPoint(mouseEvent->pos());
                                break;
                            case ModeMapClipping:
                                if (clippingRectItem_) {
                                    clipMap();
                                } else {
                                    addClippingBeginPoint(mouseEvent->pos());
                                }
                                break;
                        }
                    }
                    break;
                case QEvent::MouseMove:
                    showCursorCoordinates(mouseEvent->pos());
                    if (isMousePressed) {
                        deltaX = static_cast<int>(mouseEvent->x() - lastMousePressPosX);
                        deltaY = static_cast<int>(mouseEvent->y() - lastMousePressPosY);

                        ui->graphicsView->horizontalScrollBar()->setValue(
                            ui->graphicsView->horizontalScrollBar()->value() - deltaX);
                        ui->graphicsView->verticalScrollBar()->setValue(
                            ui->graphicsView->verticalScrollBar()->value() - deltaY);

                        lastMousePressPosX = mouseEvent->x();
                        lastMousePressPosY = mouseEvent->y();
                    } else if (mode_ == ModeMapClipping && clippingRectItem_) {
                        addClippingEndPoint(mouseEvent->pos());
                    }

                    break;
                default:
                    break;
            }
        }
    }

    return false;
}

void MainWidget::on_pushOpenMap_clicked() {
    QString filename =
        QFileDialog::getOpenFileName(this, "Open map", QString(), "Map files *.shp *.tab");

    if (filename.isEmpty()) {
        qDebug() << "Filename is empty";
        return;
    }

    clearClippingRectangleLines();
    clearMap();
    readMap(filename);
    calculateDiameterPrimitives();
    drawMap();
    fitViewUnderCurrentMap();
}

void MainWidget::windowToCenter() {
    QList<QScreen *> screens = qApp->screens();
    if (screens.count() > 0) {
        setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(),
                                        screens.first()->availableGeometry()));
    }
}

void MainWidget::drawMap() {
    QPen pen(QColor(0x635c44), 2);
    pen.setCosmetic(true);

    for (const GisEntity &entity : readerConvertDecorator_->entities()) {
        QPolygonF poly;
        for (const GAPoint &point : entity.points()) {
            poly.push_back(QPointF(point.x(), point.y()));
        }
        mapItems_.push_back(scene_->addPolygon(poly, pen, QBrush(QRgb(0xb5a87c))));
    }
}

void MainWidget::clearMap() {
    for (QGraphicsPolygonItem *mapItem : mapItems_) {
        delete mapItem;
    }
    mapItems_.clear();

    clearClippingItems();
    clearTrajectoryItems();
}

void MainWidget::clearClippingItems() {
    delete clippingRectItem_;
    clippingRectItem_ = nullptr;
}

void MainWidget::clearTrajectoryItems() {
    delete trajectoryBeginItem_;
    trajectoryBeginItem_ = nullptr;

    delete trajectoryEndItem_;
    trajectoryEndItem_ = nullptr;

    delete trajectoryLineItem_;
    trajectoryLineItem_ = nullptr;
}

void MainWidget::fitViewUnderCurrentMap() {
    QRectF mapRect(QPointF(readerConvertDecorator_->minX(), readerConvertDecorator_->minY()),
                   QPointF(readerConvertDecorator_->maxX(), readerConvertDecorator_->maxY()));

    ui->graphicsView->fitInView(mapRect, Qt::KeepAspectRatio);

    // drawCurrentMapBoundingRect();

    qDebug() << "Map min X:" << QString::number(readerConvertDecorator_->minX(), 'f', 2)
             << "Map max X:" << QString::number(readerConvertDecorator_->maxX(), 'f', 2)
             << "Map min Y:" << QString::number(readerConvertDecorator_->minY(), 'f', 2)
             << "Map max Y:" << QString::number(readerConvertDecorator_->maxY(), 'f', 2);
}

void MainWidget::drawCurrentMapBoundingRect() {
    QPen pen(Qt::red);
    pen.setWidth(1);
    pen.setCosmetic(true);
    QBrush brush(Qt::red);
    QRectF rect(QPointF(readerConvertDecorator_->minX(), readerConvertDecorator_->minY()),
                QPointF(readerConvertDecorator_->maxX(), readerConvertDecorator_->maxY()));
    scene_->addRect(rect, pen);
}

void MainWidget::readMap(const QString &filename) {
    GisFileReader *fileReader = gisCreateGisFileReader(filename.toStdString());

    if (!fileReader) {
        qDebug() << "Error create GisFileReader object";
        return;
    }

    readerConvertDecorator_->setGisFileReader(fileReader);
    readerConvertDecorator_->readFile();

    for (auto &entity : readerConvertDecorator_->entities()) {
        qDebug() << QString::fromStdString(entity.fieldsToString()) << entity.points().size();
    }
}

void MainWidget::calculateDiameterPrimitives(double sizeFactor) {
    double mapWidth = std::abs(readerConvertDecorator_->maxX() - readerConvertDecorator_->minX());
    double mapHeight = std::abs(readerConvertDecorator_->maxY() - readerConvertDecorator_->minY());
    diameterPrimitives_ = (mapWidth + mapHeight) * sizeFactor;
}

void MainWidget::addClippingBeginPoint(const QPoint &point) {
    clippingRect_.setTopLeft(ui->graphicsView->mapToScene(point));
    addClippingEndPoint(point);
}

void MainWidget::addClippingEndPoint(const QPoint &point) {
    clippingRect_.setBottomRight(ui->graphicsView->mapToScene(point));
    clearClippingItems();

    QBrush brushPen(QColor(0x0182b8));
    QPen pen(brushPen, 2);
    pen.setCosmetic(true);

    QColor colorBrush(0x0182b8);
    colorBrush.setAlpha(50);
    QBrush brush(colorBrush);

    clippingRectItem_ = scene_->addRect(clippingRect_, pen, brush);
}

void MainWidget::clipMap() {
    ui->lineClipTopLeftX->setText(QString::number(clippingRect_.topLeft().x(), 'f', 5));
    ui->lineClipTopLeftY->setText(QString::number(clippingRect_.topLeft().y(), 'f', 5));
    ui->lineClipBottomRightX->setText(QString::number(clippingRect_.bottomRight().x(), 'f', 5));
    ui->lineClipBottomRightY->setText(QString::number(clippingRect_.bottomRight().y(), 'f', 5));

    readerConvertDecorator_->clipPolygons(clippingRect_.x(), clippingRect_.y(),
                                          clippingRect_.right(), clippingRect_.bottom());
    clearMap();
    drawMap();
}

void MainWidget::addTrajectoryPoint(const QPoint &point) {
    QPointF pointMapped = ui->graphicsView->mapToScene(point);

    if (!trajectoryBeginItem_ || trajectoryLineItem_) {
        clearTrajectoryItems();
        addTrajectoryPointBegin(pointMapped);
    } else {
        addTrajectoryPointEnd(pointMapped);
    }

    updateTrajectoryDataGui();
}

void MainWidget::addTrajectoryPointBegin(const QPointF &point) {
    trajectoryPointBegin_ = GAPoint(point.x(), point.y());

    const double diameter = diameterPrimitives_;
    trajectoryBeginItem_ = scene_->addEllipse(trajectoryPointBegin_.x() - diameter / 2,
                                              trajectoryPointBegin_.y() - diameter / 2, diameter,
                                              diameter, QPen(), QBrush(0x80b3f2));
}

void MainWidget::addTrajectoryPointEnd(const QPointF &point) {
    trajectoryPointEnd_ = GAPoint(point.x(), point.y());

    QPen pen(QColor(0x80b3f2), 2);
    pen.setStyle(Qt::DotLine);
    pen.setCosmetic(true);

    const double diameter = diameterPrimitives_ / 2.2;
    trajectoryEndItem_ = scene_->addEllipse(trajectoryPointEnd_.x() - diameter / 2,
                                            trajectoryPointEnd_.y() - diameter / 2, diameter,
                                            diameter, QPen(), QBrush(0x80b3f2));

    QLineF trajectorLineConverted(trajectoryPointBegin_.x(), trajectoryPointBegin_.y(),
                                  trajectoryPointEnd_.x(), trajectoryPointEnd_.y());
    trajectoryLineItem_ = scene_->addLine(trajectorLineConverted, pen);
}

void MainWidget::updateTrajectoryDataGui() {
    ui->lineProjX->setText(QString::number(trajectoryPointBegin_.x(), 'f', 5));
    ui->lineProjY->setText(QString::number(trajectoryPointBegin_.y(), 'f', 5));

    GAPoint trajectoryPointBeginGeo =
        readerConvertDecorator_->coordinatesConverter()->transformCoordinateBack(
            trajectoryPointBegin_);

    ui->lineGeoLong->setText(QString::number(trajectoryPointBeginGeo.x(), 'f', 5));
    ui->lineGeoLat->setText(QString::number(trajectoryPointBeginGeo.y(), 'f', 5));

    ui->lineDistance->setText(
        QString::number(trajectoryPointBegin_.distance(trajectoryPointEnd_), 'f', 5));
    ui->lineHeadingAngle->setText(
        QString::number(GAVector(trajectoryPointBegin_, trajectoryPointEnd_).angleHeading()));
}

void MainWidget::updateConverter(double mapCenterLongitude, double mapCenterLatitude) {
    readerConvertDecorator_->setCoordinatesConverter(
        new GisCoordinatesConverterSimple(mapCenterLongitude, mapCenterLatitude));
    readerConvertDecorator_->readFile();
}

void MainWidget::updateConverter() {
    double mapCenterLongitude = ui->lineGeoCenterLong->text().toDouble();
    double mapCenterLatitude = ui->lineGeoCenterLat->text().toDouble();

    updateConverter(mapCenterLongitude, mapCenterLatitude);
}

void MainWidget::clearClippingRectangleLines() {
    ui->lineClipTopLeftX->clear();
    ui->lineClipTopLeftY->clear();
    ui->lineClipBottomRightX->clear();
    ui->lineClipBottomRightY->clear();
}

void MainWidget::showCursorCoordinates(const QPoint &point) {
    QPointF pointMapped = ui->graphicsView->mapToScene(point);
    ui->lineX->setText(QString::number(pointMapped.x(), 'f', 5));
    ui->lineY->setText(QString::number(pointMapped.y(), 'f', 5));
}

void MainWidget::redrawMapAfterChangeCenter() {
    updateConverter();
    clearMap();
    drawMap();
}
void MainWidget::on_lineGeoCenterLong_editingFinished() { redrawMapAfterChangeCenter(); }

void MainWidget::on_lineGeoCenterLat_editingFinished() { redrawMapAfterChangeCenter(); }

void MainWidget::on_radioTrajectory_clicked() {
    mode_ = ModeTrajectorySelecting;
    clearClippingItems();
}

void MainWidget::on_radioClipping_clicked() {
    mode_ = ModeMapClipping;
    clearTrajectoryItems();
}

void MainWidget::on_pushRestoreMap_clicked() {
    readerConvertDecorator_->restorePolygons();
    clearClippingRectangleLines();
    clearMap();
    drawMap();
}
