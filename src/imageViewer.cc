#include <ziwi/imageViewer.h>

#include <QLayout>

using namespace Lux::ziwi;

SynchableGraphicsView::SynchableGraphicsView(QGraphicsScene* scene,
                                             QWidget* parent)
    : QGraphicsView(scene, parent) {
    handDrag_ = false;
    clearTransformChanges();
    connectSbarSignals(&SynchableGraphicsView::scrollChanged);
}

QPointF SynchableGraphicsView::scrollState() const {
    auto centerPoint =
        mapToScene(viewport()->width() / 2, viewport()->height() / 2);

    auto sceneRect = this->sceneRect();
    auto centerWidth = centerPoint.x() - sceneRect.left();
    auto centerHeight = centerPoint.y() - sceneRect.top();
    auto sceneWidth = sceneRect.width();
    auto sceneHeight = sceneRect.height();

    auto sceneWidthPercent = sceneWidth != 0 ? centerWidth / sceneWidth : 0;
    auto sceneHeightPercent = sceneHeight != 0 ? centerHeight / sceneHeight : 0;

    return {sceneWidthPercent, sceneHeightPercent};
}

void SynchableGraphicsView::setScrollState(const QPointF& scrollState) {
    this->centerOn((scrollState.x() * this->sceneRect().width() +
                    this->sceneRect().left()),
                   (scrollState.y() * this->sceneRect().height() +
                    this->sceneRect().top()));
}

void SynchableGraphicsView::setZoomFactor(float zoomFactor) {
    zoomFactor = zoomFactor / this->zoomFactor();
    this->scale(zoomFactor, zoomFactor);
}

void SynchableGraphicsView::wheelEvent(QWheelEvent* event) {
    // std::cout << __LINE__ << __FUNCTION__ << std::endl;

    if (event->modifiers() & Qt::KeyboardModifier::ControlModifier) {
        // std::cout << event->angleDelta().x() << std::endl;
        // std::cout << event->angleDelta().y() << std::endl;
        // std::cout << event->angleDelta().y() / 240.0f << std::endl;

        emit this->wheelNotches(event->angleDelta().y() / 240.0f);
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void SynchableGraphicsView::keyReleaseEvent(QKeyEvent* event) {
    std::cout << "graphicsView keyRelease count=" << event->count()
              << ", autoRepeat=" << event->isAutoRepeat() << std::endl;
    event->ignore();
    // QGraphicsView::keyReleaseEvent(event);
}

bool SynchableGraphicsView::checkTransformChanged() {
    float delta = 0.001;
    bool result = false;

    auto different = [](float t, float u) {
        float d = 0.0;
        if (u == 0.0)
            d = std::fabs(t - u);
        else
            d = std::fabs((t - u) / u);
        return d > 0.001;
    };

    auto t = transform();
    auto u = transform_;

    if (different(t.m11(), u.m11()) || different(t.m22(), u.m22()) ||
        different(t.m12(), u.m12()) || different(t.m21(), u.m21()) ||
        different(t.m31(), u.m31()) || different(t.m32(), u.m32())) {
        this->transform_ = t;
        emit transformChanged();
        result = true;
    }
    return result;
}

void SynchableGraphicsView::scrollToTop() {
    auto sbar = verticalScrollBar();
    sbar->setValue(sbar->minimum());
}
void SynchableGraphicsView::scrollToBottom() {
    auto sbar = verticalScrollBar();
    sbar->setValue(sbar->maximum());
}
void SynchableGraphicsView::scrollToBegin() {
    auto sbar = horizontalScrollBar();
    sbar->setValue(sbar->minimum());
}
void SynchableGraphicsView::scrollToEnd() {
    auto sbar = horizontalScrollBar();
    sbar->setValue(sbar->maximum());
}
void SynchableGraphicsView::centerView() {
    auto sbar = verticalScrollBar();
    sbar->setValue((sbar->maximum() + sbar->minimum()) / 2);

    sbar = horizontalScrollBar();
    sbar->setValue((sbar->maximum() + sbar->minimum()) / 2);
}
void SynchableGraphicsView::enableScrollBars(bool enable) {
    if (enable) {
        setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
        setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAsNeeded);
    } else {
        setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
    }
}
void SynchableGraphicsView::enableHandDrag(bool enable) {
    if (enable) {
        if (!handDrag_) {
            handDrag_ = true;
            setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
        }
    } else {
        if (handDrag_) {
            handDrag_ = false;
            setDragMode(QGraphicsView::DragMode::NoDrag);
        }
    }
}

void SynchableGraphicsView::dumpTransform(const QTransform& t,
                                          const QString& padding) {
    printf("%s%5.3f %5.3f %5.3f\n", padding.toStdString().c_str(), t.m11(),
           t.m12(), t.m13());
    printf("%s%5.3f %5.3f %5.3f\n", padding.toStdString().c_str(), t.m21(),
           t.m22(), t.m23());
    printf("%s%5.3f %5.3f %5.3f\n", padding.toStdString().c_str(), t.m31(),
           t.m32(), t.m33());
}
void SynchableGraphicsView::mouseReleaseEvent(QMouseEvent* event) {
    emit mouseReleased(event);
    QGraphicsView::mouseReleaseEvent(event);
    viewport()->setCursor(Qt::CursorShape::CrossCursor);
}
void SynchableGraphicsView::mousePressEvent(QMouseEvent* event) {
    emit mousePressed(event);
    QGraphicsView::mousePressEvent(event);
    viewport()->setCursor(Qt::CursorShape::CrossCursor);
}
void SynchableGraphicsView::enterEvent(QEvent* event) {
    QGraphicsView::enterEvent(event);
    viewport()->setCursor(Qt::CursorShape::CrossCursor);
}

ImageViewer::ImageViewer(QPixmap* pixmap, QString name)
    : QFrame(),
      scene_(new QGraphicsScene()),
      view_(new SynchableGraphicsView(scene_, this)),
      pixmapItem_(nullptr),
      layout_(new QGridLayout(this)),
      backgroundPiximageItem_(new QPixmap(20, 20)),
      zoomFactor_(1.0f),
      zoomFactorDelta_(1.25f) {
    connect(scene_, &QGraphicsScene::changed, this, &ImageViewer::sceneChanged);
    connect(view_, &SynchableGraphicsView::transformChanged, this,
            &ImageViewer::transformChanged);
    connect(view_, &SynchableGraphicsView::scrollChanged, this,
            &ImageViewer::scrollChanged);
    connect(view_, &SynchableGraphicsView::wheelNotches, this,
            &ImageViewer::handleWheelNotches);

    backgroundPiximageItem_->fill(QColor("powderblue"));
    QPainter painter(backgroundPiximageItem_);
    painter.fillRect(0, 0, 10, 10, QColor("palegoldenrod"));
    painter.fillRect(10, 10, 10, 10, QColor("palegoldenrod"));
    painter.end();
    scene_->setBackgroundBrush(*backgroundPiximageItem_);
    view_->setRenderHint(QPainter::RenderHint::SmoothPixmapTransform);

    pixmapItem_.reset(new QGraphicsPixmapItem());
    scene_->addItem(pixmapItem_.get());

    if (pixmap != nullptr) pixmapItem_->setPixmap(*pixmap);

    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->addWidget(view_, 0, 0);
    setLayout(layout_);

    zoomFactor_ = view_->transform().m11();
    view_->show();
}

void ImageViewer::wheelEvent(QWheelEvent* event) { view_->wheelEvent(event); }

void ImageViewer::setImage(const QPixmap& pixmap) {
    pixmapItem_->setPixmap(pixmap);
    pixmapItem_->setTransformationMode(
        Qt::TransformationMode::SmoothTransformation);
    fitToWindow();
}

void ImageViewer::fitToWindow() {
    if (pixmapItem_->pixmap().isNull()) {
        std::cout << "pixmap is null" << std::endl;
        return;
    }
    pixmapItem_->setTransformationMode(
        Qt::TransformationMode::SmoothTransformation);
    view_->fitInView(pixmapItem_.get(), Qt::KeepAspectRatio);
    view_->checkTransformChanged();
}

void ImageViewer::fitToWindowWidth() {
    if (pixmapItem_->pixmap().isNull() || pixmapItem_->pixmap().width() == 0)
        return;

    // TODO
    auto viewRect = view_->viewport()->rect().adjusted(2, 2, -2, -2);
    float f = (float)(viewRect.width()) / pixmapItem_->pixmap().width();

    scaleImage(f, false);
}

void ImageViewer::fitToWindowHeight() {
    if (pixmapItem_->pixmap().isNull() || pixmapItem_->pixmap().height() == 0)
        return;

    // TODO
    auto viewRect = view_->viewport()->rect().adjusted(2, 2, -2, -2);
    float f = (float)(viewRect.height()) / pixmapItem_->pixmap().height();

    scaleImage(f, false);
}

void ImageViewer::handleWheelNotches(float notches) {
    // std::cout << __FUNCTION__ << std::endl;
    // std::cout << zoomFactorDelta_ << ", " << notches << ", "
    //           << double(std::pow(zoomFactorDelta_, notches)) << std::endl;
    scaleImage(double(std::pow(zoomFactorDelta_, notches)));
}

void ImageViewer::closeEvent(QCloseEvent* event) {
    disconnectSbarSignals();
    QFrame::closeEvent(event);
}

void ImageViewer::scaleImage(float factor, bool combine) {
    if (pixmapItem_->pixmap().isNull()) {
        std::cout << "pixmap is null" << std::endl;
        return;
    }

    if (combine) {
        setZoomFactor(zoomFactor() * factor);
    } else {
        setZoomFactor(factor);
    }
    view_->checkTransformChanged();
}
void ImageViewer::setZoomFactor(float factor) {
    if (factor < 1.0) {
        pixmapItem_->setTransformationMode(
            Qt::TransformationMode::SmoothTransformation);
    } else {
        pixmapItem_->setTransformationMode(
            Qt::TransformationMode::FastTransformation);
    }
    view_->setZoomFactor(factor);
}