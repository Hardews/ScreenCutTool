#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QDesktopServices>

class main_window : public QMainWindow
{
public:
    main_window(QWidget* parent = nullptr);
private:
    QPushButton* screen_cut_button;
    QMenu *trayMenu;                // 托盘菜单
    QSystemTrayIcon *tray;          // 托盘图标添加成员
protected:
    void closeEvent(QCloseEvent *event);
private slots:
    void on_screen_cut_btn_clicked();
    void on_screen_cut_and_copy_btn_clicked();
    void on_screen_cut_and_save_as_btn_clicked();
    void icon_activated(QSystemTrayIcon::ActivationReason ireason);
    void keyPress(int key);
};

#endif // MAIN_WINDOW_H
