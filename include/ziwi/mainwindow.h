#pragma once

#include <ziwi/about.h>
#include <ziwi/algorithm.h>
#include <ziwi/imageViewer.h>
#include <ziwi/parameterConfigDialog.h>

#include <QAction>
#include <QColor>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

namespace Lux {
namespace ziwi {
class DeCompImgViewMainWindow : public QMainWindow {
    Q_OBJECT

private:
    std::unique_ptr<Ui::MainWindow> ui_;

    QColor normalColor_;
    QColor selectedColor_;

    unsigned char workspace_;
    int mode_;
    bool endian_;
    int width_;
    int height_;
    int bpp_;
    int channel_;

    // QGraphicsScene* scene_;
    Lux::ziwi::ImageViewer* imageViewer_;
    QLabel* appLabel_;
    QLabel* fileLabel_;

    DisplayUtils* imgCore_;
    Lux::ziwi::ParaConfDialog* paraConfDialog_;

public:
    DeCompImgViewMainWindow(QPixmap* pixmap = nullptr,
                            std::string name = "Ziwi");
    ~DeCompImgViewMainWindow();

private:
    void buildStatusBar();
    void buildAction();
    void buildImageViewer();

    unsigned char* loadImageData();
    void paramConfig();

private slots:
    void onShowLVDSImage();
    void onActualSize();
    void onFitWindow();
    void onFitWidth();
    void onFitHeight();
    void onImageCodec();
    void onAbout();
    void transformChanged();
    void scrollChanged();
};
}  // namespace ziwi
}  // namespace Lux