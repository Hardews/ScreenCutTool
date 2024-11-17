#include <QApplication>
#include "main_window.h"
#include "screen_widget.h"
#include "QHotKey/qhotkey.h"

main_window::main_window(QWidget*) {
    // 托盘
    tray = new QSystemTrayIcon(this);// 初始化托盘对象 tray
    tray->setIcon(QIcon("://logo/logo.svg")); // 托盘图标
    tray->setToolTip("截图工具 学习版\n截图快捷键: Ctrl+Alt+F"); // 提示文字
    QString title="截图工具";
    QString text="截图工具已启动";
    tray->show();
    tray->showMessage(title, text, QSystemTrayIcon::Information, 3000);

    trayMenu = new QMenu();
    trayMenu->addAction("截图", this, &main_window::on_screen_cut_btn_clicked);
    trayMenu->addAction("截图并自动复制", this, &main_window::on_screen_cut_and_copy_btn_clicked);
    trayMenu->addAction("截图并另存为", this, &main_window::on_screen_cut_and_save_as_btn_clicked);
    trayMenu->addAction("退出软件", this, &QApplication::exit);
    tray->setContextMenu(trayMenu);
    connect(tray, &QSystemTrayIcon::activated, this, &main_window::icon_activated);

    // 全局获取键盘输入
    QHotkey *hotkey = new QHotkey(QKeySequence("Ctrl+Alt+F"), true);
    // 连接热键被按下的信号与槽
    QObject::connect(hotkey,&QHotkey::activated,[=](){
        // 热键被按下的处理代码段
        ScreenWidget::Instance()->showFullScreen();
    });
}

void main_window::closeEvent(QCloseEvent *event)
{
    if(tray->isVisible())
    {
        hide(); // 隐藏窗口
        event->ignore(); // 忽略事件
    }
}

void main_window::on_screen_cut_btn_clicked()
{
    ScreenWidget::Instance()->showFullScreen();
}

void main_window::on_screen_cut_and_copy_btn_clicked()
{
    ScreenWidget::Instance(true)->showFullScreen();
}

void main_window::on_screen_cut_and_save_as_btn_clicked()
{
    ScreenWidget::Instance(false, true)->showFullScreen();
}

void main_window::icon_activated(QSystemTrayIcon::ActivationReason ireason)
{
    switch (ireason)
    {
    case QSystemTrayIcon::Trigger:
        // 单击应用图标，启用截图功能
        ScreenWidget::Instance()->showFullScreen();
        break;
    default:
        break;
    }
}
