#include <ziwi/about.h>
#include <ziwi/common.h>

#include <QDebug>
#include <QFont>
#include <QFrame>
#include <QLayout>
#include <QPalette>

#define SAFE_FREE(p)     \
    {                    \
        if (p != NULL) { \
            delete p;    \
            p = NULL;    \
        }                \
    }

using namespace Lux::ziwi;

About::About(QWidget *parent) : QWidget(parent) {
    /* init ui */
    initUiComponent();
}

About::~About() {
    SAFE_FREE(titleLabel);
    SAFE_FREE(infoLabel);
    SAFE_FREE(infoTextEdit);
    SAFE_FREE(exitBtn);
}

void About::exitBtnClicked(void) { this->close(); }

void About::initUiComponent(void) {
    int label_w = ABOUT_WIDGET_WIDTH, label_h = 20;
    int text_w = ABOUT_WIDGET_WIDTH - 40, text_h = 120;
    int btn_w = 80, btn_h = 30;
    int btn_x = ABOUT_WIDGET_WIDTH - btn_w;
    int btn_y = ABOUT_WIDGET_HEIGHT - btn_h;

    titleLabel = new QLabel(this);
    titleLabel->setText(tr("ImageViewer"));
    titleLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    auto titleFont = QFont(FONT);
    titleFont.setBold(true);
    titleFont.setPointSize(10);
    titleLabel->setFont(titleFont);

    infoLabel = new QLabel(this);
    infoLabel->setOpenExternalLinks(true);
    infoLabel->setText(
        tr("<a href=\"https://github.com/lutianen/Ziwi\">"
           "[https://github.com/lutianen/Ziwi]"));

    infoLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    QString info;
    info.append(
        "Ziwi is a free and open source project. "
        "It is a simple image viewer based on Qt5. It can display images.");

    infoTextEdit = new QTextEdit(this);
    infoTextEdit->setText(info);
    infoTextEdit->setReadOnly(1);
    infoTextEdit->setGeometry(15, 80, text_w, text_h);

    QPalette palette;
    palette.setColor(QPalette::Text, kNormalColor);
    infoTextEdit->setPalette(palette);

    auto infoFont = QFont(FONT);
    infoFont.setWeight(QFont::Cursive);
    infoFont.setPointSize(12);
    infoTextEdit->setFont(infoFont);

    exitBtn = new QPushButton(this);
    exitBtn->setText(tr("OK"));
    exitBtn->setGeometry(btn_x - 10, btn_y - 5, btn_w, btn_h);
    connect(exitBtn, SIGNAL(clicked(bool)), this, SLOT(exitBtnClicked()));

    auto layout = new QVBoxLayout(this);
    layout->addWidget(titleLabel);
    layout->addWidget(infoLabel);
    layout->addWidget(infoTextEdit);
    layout->addWidget(exitBtn);
}