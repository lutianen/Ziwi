#include <ziwi/common.h>
#include <ziwi/mainwindow.h>

#include <QApplication>
#include <iostream>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(kICON_LOGO.c_str()));

    Lux::ziwi::DeCompImgViewMainWindow win;

    win.show();
    return app.exec();
}