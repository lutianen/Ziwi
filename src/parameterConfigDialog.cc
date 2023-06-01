
#include <ziwi/parameterConfigDialog.h>

#include <QButtonGroup>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <iostream>

using Lux::ziwi::ParaConfDialog;

ParaConfDialog::ParaConfDialog(QDialog *parent)
    : QDialog(parent),
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
      radioAll8_(new QRadioButton("AllIn8", this)),
      conf_(new Lux::ziwi::ParaConf) {
    QButtonGroup *radioBits = new QButtonGroup(this);

    radioBits->addButton(radio8_, 0);
    radioBits->addButton(radio12_, 1);
    radioBits->addButton(radio16_, 2);
    radio16_->setChecked(true);

    QButtonGroup *radioEndians = new QButtonGroup(this);
    radioEndians->addButton(radioBigEndian_, 0);
    radioEndians->addButton(radioLittleEndian_, 1);
    radioLittleEndian_->setChecked(true);

    QButtonGroup *radio8s = new QButtonGroup();
    radio8s->addButton(radio18_, 0);
    radio8s->addButton(radio28_, 1);
    radio8s->addButton(radio38_, 2);
    radio8s->addButton(radio48_, 3);
    radio8s->addButton(radioAll8_, 4);
    radioAll8_->setChecked(true);

    // Width
    auto widthLabel = new QLabel("Width: ", this);
    width_->setText("5120");
    connect(width_, &QLineEdit::textChanged, this,
            &ParaConfDialog::onWidthChanged);
    widthLabel->setBuddy(width_);

    // Height
    auto heightLabel = new QLabel("Height: ", this);
    height_->setText("3840");
    connect(height_, &QLineEdit::textChanged, this,
            &ParaConfDialog::onHeightChanged);
    heightLabel->setBuddy(height_);

    // Channels
    auto channelsLabel = new QLabel("Channels: ", this);
    channels_->setText("1");
    connect(channels_, &QLineEdit::textChanged, this,
            &ParaConfDialog::onChannelsChanged);
    channelsLabel->setBuddy(channels_);

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
    layout->addWidget(radioAll8_, 10, 1);

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
    else if (radioAll8_->isChecked())
        conf_->mode = 4;

    accept();
}

void ParaConfDialog::onTextChanged() {
    QWidget *widget = qobject_cast<QWidget *>(sender());
    if (widget) widget->setStyleSheet("QLineEdit { background-color: #C4DF9B}");
}

Lux::ziwi::ParaConf ParaConfDialog::paraConf() const { return *conf_; }