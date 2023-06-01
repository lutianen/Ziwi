#pragma once

#include <QColor>
#include <QFont>
#include <QIcon>
#include <string>

#define ABOUT_WIDGET_WIDTH 480
#define ABOUT_WIDGET_HEIGHT 240
const char *const kAppName = "Ziwi - DeCompImageViewer";
const QColor kNormalColor = QColor("#2C3E50");
const QColor kSelectedColor = QColor("#2980B9");

//
const std::string kBASE_DIR{"/home/lux/Desktop/Ziwi/"};
const std::string kICON_LOGO = kBASE_DIR + "icon/logo.ico";
const std::string kICON_ACTUAL_SIZE = kBASE_DIR + "icon/actualSize.svg";
const std::string kICON_CODEC = kBASE_DIR + "icon/codec.svg";
const std::string kICON_FIT_HEIGHT = kBASE_DIR + "icon/fitHeight.svg";
const std::string kICON_FIT_WIDTH = kBASE_DIR + "icon/fitWidth.svg";
const std::string kICON_FIT_WINDOW = kBASE_DIR + "icon/fitWindow.svg";
const std::string kICON_GETIMAGE = kBASE_DIR + "icon/getImg.svg";

//
const std::string RELAY_DIR = kBASE_DIR + "internal";
const std::string RELAY_FILE = kBASE_DIR + "internal/internal_only_disp.raw";

#ifdef _WIN32
const QFont FONT = QFont("微软雅黑", 9);
#else
const QFont FONT = QFont("WenQuanYi Zen Hei", 9);
#endif