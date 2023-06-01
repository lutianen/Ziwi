#pragma once

#include <QGraphicsItemGroup>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QScrollBar>
#include <QWheelEvent>
#include <QWidget>
#include <cmath>
#include <iostream>

namespace Lux {
namespace ziwi {

class SynchableGraphicsView : public QGraphicsView {
    Q_OBJECT

    SynchableGraphicsView(const SynchableGraphicsView &) = delete;
    SynchableGraphicsView &operator=(const SynchableGraphicsView &) = delete;

private:
    float zoomFactor_;
    bool handDrag_;
    QTransform transform_;

signals:
    // Signals
    void transformChanged();
    void scrollChanged();
    void wheelNotches(float);
    void mouseReleased(QMouseEvent *event);
    void mousePressed(QMouseEvent *event);

public:
    SynchableGraphicsView(QGraphicsScene *scene = nullptr,
                          QWidget *parent = nullptr);

    /* Connect to scrollbar changed signals to synchronize panning.
        slot to connect scrollbar signals to. */
    template <class Func>
    void connectSbarSignals(Func &&slot) {
        auto sbar = horizontalScrollBar();
        connect(sbar, &QScrollBar::valueChanged, this, slot,
                Qt::UniqueConnection);
        // connect(sbar, &QScrollBar::sliderMoved, this, slot,
        //         Qt::UniqueConnection);
        connect(sbar, &QScrollBar::rangeChanged, this, slot,
                Qt::UniqueConnection);

        auto bar = verticalScrollBar();
        connect(bar, &QScrollBar::valueChanged, this, slot,
                Qt::ConnectionType::UniqueConnection);
        // connect(bar, &QScrollBar::sliderMoved, this, slot,
        //         Qt::UniqueConnection);
        connect(bar, &QScrollBar::rangeChanged, this, slot,
                Qt::UniqueConnection);
    }

    // Disconnect from scrollbar changed signals.
    void disConnectSbarSignals() {
        auto sbar = horizontalScrollBar();
        disconnect(sbar, &QScrollBar::valueChanged, this,
                   &SynchableGraphicsView::scrollChanged);
        // disconnect(sbar, &QScrollBar::sliderMoved, this,
        //            &SynchableGraphicsView::scrollChanged);
        disconnect(sbar, &QScrollBar::rangeChanged, this,
                   &SynchableGraphicsView::scrollChanged);

        auto bar = verticalScrollBar();
        disconnect(bar, &QScrollBar::valueChanged, this,
                   &SynchableGraphicsView::scrollChanged);
        // disconnect(bar, &QScrollBar::sliderMoved, this,
        //            &SynchableGraphicsView::scrollChanged);
        disconnect(bar, &QScrollBar::rangeChanged, this,
                   &SynchableGraphicsView::scrollChanged);
    }

    // Properties
    bool handDragging() const { return handDrag_; }

    QPointF scrollState() const;
    void setScrollState(const QPointF &scrollState);

    float zoomFactor() const { return transform().m11(); }
    void setZoomFactor(float zoomFactor);

    void wheelEvent(QWheelEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    bool checkTransformChanged();
    /* Reset view transform changed info. */
    void clearTransformChanges() { transform_ = transform(); }
    void scrollToTop();
    void scrollToBottom();
    void scrollToBegin();
    void scrollToEnd();
    void centerView();
    void enableScrollBars(bool enable);
    void enableHandDrag(bool enable);
    void dumpTransform(const QTransform &t, const QString &padding = "");
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *event) override;
};

/**
 * @brief ImageViewer - A public Interface for ImageViewer.
 *
 */
class ImageViewer : public QFrame {
    Q_OBJECT

    ImageViewer(const ImageViewer &) = delete;
    ImageViewer &operator=(const ImageViewer &) = delete;

private:
    QGraphicsScene *scene_;
    SynchableGraphicsView *view_;
    std::unique_ptr<QGraphicsPixmapItem> pixmapItem_;
    QGridLayout *layout_;
    QPixmap *backgroundPiximageItem_;
    double zoomFactor_;
    double zoomFactorDelta_;

public:
    ImageViewer(QPixmap *pixmap, QString name);
    void enableScrollBars(bool enable) { view_->enableScrollBars(enable); }
    void enableHandDrag(bool enable) { view_->enableHandDrag(enable); }

    bool handDragging() const { return view_->handDragging(); }
    void dumpTransform() { view_->dumpTransform(view_->transform(), "    "); }

    QPixmap pixmap() { return pixmapItem_->pixmap(); }
    void setImage(const QPixmap &pixmap);

signals:
    void sceneChanged();
    void transformChanged();
    void scrollChanged();

public slots:
    void scrollToBottom() { view_->scrollToBottom(); }
    void scrollToTop() { view_->scrollToTop(); }
    void scrollToBegin() { view_->scrollToBegin(); }
    void scrollToEnd() { view_->scrollToEnd(); }
    void centerView() { view_->centerView(); }

    void zoomIn() { scaleImage(zoomFactorDelta_); }
    void zoomOut() { scaleImage(1.0 / zoomFactorDelta_); }
    void actualSize() { scaleImage(1.0, false); }

    void fitToWindow();
    void fitToWindowWidth();
    void fitToWindowHeight();

    void handleWheelNotches(float notches);
    void closeEvent(QCloseEvent *event) override;

private:
    template <class Func>
    void connectSbarSignals(Func slot) {
        view_->connectSbarSignals(slot);
    }
    void disconnectSbarSignals() { view_->disConnectSbarSignals(); }
    void wheelEvent(QWheelEvent *event) override;
    void scaleImage(float factor, bool combine = true);

    QPointF scrollState() const { return view_->scrollState(); }
    void setScrollState(const QPointF &p) { view_->setScrollState(p); }

    float zoomFactor() const { return view_->zoomFactor(); }
    void setZoomFactor(float factor);
};

}  // namespace ziwi
}  // namespace Lux
