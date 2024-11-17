#include "main_window.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setFont(QFont("Microsoft Yahei", 9));

    // 默认隐藏到系统托盘
    main_window w;
    w.hide();

    return a.exec();
}
