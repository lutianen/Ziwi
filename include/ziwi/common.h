#pragma once

#include <QColor>
#include <QFont>
#include <QIcon>
#include <string>

const std::string VERSION = "Version: 1.1.0\n";
const std::string DATE = "Date: 02/06/2023\n";
const std::string DEVELOPER =
    "Developer: 西安电子科技大学图像传输与处理研究所（西电图像所）";

const std::string SYSTEM_INFO =
    "Ziwi is a free and open source project. "
    "It is a simple image viewer based on Qt5 and OpenCV4. It can display "
    "raw images(8/12/16 bits).";

#define ABOUT_WIDGET_WIDTH 560
#define ABOUT_WIDGET_HEIGHT 480
const char *const kAppName = "Ziwi - DeCompImageViewer";
const QColor kNormalColor = QColor("#2C3E50");
const QColor kSelectedColor = QColor("#2980B9");

//
const std::string kBASE_DIR{"/home/lux/Desktop/Ziwi/"};
const std::string kICON_LOGO = kBASE_DIR + "icon/logo.ico";
const std::string kICON_LOGO_2 = kBASE_DIR + "icon/ziwi.png";
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