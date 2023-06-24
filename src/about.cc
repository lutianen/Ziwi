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
}

void About::initUiComponent(void) {
    infoLabel = new QLabel(this);
    infoLabel->setOpenExternalLinks(true);
    infoLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);
    infoLabel->setText(tr(Github.c_str()));

    auto infoFont = QFont(FONT);
    infoFont.setWeight(QFont::Cursive);
    infoFont.setPointSize(10);
    infoTextEdit = new QTextEdit(this);
    infoTextEdit->setFont(infoFont);
    infoTextEdit->setText(tr(SYSTEM_INFO.c_str()));
    infoTextEdit->setReadOnly(true);
    infoTextEdit->setFixedSize(ABOUT_WIDGET_WIDTH - 36, 128);

    auto iconLabel = new QLabel(this);
    iconLabel->setScaledContents(true);
    iconLabel->setPixmap(QPixmap(kICON_LOGO.c_str()));
    iconLabel->setAlignment(Qt::AlignmentFlag::AlignCenter);

    auto layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignmentFlag::AlignCenter);
    layout->addWidget(iconLabel);
    layout->addWidget(infoLabel);
    layout->addWidget(infoTextEdit);
}