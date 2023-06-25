
#include <ziwi/common.h>
#include <ziwi/parameterConfigDialog.h>

#include <QButtonGroup>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSettings>
#include <iostream>
#include <memory>

using Lux::ziwi::ParaConfDialog;

ParaConfDialog::ParaConfDialog(QDialog *parent)
    : QDialog(parent),
      ce7_(new QRadioButton("CE7", this)),
      tw2_(new QRadioButton("TW2", this)),
      width_(new QLineEdit(this)),
      height_(new QLineEdit(this)),
      channels_(new QLineEdit(this)),
      radio8_(new QRadioButton("8", this)),
      radio12_(new QRadioButton("12", this)),
      radio16_(new QRadioButton("16", this)),
      radioBigEndian_(new QRadioButton("大端", this)),
      radioLittleEndian_(new QRadioButton("小端", this)),
      radio18_(new QRadioButton("第一高八位", this)),
      radio28_(new QRadioButton("第二高八位", this)),
      radio38_(new QRadioButton("第三高八位", this)),
      radio48_(new QRadioButton("第四高八位", this)),
      radio58_(new QRadioButton("第五高八位", this)),
      radioAll8_(new QRadioButton("AllIn8", this)),
      conf_(new Lux::ziwi::ParaConf) {
    auto setPtr =
        std::make_unique<QSettings>(kPARA_INI.c_str(), QSettings::IniFormat);

    QButtonGroup *workSpace = new QButtonGroup(this);
    QLabel *workSpaceLabel = new QLabel("Worksapce: ", this);
    workSpace->addButton(ce7_, 0);
    workSpace->addButton(tw2_, 1);
    if (setPtr->value("workspace").toInt() == 0)
        ce7_->setChecked(true);
    else
        tw2_->setChecked(true);

    QButtonGroup *radioBits = new QButtonGroup(this);
    radioBits->addButton(radio8_, 0);
    radioBits->addButton(radio12_, 1);
    radioBits->addButton(radio16_, 2);
    switch (setPtr->value("bpp").toInt()) {
        case 8:
            radio8_->setChecked(true);
            break;
        case 12:
            radio12_->setChecked(true);
            break;
        case 16:
            radio16_->setChecked(true);
            break;
        default:
            break;
    }

    QButtonGroup *radioEndians = new QButtonGroup(this);
    radioEndians->addButton(radioBigEndian_, 0);
    radioEndians->addButton(radioLittleEndian_, 1);
    if (setPtr->value("endian") == "true")
        radioBigEndian_->setChecked(true);
    else
        radioLittleEndian_->setChecked(true);

    QButtonGroup *radio8s = new QButtonGroup();
    radio8s->addButton(radio18_, 0);
    radio8s->addButton(radio28_, 1);
    radio8s->addButton(radio38_, 2);
    radio8s->addButton(radio48_, 3);
    radio8s->addButton(radio58_, 4);
    radio8s->addButton(radioAll8_, 5);
    switch (setPtr->value("mode").toInt()) {
        case 0:
            radio18_->setChecked(true);
            break;
        case 1:
            radio28_->setChecked(true);
            break;
        case 2:
            radio38_->setChecked(true);
            break;
        case 3:
            radio48_->setChecked(true);
            break;
        case 4:
            radio58_->setChecked(true);
            break;
        case 5:
            radioAll8_->setChecked(true);
            break;
        default:
            break;
    }
    // radioAll8_->setChecked(true);

    // Width
    auto widthLabel = new QLabel("Width: ", this);
    connect(width_, &QLineEdit::textChanged, this,
            &ParaConfDialog::onWidthChanged);
    widthLabel->setBuddy(width_);
    if (setPtr->value("width").toInt() != 0)
        width_->setText(setPtr->value("width").toString());
    else
        width_->setText("5120");

    // Height
    auto heightLabel = new QLabel("Height: ", this);
    connect(height_, &QLineEdit::textChanged, this,
            &ParaConfDialog::onHeightChanged);
    heightLabel->setBuddy(height_);
    if (setPtr->value("height").toInt() != 0)
        height_->setText(setPtr->value("height").toString());
    else
        height_->setText("3840");

    // Channels
    auto channelsLabel = new QLabel("Channels: ", this);
    connect(channels_, &QLineEdit::textChanged, this,
            &ParaConfDialog::onChannelsChanged);
    channelsLabel->setBuddy(channels_);
    if (setPtr->value("channels").toInt() != 0)
        channels_->setText(setPtr->value("channels").toString());
    else
        channels_->setText("1");

    // bpp
    auto bppLabel = new QLabel("Bits per pixel: ", this);

    // mode
    auto modeLabel = new QLabel("Mode: ", this);

    // endian
    auto endianLabel = new QLabel("Endian: ", this);

    auto errorMsgLabel = new QLabel(this);
    errorMsgLabel->setStyleSheet("color: red");

    auto submitBtn = new QPushButton("Submit", this);
    connect(submitBtn, &QPushButton::clicked, this, &ParaConfDialog::onSubmit);

    auto layout = new QGridLayout(this);
    layout->addWidget(workSpaceLabel, 0, 0);
    layout->addWidget(ce7_, 0, 1);
    layout->addWidget(tw2_, 0, 2);

    layout->addWidget(widthLabel, 1, 0);
    layout->addWidget(width_, 1, 1);
    layout->addWidget(new QLabel("像素", this), 1, 2);

    layout->addWidget(heightLabel, 2, 0);
    layout->addWidget(height_, 2, 1);
    layout->addWidget(new QLabel("像素", this), 2, 2);

    layout->addWidget(channelsLabel, 3, 0);
    layout->addWidget(channels_, 3, 1);

    layout->addWidget(bppLabel, 4, 0);
    layout->addWidget(radio8_, 5, 0);
    layout->addWidget(radio12_, 5, 1);
    layout->addWidget(radio16_, 5, 2);

    layout->addWidget(endianLabel, 6, 0);
    layout->addWidget(radioBigEndian_, 7, 0);
    layout->addWidget(radioLittleEndian_, 7, 1);

    layout->addWidget(modeLabel, 8, 0);
    layout->addWidget(radio18_, 9, 0);
    layout->addWidget(radio28_, 9, 1);
    layout->addWidget(radio38_, 9, 2);
    layout->addWidget(radio48_, 10, 0);
    layout->addWidget(radio58_, 10, 1);
    layout->addWidget(radioAll8_, 10, 2);

    layout->addWidget(submitBtn, 11, 1, 1, 1);
    setLayout(layout);
    setWindowTitle("Raw 图像参数配置");
    setMaximumSize(400, 300);
    setMinimumSize(400, 300);
}

void ParaConfDialog::onWidthChanged(const QString &width) {
    // width_ = width.toUInt();
    std::cout << width.toStdString() << std::endl;
    onTextChanged();
}

void ParaConfDialog::onHeightChanged(const QString &height) {
    // height_ = height.toUInt();
    std::cout << height.toStdString() << std::endl;
    onTextChanged();
}

void ParaConfDialog::onChannelsChanged(const QString &channels) {
    // channels_ = channels.toUInt();
    std::cout << channels.toStdString() << std::endl;
    onTextChanged();
}

void ParaConfDialog::onSubmit() {
    if (width_->text().isEmpty() || height_->text().isEmpty() ||
        channels_->text().isEmpty()) {
        std::cout << "Please input all parameters" << std::endl;
        return;
    }

    if (ce7_->isChecked())
        conf_->workspace = 0;
    else if (tw2_->isChecked())
        conf_->workspace = 1;

    conf_->width = width_->text().toUInt();
    conf_->height = height_->text().toUInt();
    conf_->channels = channels_->text().toUInt();

    if (radio8_->isChecked())
        conf_->bpp = 8;
    else if (radio12_->isChecked())
        conf_->bpp = 12;
    else if (radio16_->isChecked())
        conf_->bpp = 16;

    if (radioBigEndian_->isChecked())
        conf_->bigEndian = true;
    else if (radioLittleEndian_->isChecked())
        conf_->bigEndian = false;

    if (radio18_->isChecked())
        conf_->mode = 0;
    else if (radio28_->isChecked())
        conf_->mode = 1;
    else if (radio38_->isChecked())
        conf_->mode = 2;
    else if (radio48_->isChecked())
        conf_->mode = 3;
    else if (radio58_->isChecked())
        conf_->mode = 4;
    else if (radioAll8_->isChecked())
        conf_->mode = 5;

    accept();
}

void ParaConfDialog::onTextChanged() {
    QWidget *widget = qobject_cast<QWidget *>(sender());
    if (widget) widget->setStyleSheet("QLineEdit { background-color: #C4DF9B}");
}

Lux::ziwi::ParaConf ParaConfDialog::paraConf() const { return *conf_; }