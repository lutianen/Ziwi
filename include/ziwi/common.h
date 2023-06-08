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
    "Ziwi is a free and open source image viewer, based on Qt5 and OpenCV4, "
    "implemented in C++ programming. It can be used to view 8 bits / 12 bits / "
    "16 bits Bayer images, but also supports a variety of common image "
    "formats, such as PNG, JPG, TIFF, SVG, etc.\n\n"
    
    "BSD 3-Clause License\n"

    "Copyright (c) 2023, lutianen\n"

    "Redistribution and use in source and binary forms, with or without"
    "modification, are permitted provided that the following conditions are "
    "met:\n"

    "1. Redistributions of source code must retain the above copyright notice, "
    "this"
    "list of conditions and the following disclaimer.\n"

    "2. Redistributions in binary form must reproduce the above copyright "
    "notice,"
    "this list of conditions and the following disclaimer in the documentation"
    "and/or other materials provided with the distribution.\n"

    "3. Neither the name of the copyright holder nor the names of its"
    "contributors may be used to endorse or promote products derived from"
    "this software without specific prior written permission.\n"

    "THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS "
    "IS\""
    "AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE"
    "IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR "
    "PURPOSE ARE"
    "DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE "
    "LIABLE"
    "FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL"
    "DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR"
    "SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER"
    "CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT "
    "LIABILITY,"
    "OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE "
    "USE"
    "OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.";

#define ABOUT_WIDGET_WIDTH 560
#define ABOUT_WIDGET_HEIGHT 570
const char *const kAppName = "Ziwi";
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