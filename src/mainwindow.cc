#include <ziwi/common.h>
#include <ziwi/mainwindow.h>
#include <ziwi/parameterConfigDialog.h>

#include <QActionGroup>
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <fstream>
#include <iostream>

#include "./ui_mainwindow.h"

using Lux::ziwi::DeCompImgViewMainWindow;

DeCompImgViewMainWindow::DeCompImgViewMainWindow(QPixmap* pixmap,
                                                 std::string name)
    : QMainWindow(),
      ui_(new Ui::MainWindow),
      normalColor_(kNormalColor),
      selectedColor_(kSelectedColor),
      workspace_(0),
      mode_(5),
      endian_(false),
      width_(5120),
      height_(3840),
      bpp_(16),
      channel_(1),
      imageViewer_(new Lux::ziwi::ImageViewer(nullptr, "ziwi")),
      appLabel_(new QLabel(this)),

      fileLabel_(new QLabel("请选择待查看图像", this)),
      imgCore_(new DisplayUtils(true, RELAY_FILE)) {
    ui_->setupUi(this);

    buildStatusBar();
    buildAction();
    buildImageViewer();
}

DeCompImgViewMainWindow::~DeCompImgViewMainWindow() { delete imgCore_; }

void DeCompImgViewMainWindow::buildStatusBar() {
    appLabel_->setText(kAppName);
    appLabel_->setAlignment(Qt::AlignmentFlag::AlignLeft);
    ui_->bottomBar->addPermanentWidget(fileLabel_, 1);
    ui_->bottomBar->addPermanentWidget(appLabel_);
}

void DeCompImgViewMainWindow::buildAction() {
    ui_->toolBar->setFont(FONT);
    ui_->toolBar->setContentsMargins(0, 0, 0, 0);
    ui_->toolBar->layout()->setSpacing(0);
    ui_->toolBar->layout()->setContentsMargins(0, 0, 0, 0);

    // open image
    ui_->actionOpenImage->setIcon(QIcon(kICON_GETIMAGE.c_str()));
    connect(ui_->actionOpenImage, &QAction::triggered, this,
            &DeCompImgViewMainWindow::onShowLVDSImage);

    // show mode
    auto actionShowModeGrop = new QActionGroup(this);
    actionShowModeGrop->setExclusive(true);  // 设置互斥
    actionShowModeGrop->addAction(ui_->actionActualSize);
    connect(ui_->actionActualSize, &QAction::triggered, this,
            &DeCompImgViewMainWindow::onActualSize);
    ui_->actionActualSize->setIcon(QIcon(kICON_ACTUAL_SIZE.c_str()));

    actionShowModeGrop->addAction(ui_->actionFitWindow);
    connect(ui_->actionFitWindow, &QAction::triggered, this,
            &DeCompImgViewMainWindow::onFitWindow);
    ui_->actionFitWindow->setIcon(QIcon(kICON_FIT_WINDOW.c_str()));

    actionShowModeGrop->addAction(ui_->actionFitWidth);
    connect(ui_->actionFitWidth, &QAction::triggered, this,
            &DeCompImgViewMainWindow::onFitWidth);
    ui_->actionFitWidth->setIcon(QIcon(kICON_FIT_WIDTH.c_str()));

    actionShowModeGrop->addAction(ui_->actionFitHeight);
    connect(ui_->actionFitHeight, &QAction::triggered, this,
            &DeCompImgViewMainWindow::onFitHeight);
    ui_->actionFitHeight->setIcon(QIcon(kICON_FIT_HEIGHT.c_str()));

    // Codec
    connect(ui_->actionCodec, &QAction::triggered, this,
            &DeCompImgViewMainWindow::onImageCodec);
    ui_->actionCodec->setIcon(QIcon(kICON_CODEC.c_str()));

    // about
    connect(ui_->actionAbout, &QAction::triggered, this,
            &DeCompImgViewMainWindow::onAbout);
    ui_->actionAbout->setIcon(QIcon(kICON_LOGO.c_str()));
}

void DeCompImgViewMainWindow::buildImageViewer() {
    imageViewer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    imageViewer_->enableScrollBars(true);

    connect(this->imageViewer_, &Lux::ziwi::ImageViewer::transformChanged, this,
            &DeCompImgViewMainWindow::transformChanged);
    connect(this->imageViewer_, &Lux::ziwi::ImageViewer::scrollChanged, this,
            &DeCompImgViewMainWindow::scrollChanged);

    ui_->gridLayout->removeWidget(ui_->graphicsViewPlaceholder);
    ui_->gridLayout->addWidget(imageViewer_);
    ui_->centralwidget->setLayout(ui_->gridLayout);
}

ImageInfo* DeCompImgViewMainWindow::loadImageData() {
    // std::cout << __FUNCTION__ << std::endl;

    QString fileName = QFileDialog::getOpenFileName(
        this, tr("选择图像"), kBASE_DIR.c_str(),
        tr("图像文件(*.raw *.jpg *.png *.tiff *.svg);;所有文件 (*.*)"));
    fileLabel_->setText(fileName);

    ImageInfo* info = new ImageInfo();

    if (fileName.endsWith("raw", Qt::CaseSensitivity::CaseInsensitive)) {
        info->type_ = ImageType::RAW;
    } else if (fileName.endsWith("jpg", Qt::CaseSensitivity::CaseInsensitive)) {
        info->type_ = ImageType::JPG;
    } else if (fileName.endsWith("png", Qt::CaseSensitivity::CaseInsensitive)) {
        info->type_ = ImageType::PNG;
    } else if (fileName.endsWith("tiff",
                                 Qt::CaseSensitivity::CaseInsensitive)) {
        info->type_ = ImageType::TIFF;
    } else if (fileName.endsWith("svg", Qt::CaseSensitivity::CaseInsensitive)) {
        info->type_ = ImageType::SVG;
    } else {
        info->type_ = ImageType::UNKNOWN;
    }

    if (!fileName.isEmpty()) {
        info->name_ = fileName.toStdString();

        //
        if (info->type_ == ImageType::RAW) {
            std::ifstream ifs(fileName.toStdString(), std::ios::binary);
            if (ifs.is_open()) {
                ifs.seekg(0, std::ios::end);
                int length = ifs.tellg();
                ifs.seekg(0, std::ios::beg);
                // unsigned char* buffer = new unsigned char[length];
                info->data_ = new unsigned char[length];
                ifs.read((char*)info->data_, length);
                ifs.close();
                return info;
            } else {
                QMessageBox::information(this, tr("提示"), tr("打开文件失败"));
                return nullptr;
            }
        } else {
            info->data_ = nullptr;
            return info;
        }

    } else {
        QMessageBox::information(this, tr("提示"), tr("打开文件失败"));
    }
    return nullptr;
}

void DeCompImgViewMainWindow::onShowLVDSImage() {
    // std::cout << __FUNCTION__ << std::endl;

    // auto imgdata = loadImageData();
    auto imgInfo = loadImageData();
    if (imgInfo == nullptr) return;

    if (imgInfo->type_ == ImageType::RAW) {
        paramConfig();
        std::string tiffFile = "";
        auto outData = imgCore_->LoadDataForDisplaySelectableMode(
            workspace_, imgInfo->data_, 1, false, tiffFile, mode_, endian_,
            width_, height_, bpp_, channel_);
        if (outData == nullptr) {
            delete imgInfo;
            QMessageBox::information(this, tr("提示"), tr("转换失败"));
            return;
        }

        // QPixmap pixmap(fileName);
        imageViewer_->setImage(QPixmap::fromImage(
            QImage(outData, width_, height_, width_, QImage::Format_Indexed8)));

    } else if (imgInfo->type_ == ImageType::UNKNOWN) {
        QMessageBox::information(this, tr("提示"), tr("图片类型暂不支持"));
    } else {  // jpg, png, tiff
        imageViewer_->setImage(QPixmap(imgInfo->name_.c_str()));
    }

    delete imgInfo;
}

void DeCompImgViewMainWindow::onActualSize() {
    // std::cout << __FUNCTION__ << std::endl;
    imageViewer_->actualSize();
}

void DeCompImgViewMainWindow::onFitWindow() {
    // std::cout << __FUNCTION__ << std::endl;
    imageViewer_->fitToWindow();
}

void DeCompImgViewMainWindow::onFitWidth() {
    // std::cout << __FUNCTION__ << std::endl;
    imageViewer_->fitToWindowWidth();
}

void DeCompImgViewMainWindow::onFitHeight() {
    // std::cout << __FUNCTION__ << std::endl;
    imageViewer_->fitToWindowHeight();
}

void DeCompImgViewMainWindow::onImageCodec() {
    std::cout << __FUNCTION__ << std::endl;
}

void DeCompImgViewMainWindow::onAbout() {
    // std::cout << __FUNCTION__ << std::endl;
    auto about = new Lux::ziwi::About;
    about->setWindowTitle("About Ziwi");
    about->setWindowIcon(QIcon(kICON_LOGO.c_str()));
    about->setFixedSize(ABOUT_WIDGET_WIDTH, ABOUT_WIDGET_HEIGHT);
    about->show();
}

void DeCompImgViewMainWindow::transformChanged() {
    // std::cout << __FUNCTION__ << std::endl;
}
void DeCompImgViewMainWindow::scrollChanged() {
    // std::cout << __FUNCTION__ << std::endl;
}

void DeCompImgViewMainWindow::paramConfig() {
    // std::cout << __FUNCTION__ << std::endl;
    paraConfDialog_ = new Lux::ziwi::ParaConfDialog;
    paraConfDialog_->setWindowIcon(QIcon(kICON_LOGO.c_str()));

    auto ret = paraConfDialog_->exec();
    if (ret == QDialog::Accepted) {
        Lux::ziwi::ParaConf paraConf = paraConfDialog_->paraConf();
        workspace_ = paraConf.workspace;
        width_ = paraConf.width;
        height_ = paraConf.height;
        channel_ = paraConf.channels;
        bpp_ = paraConf.bpp;
        mode_ = paraConf.mode;
        endian_ = paraConf.bigEndian;
    } else {
        std::cout << "cancel" << std::endl;
    }
}